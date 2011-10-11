/** prototls - Portable asynchronous client/server communications C++ library 
   
     See LICENSE for copyright information.
*/
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
    /** exception class from socket errors */
    class SocketExcept : public std::exception {
        const char* msg;
    public:
        /** stores error message pointer */
        SocketExcept(const char* m) : msg(m) {}

        /** \return the error message */
        const char* what() const throw() { return msg; }
    };
    /** Portable(?) TCP socket wrapper. Note: call Socket::init
     before using. */
    class Socket {
    public:
#ifdef __linux
        typedef int Fd;
#endif
#ifdef WIN32
        typedef SOCKET Fd;
#endif

    protected:
        /** socket descriptor */
        Fd fd;

        /** connection endpoint information */
        std::string info;

    private:
        /** socket domain, typically AF_INET */
        const int domain;

        /** socket type, typically SOCK_STREAM */
        const int type;

        /** socket protocol, typically 0 */
        const int protocol;

        /** creates a new socket (closes the existing first if open) */
        void create();

        /** declared but not defined to prevent copying */
        Socket(const Socket& p);

        /** declared but not defined to prevent copying */
        Socket& operator=(const Socket& p);
    protected:

        /** assigns socket descriptor and inherits socket type */
        Socket(Fd fd, const Socket& parent, const std::string& info);
        /** wrapper for ::accept system call. sets connection
          endpoint information
          \return a socket or -1 if error */
        Fd _accept(std::string& info);
    public:
        /** system specific socket communication init */
        static void init();

        /** cleans up system specific socket communication */
        static void deinit();

        /** sets socket type */
        Socket(int domain = AF_INET, int type = SOCK_STREAM, int protocol = 0);
        /** empty destructor */
        virtual ~Socket();

        /** connects to the specified address */
        virtual void connect(const std::string& addr, int port);

        /** sets the maximum number of incoming connections that can wait
          to be accepted */
        void listen(int peers);

        /** creates a socket and sets it to listen on the specified port */
        void bind(int port);

        /** \return socket endpoint information */
        const std::string& getInfo() const;

        /** tries to send data over the socket
          \param buf pointer to the data
          \param len number of bytes to send
          \return number of bytes sent (or -1 if error) */
        ssize_t send(const void* buf, size_t len);

        /** tries to receive data from the socket
          \param buf pointer to a buffer
          \param len maximum number of bytes that can be read
          \return number of bytes read (or -1 if error)*/
        ssize_t recv(void* buf, size_t len);

        /** a convenience method to use regular sockets and TLS
          sockets interchangeably. For regular sockets, this
         code does nothing, but for TLS sockets, it performs the
         TLS handshake (see TLSSocket::handshake) */
        virtual int handshake();

        /** sets the socket non-blocking meaning that accept,
          connect, read, and write will no longer block */
        void setNonBlocking();

        /** accepts an incoming connection 
          \return a new Socket object */
        virtual Socket* accept();

        /** closes the socket */
        virtual void close();

        /** \return true, if the socket is connected */
        bool isActive() const;

        /** \return socket descriptor */ 
        Fd getFd() const;
    };
}
#endif
