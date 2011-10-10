#include "prototls.hpp"
#include <sstream>
#include <cstdio>
using namespace std;
namespace prototls {
    Peer::Peer() :  msgSize(0), inBufPos(0) {

    }
    void Peer::setup(Socket* s_) {
        sock.reset(s_);
        msgSize = 0;
        inBuf = "";
    }
    void Peer::close() {
        sock->close();
    }

    void Peer::onInput() {
        char b[1024];

        ssize_t result = sock->recv(b, 1024);
        if (result <= 0) {
            close();
            return;
        }
        inBuf += std::string(b, result);
        if (!msgSize)
            readMessageSize();
    }
    void Peer::readMessageSize() {

        if (!msgSize && inBuf.size() - inBufPos >= 4) {
            msgSize = ntohl(*((uint32_t*) (inBuf.c_str()+inBufPos)));
            inBufPos += 4;
        }
    }
    void Peer::send(const google::protobuf::MessageLite& m) {
        size_t pos = outBuf.size();
        outBuf += "SIZE";
        if (!m.AppendToString(&outBuf)) {
            throw SocketExcept("Failed to serialize");
        }
        *((uint32_t*) (outBuf.c_str()+pos)) = htonl(outBuf.size()-pos-4);
    }
    void Peer::flush() {
        ssize_t result = sock->send(outBuf.c_str(), outBuf.size());
        if (result < outBuf.size()) {
            close();
            return;
        } 
        outBuf = "";
    }

}
