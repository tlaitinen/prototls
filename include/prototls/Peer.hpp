#ifndef _prototls_peer_hpp_
#define _prototls_peer_hpp_
#include <google/protobuf/message.h>
#include "prototls/Socket.hpp"
#include <boost/smart_ptr.hpp>
namespace prototls {
    /** Packet serializer on top of a Socket */
    class Peer {
        /** socket operated and owned by peer */
        boost::scoped_ptr<Socket> sock;

        /** the length of the next protobuf message if nonzero */
        size_t msgSize;

        /** buffer for incoming data */
        std::string inBuf;

        /** buffer for outgoing data */
        std::string outBuf;

        /** byte position in the incoming data buffer for parsing packets */
        int inBufPos;

        /** reads the next protobuf message size from incoming data buffer
          and sets 'msgSize' */
        void readMessageSize();
     public:

        /** initializes fields to zero */
        Peer();

        /** sets the socket to use for transferring data */
        void setup(Socket* sock);

        /** closes the socket */
        void close();

        /** \return the socket descriptor */
        int getFd() const {
            return sock->getFd();
        }

        /** \return true if the socket is set and alive */
        bool isActive() const {
            return sock.get() ? sock->isActive() : false;
        }

        /** \return socket specific address description */
        const std::string& getInfo() const {
            return sock->getInfo();
        }

        /** reads data from socket and tries to read the next message size */
        void onInput();

        /** serializes protobuf message and stores the data in the
          outgoing data buffer */
        void send(const google::protobuf::MessageLite& m);

        /** \return true, if a packet can be deserialized from the
          incoming data buffer */
        bool hasPacket() const {
            return msgSize && inBuf.size() - inBufPos >= msgSize;
        }
        /** deserializes a protobuf message of type T from the incoming
          data buffer */
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

        /** sends the outgoing data buffer and empties it */
        void flush();
    };
}
#endif
