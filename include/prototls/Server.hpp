#ifndef _prototls_server_hpp_
#define _prototls_server_hpp_
#include "prototls/Socket.hpp"
#include "prototls/TLSSocket.hpp"
#include "prototls/Peer.hpp"
#include "prototls/TSDeque.hpp"
#include <boost/smart_ptr.hpp>
#include "prototls/Select.hpp"
#include <boost/thread/mutex.hpp>
#include "boost/threadpool.hpp"
#include <sstream>
#include <cstdio>
#include <iostream>
#include <vector>
#include <deque>
namespace prototls {
    /** Server class template for implementing 
        asynchronous servers that send and receive protobuf messages
        with/without encrypted communication. TLS handshakes
        are performed in separate threads in order to avoid blocking 
        for long time. */
    template <class PeerT>
        class Server {
            /** socket that accepts connections */
            boost::scoped_ptr<Socket> sock;

            /** connected peers are stored in a vector holding
              boost shared pointers to simplify memory management */
            typedef std::vector< boost::shared_ptr<PeerT> > Peers;

            /** connected peers */
            Peers peers;

            /** a pool of threads for TLS handshakes */
            boost::threadpool::pool pool;

            /** thread safe deque that holds fresh TLS sockets */
            TSDeque<Socket*> socketsReady;

            /** this method is called when a peer can read a packet */
            virtual void onPacket(PeerT&p) = 0;

            /** this method is called when a new peer joins (after TLS
              handshake if encrypted communication is used) */
            virtual void onJoin(PeerT&p) = 0;

            /** this method is called when a peer leaves */
            virtual void onLeave(PeerT& p) = 0;

            /** performs TLS handshake on the socket and
             adds it to the list of ready sockets */
            void handshake(Socket* sock) {
                if (sock->handshake()) {
                    return;
                }
                socketsReady.push_back(sock);
            }
            /** flag marking that the server has been closed */
            bool closed;
        public:
            /** initializes a number of threads
              that will handle parallel TLS handshakes */
            Server(int threads) : pool(threads), closed(false) {
            }

            /** empty destructor */
            virtual ~Server() {
            }

            /** accepts and keeps track of connections. reads data
              from connected peers and notifies through virtual methods
              if packets can be deserialized 
             \param tls use GnuTLS for encryption 
             \param port listen for incoming connections at this port
             \param maxPeers maximum number of connected peers */
            void serve(bool tls, int port, int maxPeers) {
                if (tls)
                    sock.reset(new TLSSocket());
                else
                    sock.reset(new Socket());
                sock->setNonBlocking();
                sock->bind(port);
                sock->listen(maxPeers);
                Select select;
                /* Wait for a peer, send data and term */
                while (!closed)
                {
                    select.reset();
                    if (peers.size() < maxPeers)
                        select.input(sock->getFd());
                    for (typename Peers::iterator i = peers.begin(); i != peers.end(); i++) {
                        select.input((*i)->getFd());
                    }
                    if (select.select(100) == -1)
                        continue;
                    if (select.canRead(sock->getFd()) 
                            && peers.size() < maxPeers) {
                        try {
                            Socket* csock = sock->accept();

                            if (tls)  {
                                boost::threadpool::schedule(pool, 
                                        boost::bind(&Server::handshake, this, csock));
                            } else {
                                peers.push_back(boost::shared_ptr<PeerT>(new PeerT()));
                                csock->setNonBlocking();
                                peers.back()->setup(csock);
                                onJoin(*peers.back());

                            }
                        } catch (SocketExcept& e) {
                            std::cerr << e.what() << std::endl;
                        }
                    }
                    if (tls)  {
                        Socket* sock = NULL;
                        socketsReady.try_pop_front(sock);
                        if (sock) {
                            sock->setNonBlocking();
                            peers.push_back(boost::shared_ptr<PeerT>(new PeerT()));
                            peers.back()->setup(sock);
                            onJoin(*peers.back());
                        }
                    }
                    for (size_t i = 0; i < peers.size(); i++) {
                        boost::shared_ptr<PeerT>& p = peers[i];
                        if (select.canRead(p->getFd()))
                            p->onInput();
                        while (p->hasPacket()) {
                            onPacket(*p);
                        }
                    }
                    // collect dead peers
                    size_t count = peers.size();
                    for (size_t i = 0; i < count; ) {
                        if (!peers[i]->isActive()) {
                            onLeave(*peers[i]);

                            peers[i] = peers[peers.size() - 1];
                            count--;
                        } else
                            i++;
                    }
                    if (count < peers.size()) {
                        peers.resize(count);
                    }

                }
            }


            /** sets the closed-bit to true, and the running 
              Server::serve method will exit */
            void close() {
                closed = true;
            }
        };
}
#endif
