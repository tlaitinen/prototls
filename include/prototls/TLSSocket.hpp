#ifndef _prototls_tlssocket_hpp_
#define _prototls_tlssocket_hpp_
#include "prototls/Socket.hpp"
#include <gnutls/gnutls.h>
namespace prototls {
    /** Simple GnuTLS wrapper class on top of a (TCP) Socket.
      Note: call TLSSocket::init before using */
    class TLSSocket : public Socket {
        /** data structure for holding certificate information */
        static gnutls_certificate_credentials_t xcred;

        /** a prioritized list of cipher suite names */
        static gnutls_priority_t priority_cache;

        /**  the Diffie Hellman parameters for a certificate server to use */
        static gnutls_dh_params_t dh_params;

        /** GnuTLS connection state */
        gnutls_session_t session;

        /** declared but not defined to prevent copying */
        TLSSocket(const TLSSocket& t);

        /** declared but not defined to prevent copying */
        TLSSocket& operator=(const TLSSocket& t);

        /** initializes GnuTLS connection state in a server
          mode on the socket descriptor 'fd' 
          \param fd socket descriptor
          \param parent "parent" socket, inherits socket type 
          \param info connection endpoint information */
        TLSSocket(Fd fd, const TLSSocket& parent, const std::string& info);

    public:
        /** container for storing the result of verification the endpoint's
          certificate */
        struct VerifyResult {
            /** certificate is not trusted */
            bool distrusted;

            /** certificate has unknown issuer */
            bool unknownIssuer;

            /** certificate has been revoked */
            bool revoked;

            /** certificate has expired */
            bool expired;

            /** certificate is not active */
            bool inactive;
            
            /** certificate is invalid */
            bool invalidCert;

            /** hostname does not match */
            bool hostnameMismatch;
        };

        /** initializes GnuTLS global data structures (must be called!)
          \param caPath   path to certificate authority certificate file 
          \param crlPath  path to certificate revocation list file
          \param certPath path to own certificate file
          \param keyPath  path to private key file */
        static void init(const std::string& caPath, 
                const std::string& crlPath,
                const std::string& certPath,
                const std::string& keyPath);

        /** cleans up GnuTLS global data structures */
        static void deinit();

        /** empty constructor */
        TLSSocket();

        /** empty desctrucotr */
        ~TLSSocket();

        /** connects to the specified address (GnuTLS client mode) */
        void connect(const std::string& addr, int port);

        /** verifies the endpoint's certificate
          \return -1 if an error occurs, 0 otherwise */
        int verify(VerifyResult& result);

        /** performs TLS handshake
          \return nonzero if error, zero otherwise */
        int handshake();

        /** tries to send data over the TLS socket
          \param buf pointer to the data
          \param len number of bytes to send
          \return number of bytes sent (or -1 if error) */
        ssize_t send(const void* buf, size_t len);

        /** tries to receive data from the TLS socket
          \param buf pointer to a buffer
          \param len maximum number of bytes that can be read
          \return number of bytes read (or -1 if error)*/
        ssize_t recv(void* buf, size_t len);

        /** accepts a incoming connection 
          \return a TLSSocket in server mode */
        Socket* accept();

        /** closes connection */
        void close();
    };
}
#endif
