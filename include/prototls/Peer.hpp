#ifndef _prototls_peer_hpp_
#define _prototls_peer_hpp_
#include <google/protobuf/message.h>
#include "prototls/Socket.hpp"
#include <boost/smart_ptr.hpp>
namespace prototls {
    class Peer {
        boost::scoped_ptr<Socket> sock;
        size_t msgSize;
        std::string inBuf, outBuf;
        int inBufPos;

        void readMessageSize();
     public:
        Peer();

        void setup(Socket* sock);
        void close();
        int getFd() const {
            return sock->getFd();
        }
        bool isActive() const {
            return sock.get() ? sock->isActive() : false;
        }
        const std::string& getInfo() const {
            return sock->getInfo();
        }
        void onInput();
        void send(const google::protobuf::MessageLite& m);
        bool hasPacket() const {
            return msgSize && inBuf.size() - inBufPos >= msgSize;
        }
        template <class T>
            void recv(T& m) {
                m.ParseFromArray(inBuf.c_str() + inBufPos, msgSize);
                inBufPos += msgSize;
                msgSize = 0;
                if (inBufPos == inBuf.size()) {
                    inBufPos = 0;
                    inBuf.clear();
                }
                readMessageSize();
            }
        void flush();

    };
}
#endif
