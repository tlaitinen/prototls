# prototls - Portable asynchronous client/server communications C++ library

An easy approach to implement portable interprocess communication using Protocol Buffers for data serialization and GnuTLS for security.

## Features

 * Portable C++ TCP socket wrappers
 * C++ TLS socket wrappers using [GnuTLS](http://www.gnu.org/s/gnutls/)
 * template classes for implementing TCP socket servers and clients
 * parallel TLS handshakes using [threadpool](http://threadpool.sourceforge.net/)
 * packet serialization using [Protocol Buffers (protobuf)](http://code.google.com/apis/protocolbuffers/)

## License 

This library is distributed under the terms of [BSD license](LICENSE). However, in order to use the library, you must also accept the conditions of the licenses of [Boost](http://www.boost.org), [threadpool](http://threadpool.sourceforge.net/), [GnuTLS](http://www.gnu.org/s/gnutls/), and 
[Protocol Buffers](http://code.google.com/apis/protocolbuffers/)

## Status 

The most important features are in place and the implementation is ready for testing. We aim to maintain a version for Windows and for Linux, although we gladly accept contributions to make it run on other platforms, too. Features improving scalability may be added.

## Quick Start

### Step 1 : Write the protocol description
    package my_protocol;
    message Hello {
        required string greeting = 1;
    }
    
    message ServerMessage {
        optional Hello hello = 1;
        // ... other message types ...
    }

    message ClientMessage {
        optional Hello hello = 1;
        // ... other message types ...
    }

and run the protoc code generator:
`protoc --cpp_out=. my_protocol.proto`

### Step 2 : Write server code
```cpp
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
```

### Step 3 : Write client code
```cpp
#include "prototls.hpp"
#include "my_protocol.pb.h"
int main(int argc, char** argv) {
    prototls::Socket::init();

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
```

### Step 4 : compile and deploy!
