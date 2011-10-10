#include "prototls.hpp"
#include "my_protocol.pb.h"
int main(int argc, char** argv) {
    prototls::Socket::init();

    // we use a certificate authority's certificate to verify our shopping server
    prototls::TLSSocket::init("ca-cert.pem", "", "", "");

    prototls::TLSSocket* sock = new prototls::TLSSocket();
    sock->connect("localhost", 1234);

    if (sock->handshake()) {
        std::cerr << "Handshake failed" << std::endl;
        return -1;
    }

    prototls::TLSSocket::VerifyResult res;
    if (sock->verify(res)) {
        std::cerr << " verification failed !" << std::endl;
        return -1;
    } else {
        if (res.distrusted)
            std::cout << "certificate not trusted" << std::endl;
        if (res.unknownIssuer)
            std::cout << "certificate has unknown issuer" << std::endl;
        if (res.revoked)
            std::cout << "certificate has been revoked" << std::endl;
        if (res.expired)
            std::cout << "certificate has expired" << std::endl;
        if (res.inactive)
            std::cout << "certificate is not active" << std::endl;
        if (res.invalidCert)
            std::cout << "certificate is invalid" << std::endl;
        if (res.hostnameMismatch)
            std::cout << "hostname does not match" << std::endl;
    } 

    prototls::Peer peer;
    peer.setup(sock);
    prototls::Select sel;
    while (1) {
        sel.reset();
        sel.input(peer.getFd());
        sel.select(1000);
        if (sel.canRead(peer.getFd())) {
            peer.onInput();
            while (peer.hasPacket()) {
                my_protocol::ServerMessage msg;
                peer.recv(msg);
                if (msg.has_hello()) {
                    std::cout << msg.hello().greeting() << std::endl;

                    my_protocol::ClientMessage cmsg; 
                    *cmsg.mutable_hello()->mutable_greeting() = "Thanks!";
                    peer.send(cmsg);
                    peer.flush();
                } 
                // handle other packet types
            }
        }
    }
    return 0;
}
