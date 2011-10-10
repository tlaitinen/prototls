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
    template <class PeerT>
        class Server {
            boost::scoped_ptr<Socket> sock;
            typedef std::vector< boost::shared_ptr<PeerT> > Peers;
            Peers peers;
            boost::threadpool::pool pool;
            TSDeque<Socket*> socketsReady;
            virtual void onPacket(PeerT&p) = 0;
            virtual void onJoin(PeerT&p) = 0;
            virtual void onLeave(PeerT& p) = 0;
            void handshake(Socket* sock) {
                if (sock->handshake()) {
                    return;
                }
                socketsReady.push_back(sock);
            }
            bool closed;
        public:
            Server(int threads=16) : pool(threads), closed(false) {
            }
            virtual ~Server() {
            }
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
                    select.input(sock->getFd());
                    for (typename Peers::iterator i = peers.begin(); i != peers.end(); i++) {
                        select.input((*i)->getFd());
                    }
                    if (select.select(100) == -1)
                        continue;
                    if (select.canRead(sock->getFd())) {
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


            void close() {
                closed = true;
            }
        };
}
#endif
