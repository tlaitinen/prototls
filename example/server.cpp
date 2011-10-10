
#include "prototls.hpp"
#include "my_protocol.pb.h"

class MyPeer : public prototls::Peer {
public:
        // additional connection-specific state and routines
};

class MyServer : public prototls::Server<MyPeer> {
    public:
        // start 8 threads to perform TLS handshakes simultaneously
        MyServer() : prototls::Server<MyPeer>(8) {}

        void onPacket(MyPeer&p) {
            my_protocol::ClientMessage msg;
            p.recv(msg);
            std::cout << msg.hello().greeting() << std::endl;

            // handle the message 
            my_protocol::ServerMessage reply;
            p.send(reply);
            p.flush();
        }
        void onJoin(MyPeer&p) {
            my_protocol::ServerMessage msg;
            my_protocol::Hello* hello = msg.mutable_hello();
            hello->set_greeting("Welcome!");
            p.send(msg);
            p.flush();      
        }
        void onLeave(MyPeer&p) {
            // clean up
        }
};

// server main function
int main(int argc, char** argv) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    prototls::Socket::init();
    prototls::TLSSocket::init("", "", "cert.pem", "key.pem");

    MyServer server;

    // encryption on, serve on port 1234 for at most 1024 peers
    server.serve(true, 1234, 1024);  
    return 0;
}
