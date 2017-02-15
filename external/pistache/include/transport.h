/* 
   Mathieu Stefani, 26 janvier 2016
   
   Transport TCP layer
*/

#pragma once

#include "reactor.h"
#include "mailbox.h"
#include "optional.h"
#include "async.h"
#include "stream.h"

namespace Net {

namespace Tcp {

class Peer;
class Handler;

class Transport : public Aio::Handler {
public:
    Transport(const std::shared_ptr<Tcp::Handler>& handler);

    void init(const std::shared_ptr<Tcp::Handler>& handler);

    void registerPoller(Polling::Epoll& poller);

    void handleNewPeer(const std::shared_ptr<Peer>& peer);
    void onReady(const Aio::FdSet& fds);

    template<typename Buf>
    Async::Promise<ssize_t> asyncWrite(Fd fd, const Buf& buffer, int flags = 0) {
        // If the I/O operation has been initiated from an other thread, we queue it and we'll process
        // it in our own thread so that we make sure that every I/O operation happens in the right thread
        auto ctx = context();
        const bool isInRightThread = std::this_thread::get_id() == ctx.thread();
        if (!isInRightThread) {
            return Async::Promise<ssize_t>([=](Async::Deferred<ssize_t> deferred) mutable {
                BufferHolder holder(buffer);
                auto detached = holder.detach();
                WriteEntry write(std::move(deferred), detached, flags);
                write.peerFd = fd;
                auto *e = writesQueue.allocEntry(std::move(write));
                writesQueue.push(e);
            });
        }
        return Async::Promise<ssize_t>([&](Async::Resolver& resolve, Async::Rejection& reject) {

            auto it = toWrite.find(fd);
            if (it != std::end(toWrite)) {
                reject(Net::Error("Multiple writes on the same fd"));
                return;
            }

            asyncWriteImpl(fd, flags, BufferHolder(buffer), Async::Deferred<ssize_t>(std::move(resolve), std::move(reject)));

        });
    }

    Async::Promise<rusage> load() {
        return Async::Promise<rusage>([=](Async::Deferred<rusage> deferred) {
            loadRequest_ = std::move(deferred);
            notifier.notify();
        });
    }


    template<typename Duration>
    void armTimer(Fd fd, Duration timeout, Async::Deferred<uint64_t> deferred) {
        armTimerMs(
                fd, std::chrono::duration_cast<std::chrono::milliseconds>(timeout),
                std::move(deferred));

    }

    void disarmTimer(Fd fd);

    std::shared_ptr<Aio::Handler> clone() const;

private:
    enum WriteStatus {
        FirstTry,
        Retry
    };

    struct BufferHolder {
        enum Type { Raw, File };

        explicit BufferHolder(const Buffer& buffer)
            : u(buffer)
            , type(Raw)
        {
            size_ = buffer.len;
        }

        explicit BufferHolder(const FileBuffer& buffer)
            : u(buffer.fd())
            , type(File)
        {
            size_ = buffer.size();
        }

        bool isFile() const { return type == File; }
        bool isRaw() const { return type == Raw; }
        size_t size() const { return size_; }

        Fd fd() const {
            if (!isFile())
                throw std::runtime_error("Tried to retrieve fd of a non-filebuffer");

            return u.fd;

        }

        Buffer raw() const {
            if (!isRaw())
                throw std::runtime_error("Tried to retrieve raw data of a non-buffer");

            return u.raw;
        }

        BufferHolder detach(size_t offset = 0) const {
            if (!isRaw())
                return BufferHolder(u.fd, size_);

            if (u.raw.isOwned)
                return BufferHolder(u.raw);

            auto detached = u.raw.detach(offset);
            return BufferHolder(detached);
        }

    private:
        BufferHolder(Fd fd, size_t size)
         : u(fd)
         , size_(size)
         , type(File)
        { }

        union U {
            Buffer raw;
            Fd fd;

            U(Buffer buffer) : raw(buffer) { }
            U(Fd fd) : fd(fd) { }
        } u;
        size_t size_;
        Type type;
    };

    struct WriteEntry {
        WriteEntry(Async::Deferred<ssize_t> deferred,
                    BufferHolder buffer, int flags = 0)
            : deferred(std::move(deferred))
            , buffer(std::move(buffer))
            , flags(flags)
            , peerFd(-1)
        { }

        Async::Deferred<ssize_t> deferred;
        BufferHolder buffer;
        int flags;
        Fd peerFd;
    };

    struct TimerEntry {
        TimerEntry(Fd fd, std::chrono::milliseconds value,
                Async::Deferred<uint64_t> deferred)
          : fd(fd)
          , value(value)
          , deferred(std::move(deferred))
        {
            active.store(true, std::memory_order_relaxed);
        }

        TimerEntry(TimerEntry&& other)
            : fd(other.fd)
            , value(other.value)
            , deferred(std::move(other.deferred))
            , active(other.active.load())
        { }

        void disable() {
            active.store(false, std::memory_order_relaxed);
        }

        bool isActive() {
            return active.load(std::memory_order_relaxed);
        }

        Fd fd;
        std::chrono::milliseconds value;
        Async::Deferred<uint64_t> deferred;
        std::atomic<bool> active;
    };

    struct PeerEntry {
        PeerEntry(std::shared_ptr<Peer> peer)
            : peer(std::move(peer))
        { }

        std::shared_ptr<Peer> peer;
    };

    /* @Incomplete: this should be a std::dequeue.
        If an asyncWrite on a particular fd is initiated whereas the fd is not write-ready
        yet and some writes are still on-hold, writes should queue-up so that when the
        fd becomes ready again, we can write everything
    */
    PollableQueue<WriteEntry> writesQueue;
    std::unordered_map<Fd, WriteEntry> toWrite;

    PollableQueue<TimerEntry> timersQueue;
    std::unordered_map<Fd, TimerEntry> timers;

    PollableQueue<PeerEntry> peersQueue;
    std::unordered_map<Fd, std::shared_ptr<Peer>> peers;

    Async::Deferred<rusage> loadRequest_;
    NotifyFd notifier;

    std::shared_ptr<Tcp::Handler> handler_;

    bool isPeerFd(Fd fd) const;
    bool isTimerFd(Fd fd) const;
    bool isPeerFd(Polling::Tag tag) const;
    bool isTimerFd(Polling::Tag tag) const;

    std::shared_ptr<Peer>& getPeer(Fd fd);
    std::shared_ptr<Peer>& getPeer(Polling::Tag tag);

    void
    armTimerMs(Fd fd,
              std::chrono::milliseconds value,
              Async::Deferred<uint64_t> deferred);

    void armTimerMsImpl(TimerEntry entry);

    void asyncWriteImpl(Fd fd, WriteEntry& entry, WriteStatus status = FirstTry);
    void asyncWriteImpl(
            Fd fd, int flags, const BufferHolder& buffer,
            Async::Deferred<ssize_t> deferred,
            WriteStatus status = FirstTry);

    void handlePeerDisconnection(const std::shared_ptr<Peer>& peer);
    void handleIncoming(const std::shared_ptr<Peer>& peer);
    void handleWriteQueue();
    void handleTimerQueue();
    void handlePeerQueue();
    void handleNotify();
    void handleTimer(TimerEntry entry);
    void handlePeer(const std::shared_ptr<Peer>& entry);
};

} // namespace Tcp

} // namespace Net
