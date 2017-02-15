FROM ubuntu:16.04

RUN apt-get clean && apt-get update && apt-get install -y --no-install-recommends software-properties-common imagemagick libmagick++-dev libboost-program-options1.58.0 libyaml-cpp0.5v5 iproute2
RUN echo "Asia/Kolkata" | tee /etc/timezone && dpkg-reconfigure --frontend noninteractive tzdata

RUN mkdir /images-scratch

RUN mkdir /tmp/images-cache

EXPOSE 8080

ADD build/imageresizer imageresizer
ADD config/docker.yaml config.yaml

CMD sh -c "./imageresizer -f config.yaml -t ${THREADS-8}"
