#ifndef _prototls_socket_hpp_
#define _prototls_socket_hpp_
#include <string>
#ifdef __linux__
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#endif

#ifdef WIN32
#define _WIN32_WINNT 0x0501
#include <ws2tcpip.h>
#include <winsock2.h>
#endif
namespace prototls {
    class SocketExcept : public std::exception {
        const char* msg;
    public:
        SocketExcept(const char* m) : msg(m) {}
        const char* what() const throw() { return msg; }
    };
    class Socket {
    public:
#ifdef __linux
        typedef int Fd;
#endif
#ifdef WIN32
        typedef SOCKET Fd;
#endif

    protected:
        Fd fd;
        std::string info;

    private:
        const int domain;
        const int type;
        const int protocol;
        void create();

        Socket(const Socket& p);
        Socket& operator=(const Socket& p);

    protected:

        Socket(Fd fd, const Socket& parent, const std::string& info);
        Fd _accept(std::string& info);
    public:
        static void init();
        static void deinit();

        Socket(int domain = AF_INET, int type = SOCK_STREAM, int protocol = 0);
        virtual ~Socket();

        virtual void connect(const std::string& addr, int port);
        void listen(int peers);
        void bind(int port);
        const std::string& getInfo() const;
        virtual ssize_t send(const void* buf, size_t len);
        virtual ssize_t recv(void* buf, size_t len);
        virtual int handshake();

        void setNonBlocking();
        virtual Socket* accept();

        virtual void close();
        bool isActive() const;
        Fd getFd() const;
    };
}
#endif
