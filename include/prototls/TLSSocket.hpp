#ifndef _prototls_tlssocket_hpp_
#define _prototls_tlssocket_hpp_
#include "prototls/Socket.hpp"
#include <gnutls/gnutls.h>
namespace prototls {
    class TLSSocket : public Socket {
        static gnutls_certificate_credentials_t xcred;
        static gnutls_priority_t priority_cache;
        static gnutls_dh_params_t dh_params;
        gnutls_session_t session;

        TLSSocket(const TLSSocket& t);
        TLSSocket& operator=(const TLSSocket& t);
        TLSSocket(Fd fd, const TLSSocket& parent, const std::string& info);

    public:
        struct VerifyResult {
            bool distrusted;
            bool unknownIssuer;
            bool revoked;
            bool expired;
            bool inactive;
            bool invalidCert;
            bool hostnameMismatch;
        };

        static void init(const std::string& caPath, 
                const std::string& crlPath,
                const std::string& certPath,
                const std::string& keyPath);
        static void deinit();
        TLSSocket();
        ~TLSSocket();

        void connect(const std::string& addr, int port);
        int verify(VerifyResult& result);
        int handshake();
        ssize_t send(const void* buf, size_t len);
        ssize_t recv(void* buf, size_t len);
        Socket* accept();

        void close();
    };
}
#endif
