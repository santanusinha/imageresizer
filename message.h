#pragma once

#include <sstream>

namespace imageresizer {

class Message {

public:
    Message(const std::string &message)
        :_message(message) {
    }

    operator const std::string () const {
        std::ostringstream sout;
        sout << "{ \"message\" : \"" <<  _message << "\"}";
        return sout.str();
    }

private:
   std::string _message; 
};

} //namespace imageresizer
