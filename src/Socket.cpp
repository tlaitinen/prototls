/** prototls - Portable asynchronous client/server communications C++ library 
   
     See LICENSE for copyright information.
*/
#include "prototls.hpp"
#include <cstring>
#include <cstdio>
using namespace std;

namespace prototls {
    void Socket::init() {
#ifdef WIN32
        WSADATA wsaData;
        WORD version;
        int error;

        version = MAKEWORD( 2, 0 );

        error = WSAStartup( version, &wsaData );

        /* check for error */
        if ( error != 0 )
        {
            /* error occured */
            throw SocketExcept("WSAStartup failed");
        }

        /* check for correct version */
        if ( LOBYTE( wsaData.wVersion ) != 2 ||
                HIBYTE( wsaData.wVersion ) != 0 )
        {
            /* incorrect WinSock version */
            WSACleanup();
            throw SocketExcept("Incorrect WinSock version");
        }

        /* WinSock has been initialized */
#endif
    }
    void Socket::deinit() {
#ifdef WIN32
        WSACleanup();
#endif
    }
    Socket::Socket(Fd fd_, const Socket& parent, const std::string& info_)
        : fd(fd_), domain(parent.domain),
        type(parent.type),
        protocol(parent.protocol), info(info_) {
        }
    Socket::Socket(int domain_, int type_, int protocol_)
        : fd(0), domain(domain_), type(type_), protocol(protocol_) {
        } 
    Socket::~Socket() {
    }
    void Socket::close() {
        if (fd) {
            ::shutdown(fd, 2);
#ifdef WIN32
            closesocket(fd);
#endif
#ifdef __linux
            ::close(fd);
#endif
            fd = 0;

        }
    }
    void Socket::setNonBlocking() {
        int flags;
#ifdef __linux__
        /* If they have O_NONBLOCK, use the Posix way to do it */
        /* Fixme: O_NONBLOCK is defined but broken on SunOS 4.1.x and AIX 3.2.5. */
        if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
            flags = 0;
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#endif
#ifdef WIN32
        u_long iMode=1;
        ioctlsocket(fd,FIONBIO,&iMode);
#endif

    }
    void Socket::create() {
        if (fd)
            close();
        fd = ::socket(domain, type, protocol);
    }
    const std::string& Socket::getInfo() const {
        return info;
    }

    void Socket::connect(const std::string& addr, int port) {
        if (fd)
            close();
        if (!fd)
            create();

        struct addrinfo hints;
        struct addrinfo* result;
        struct addrinfo* rp;
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = domain;
        hints.ai_socktype = type;
        hints.ai_flags = 0;
        hints.ai_protocol = protocol;
        info = addr + ":" + toString(port);
        int s = getaddrinfo(addr.c_str(), toString(port).c_str(), &hints, &result);
        if (s != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
            return;
        }
        if (result) {
            rp = result;
            if (::connect(fd, rp->ai_addr, rp->ai_addrlen) < 0) {
                freeaddrinfo(result);
                throw SocketExcept("Could not connect");
            }
        }

        freeaddrinfo(result);

    }
    void Socket::listen(int peers) {
        ::listen(fd, peers);
    }
    void Socket::bind(int port) {

        create();
        struct sockaddr_in servaddr;
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = domain;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(port);
        int optval = 1;
        setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, (char *) &optval,
                sizeof (int));
        if (::bind(fd, (struct sockaddr *) &servaddr, sizeof(servaddr))) {
            perror("bind ");
            throw SocketExcept("Cannot bind");
        }

    }
    ssize_t Socket::send(const void* buf, size_t len) {
        return ::send(fd, (const char*) buf, len, 0);
    }
    ssize_t Socket::recv(void* buf, size_t len) {
        return ::recv(fd, (char*)buf, len, 0);
    }
    int Socket::handshake() {
        return 0;
    }
    Socket::Fd Socket::_accept(string& i) {
        struct sockaddr_storage addr;
        socklen_t len = sizeof(addr);
        Fd connFd = ::accept(fd, (struct sockaddr*) &addr, &len);
        int port;
        char ipstr[INET6_ADDRSTRLEN];
        if (addr.ss_family == AF_INET) {
            struct sockaddr_in *s = (struct sockaddr_in *)&addr;
            port = ntohs(s->sin_port);
            inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
        } else { // AF_INET6
            struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
            port = ntohs(s->sin6_port);
            inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
        }
        i = string(ipstr) + ":" + toString(port);
        return connFd;
    }
    Socket* Socket::accept() {
        string inf;
        Fd connFd = _accept(inf);
        if (connFd == -1) {
            perror("accept");
            throw SocketExcept("accept failed");
        }
        return new Socket(connFd, *this, inf);
    }
    bool Socket::isActive() const {
        return fd > 0;
    }
    Socket::Fd Socket::getFd() const {
        return fd;
    }
}
