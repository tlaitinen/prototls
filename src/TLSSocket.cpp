/** prototls - Portable asynchronous client/server communications C++ library 
   
     See LICENSE for copyright information.
*/
#include "prototls.hpp"
#include <cstdio>
#include <gnutls/x509.h>
#include <gcrypt.h>

using namespace std;
namespace prototls {
    GCRY_THREAD_OPTION_PTHREAD_IMPL;
    gnutls_certificate_credentials_t TLSSocket::xcred;
    gnutls_priority_t TLSSocket::priority_cache;
    gnutls_dh_params_t TLSSocket::dh_params;

    int TLSSocket::verify(VerifyResult& result) {
        const char* hostname = (const char*) gnutls_session_get_ptr (session);

        unsigned int status;
        int ret = gnutls_certificate_verify_peers2 (session, &status);
        if (ret < 0) {
            return -1;
        }

        result.distrusted    = (status & GNUTLS_CERT_INVALID);
        result.unknownIssuer = (status & GNUTLS_CERT_SIGNER_NOT_FOUND);
        result.revoked       = (status & GNUTLS_CERT_REVOKED);
        result.expired       = (status & GNUTLS_CERT_EXPIRED);
        result.inactive      = (status & GNUTLS_CERT_NOT_ACTIVATED);
        result.invalidCert   = false;
        if (gnutls_certificate_type_get (session) != GNUTLS_CRT_X509) {
            result.invalidCert = true;
            return -1;
        }

        gnutls_x509_crt_t cert;
        if (gnutls_x509_crt_init (&cert) < 0) {
            return -1;
        }

        const gnutls_datum_t *cert_list;
        unsigned int cert_list_size;
        cert_list = gnutls_certificate_get_peers (session, &cert_list_size);
        if (cert_list == NULL) {
            result.invalidCert = true;
            gnutls_x509_crt_deinit (cert);
            return -1;
        }

        if (gnutls_x509_crt_import (cert, &cert_list[0], GNUTLS_X509_FMT_DER) < 0){
            result.invalidCert = true;
            gnutls_x509_crt_deinit (cert);
            return -1;
        }

        result.hostnameMismatch = !gnutls_x509_crt_check_hostname (cert, hostname);
        gnutls_x509_crt_deinit (cert);

        return 0;
    }
    int TLSSocket::handshake() {
        int ret ;
        do {

            ret = gnutls_handshake (session);
        } while (ret == GNUTLS_E_AGAIN || ret == GNUTLS_E_INTERRUPTED );

        if (ret < 0) {
            gnutls_perror(ret);
            Socket::close();
        }
        return ret;
    }
    void TLSSocket::init(const std::string& caPath,
            const std::string& crlPath,
            const std::string& certPath,
            const std::string& keyPath) {
        gcry_control(GCRYCTL_SET_THREAD_CBS, &gcry_threads_pthread);
        gnutls_global_init();    	
        gnutls_certificate_allocate_credentials(&xcred);
        if (!caPath.empty()) {
            gnutls_certificate_set_x509_trust_file(xcred, 
                    caPath.c_str(), 
                    GNUTLS_X509_FMT_PEM);
            gnutls_certificate_set_verify_flags (xcred,
                    GNUTLS_VERIFY_ALLOW_X509_V1_CA_CRT);
        }
        if (!crlPath.empty()) {
            gnutls_certificate_set_x509_crl_file(xcred,
                    crlPath.c_str(), 
                    GNUTLS_X509_FMT_PEM);
        }
        gnutls_certificate_set_x509_key_file(xcred,
                certPath.c_str(),
                keyPath.c_str(),
                GNUTLS_X509_FMT_PEM);
        gnutls_dh_params_init(&dh_params);
        gnutls_dh_params_generate2(dh_params, 1024);
        gnutls_priority_init(&priority_cache, "NORMAL", NULL);
        gnutls_certificate_set_dh_params(xcred, dh_params);

    }
    void TLSSocket::deinit() {
        gnutls_certificate_free_credentials(xcred); 
        gnutls_global_deinit();
    }
    TLSSocket::TLSSocket(Fd fd_, const TLSSocket& parent, const std::string& info) : Socket(fd_, parent, info) {
        gnutls_init(&session, GNUTLS_SERVER);

        gnutls_priority_set(session, priority_cache);
        gnutls_credentials_set(session, GNUTLS_CRD_CERTIFICATE, xcred);
        gnutls_transport_set_ptr (session, 
                (gnutls_transport_ptr_t) fd);


    }
    TLSSocket::~TLSSocket() {
    }

    void TLSSocket::connect(const std::string& addr, int port) {
        gnutls_init(&session, GNUTLS_CLIENT);

        gnutls_priority_set(session, priority_cache);
        gnutls_credentials_set(session, GNUTLS_CRD_CERTIFICATE, xcred);
        Socket::connect(addr, port);
        gnutls_session_set_ptr(session, (void*)getInfo().c_str());
        gnutls_transport_set_ptr (session, 
                (gnutls_transport_ptr_t) fd);
    }
    TLSSocket::TLSSocket() {
    }
    ssize_t TLSSocket::send(const void* buf, size_t len) {
        return gnutls_record_send(session, buf, len);
    }
    ssize_t TLSSocket::recv(void* buf, size_t len) {
        return gnutls_record_recv(session, buf, len);
    }
    void TLSSocket::close() {
        gnutls_bye (session, GNUTLS_SHUT_RDWR);
        Socket::close();
    }
    Socket* TLSSocket::accept() {
        string inf;
        Fd connFd = _accept(inf);
        if (connFd == -1)
            throw SocketExcept("accept failed");

        return new TLSSocket(connFd, *this, inf); 
    }
}
