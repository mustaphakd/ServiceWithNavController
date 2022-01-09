//
// Created by musta on 12/26/2021.
//

#ifndef SERVICEWITHNAVCONTROLLER_SSLHANDLER_H
#define SERVICEWITHNAVCONTROLLER_SSLHANDLER_H


#define BUFFERSIZE (4 * 1024)

#include "baseService.h"
#include <string>
#include <fs/filesystem.hpp>

#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/socket.h>
#include <resolv.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "epoller.h"

namespace fs = ghc::filesystem;

namespace wrsft {
    template <int port = 8088>
    class SSLHandler : public BaseService {

    public:
        SSLHandler(const LoggerType& logger, const std::string resDirectory);
       // ~ virutal SSLHandler();

    protected:
        const std::string res_directory;
        void run_handler();
        void stop_handler();

    private:
        SSL_CTX* InitServerCTX();
        int exit;
        int running;
        std::string cert = "sd";
        std::string key;
        std::weak_ptr<Epoller> weakPoller;

        static std::string getSslRuntimeErrors();

        void LoadCertificates(SSL_CTX *ctx, std::string CertFile, std::string KeyFile);

        int OpenListener();

        void ShowCerts(SSL *ssl);

        void Servlet(SSL *ssl);

        std::string Make_res_directory(const std::string rootDirectory);
    };
}
#endif //SERVICEWITHNAVCONTROLLER_SSLHANDLER_H



namespace wrsft {

    template<int port>
    SSLHandler<port>::SSLHandler(const LoggerType& logger, const std::string resDirectory)
    :  res_directory{Make_res_directory(resDirectory)}, exit{0}, running{0}, BaseService(logger) {
        logfunc("SSLHandler<port>::ctr", "start ***  *** - end");
        //fs::path rootdir =  { directory_path};
        //fs::path logdir = rootdir / "res";

        ///// uncomment below
        SSL_load_error_strings();
        ERR_load_crypto_strings();

    }

    template<int port>
    void SSLHandler<port>::run_handler() {
        logfunc("SSLHandler<port>::run_handler", "start");

        auto logCounter = 0;
        if (running) return;

        SSL_CTX *ctx;
        int server;

        ctx = InitServerCTX();                                /* initialize SSL */
        LoadCertificates(ctx, "cert.pem", "key.pem");    /* load certs */
        server = OpenListener();                /* create server socket */


        logfunc("SSLHandler<port>::run_handler", "constructing new instance of Epoller");
        //Epoller poller(server,logfunc);  // remove comment ***************
        std::shared_ptr<Epoller> poller = std::make_shared<Epoller>  (server,logfunc);  //(&poller);
        weakPoller = poller;


        logfunc("SSLHandler<port>::run_handler", "setting poller.setNewRequestHandler callback handler.");

        poller->setNewRequestHandler([&](Channel& channel){

            logfunc("SSLHandler<port>::run_handler-poller.setNewRequestHandler", "start");
            struct sockaddr_in addr;
            int len = sizeof(addr);
            SSL *ssl;

            int client = accept(channel.fd,
                                reinterpret_cast<sockaddr *>(&addr),
                                reinterpret_cast<socklen_t *>(&len));        /* accept connection as usual */
            printf("Connection: %s:%d\n",
                   inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
            ssl = SSL_new(ctx);                             /* get new SSL state with context */
            SSL_set_fd(ssl, client);                        /* set connection socket to SSL state */
            Servlet(ssl);

            logfunc("SSLHandler<port>::run_handler-poller.setNewRequestHandler", "end");
        });


        logfunc("SSLHandler<port>::run_handler", "setting poller.setExistingClientRequestHandler callback handler.");

        poller->setExistingClientRequestHandler([&](Channel& channel){

            logfunc("SSLHandler<port>::run_handler-poller.setExistingClientRequestHandler", "callback start-stop");
        });

        //after listening on socket, set running to true
        running = 1;
/*
        while (true)
        {
            //if(logCounter++ >= 4200000)
            {
                logfunc("SSLHandler<port>::run_handler", "running....");
                //logCounter = -100;
            }

            struct sockaddr_in addr;
            int len = sizeof(addr);
            SSL *ssl;

            int client = accept(server,
                                reinterpret_cast<sockaddr *>(&addr),
                                reinterpret_cast<socklen_t *>(&len));        / * accept connection as usual * /
            printf("Connection: %s:%d\n",
                   inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
            ssl = SSL_new(ctx);                             /* get new SSL state with context * /
            SSL_set_fd(ssl, client);                        /* set connection socket to SSL state * /
            Servlet(ssl);                                    /* service connection * /

            if(exit){
                logfunc("SSLHandler<port>::run_handler", "exit requested breaking out.");
                close(server);                                        /* close server socket * /
                SSL_CTX_free(ctx);
                exit = 0;
                break;
            }
        } */
        logfunc("SSLHandler<port>::run_handler", "poller will start polling.....");
        poller->startPolling();

        poller.reset();
        poller = nullptr;

        logfunc("SSLHandler<port>::run_handler", "exit requested. Closing server and breaking out.");
        close(server);                                        /* close server socket */
        SSL_CTX_free(ctx);
        exit = 0;

        running = 0;
        logfunc("\"SSLHandler<port>::run_handler", "end");
    }

    /*---------------------------------------------------------------------*/
    /*--- InitServerCTX - initialize SSL server  and create context     ---*/
    /*---------------------------------------------------------------------*/

    template<int port>
    SSL_CTX *SSLHandler<port>::InitServerCTX()
    {
        logfunc("\"SSLHandler<port>::InitServerCTX", "start");
        //SSL_METHOD *method;
        SSL_CTX *ctx;

        OpenSSL_add_all_algorithms();        /* load & register all cryptos, etc. */
        SSL_load_error_strings();            /* load all error messages */
        auto method = TLS_method();   //SSLv2_server_method();        /* create new server-method instance */
        ctx = SSL_CTX_new(method);            /* create new context from method */
        if ( ctx == NULL )
        {
            std::string error = getSslRuntimeErrors();
            logfunc("\"SSLHandler<port>::InitServerCTX", error);
            abort();
        }

        logfunc("\"SSLHandler<port>::InitServerCTX", "end");
        return ctx;
    }


/*---------------------------------------------------------------------*/
/*--- LoadCertificates - load from files.                           ---*/
/*---------------------------------------------------------------------*/


    template<int port>
    void SSLHandler<port>::LoadCertificates(SSL_CTX* ctx, std::string CertFile, std::string KeyFile)
    {
        logfunc("\"SSLHandler<port>::LoadCertificates", "start");

        fs::path respath =  { res_directory};
        fs::path certFullPath = respath / CertFile;
        fs::path keyFullPath = respath / KeyFile;


        logfunc("\"SSLHandler<port>::LoadCertificates", "cert file path: " + certFullPath.string());
        logfunc("\"SSLHandler<port>::LoadCertificates", "key file path: " + keyFullPath.string());

        /* set the local certificate from CertFile */
        if ( SSL_CTX_use_certificate_file(ctx, certFullPath.c_str(), SSL_FILETYPE_PEM) <= 0 )
        {
            //ERR_print_errors_fp(stderr);
            std::string error = getSslRuntimeErrors();
            logfunc("\"SSLHandler<port>::LoadCertificates", error);
            abort();
        }
        /* set the private key from KeyFile (may be the same as CertFile) */
        if ( SSL_CTX_use_PrivateKey_file(ctx, keyFullPath.c_str(), SSL_FILETYPE_PEM) <= 0 )
        {
            std::string error = getSslRuntimeErrors();
            logfunc("\"SSLHandler<port>::LoadCertificates", error);
            abort();
        }
        /* verify private key */
        if ( !SSL_CTX_check_private_key(ctx) )
        {
            fprintf(stderr, "Private key does not match the public certificate\n");
            abort();
        }

        logfunc("\"SSLHandler<port>::LoadCertificates", "end");
    }

    template<int port>
    std::string SSLHandler<port>::getSslRuntimeErrors() {
        char errorText[100];
        sprintf(errorText, "%s", ERR_error_string(ERR_get_error(), NULL));

        std::string error {errorText};
        return error;
    }


/*---------------------------------------------------------------------*/
/*--- OpenListener - create server socket                           ---*/
/*---------------------------------------------------------------------*/

    template<int port>
    int  SSLHandler<port>::OpenListener()
    {
        logfunc("\"SSLHandler<port>::OpenListener", "start");

        int sd;
        struct sockaddr_in addr;

        sd = socket(PF_INET, SOCK_STREAM, 0);
        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;

        int on = 1;
        if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
            close(sd);
            logfunc("\"SSLHandler<port>::OpenListener","could not setsockopt");
            abort();
        }

        if (bind(sd, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr)) != 0 )
        {
            logfunc("\"SSLHandler<port>::OpenListener","can't bind port");
            abort();
        }
        if ( listen(sd, 10) != 0 )
        {
            logfunc("\"SSLHandler<port>::OpenListener","Can't configure listening port");
            abort();
        }

        logfunc("\"SSLHandler<port>::OpenListener", "end");
        return sd;
    }


/*---------------------------------------------------------------------*/
/*--- ShowCerts - print out certificates.                           ---*/
/*---------------------------------------------------------------------*/

    template<int port>
    void  SSLHandler<port>::ShowCerts(SSL* ssl)
    {
        logfunc("\"SSLHandler<port>::ShowCerts", "start");
        X509 *cert;
        char *line;

        cert = SSL_get_peer_certificate(ssl);    /* Get certificates (if available) */
        if ( cert != NULL )
        {
            logfunc("\"SSLHandler<port>::ShowCerts", "Server certificates:\n");
            line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
            logfunc("\"SSLHandler<port>::ShowCerts", "Subject: " + std::string (line));
            free(line);
            line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
            logfunc("\"SSLHandler<port>::ShowCerts", "Issuer: " + std::string (line));
            free(line);
            X509_free(cert);
        }
        else
            logfunc("\"SSLHandler<port>::ShowCerts", "No certificates.\n");

        logfunc("\"SSLHandler<port>::ShowCerts", "end");
    }


/*---------------------------------------------------------------------*/
/*--- Servlet - SSL servlet (contexts can be shared)                ---*/
/*---------------------------------------------------------------------*/

    template<int port>
    void  SSLHandler<port>::Servlet(SSL* ssl)    /* Serve the connection -- threadable */
    {
        logfunc("\"SSLHandler<port>::Servlet", "start");

        char buf[1024];
        char reply[1024];
        int sd, bytes;
        std::string HTMLecho="<html><body>jhjljlkjk</body></html>\n\n";


        size_t bytes_received = -1;
        char buffer[BUFFERSIZE] = { 0 };

        std::string format = "HTTP/1.1 200 OK\r\n"
                             "Location: localhost:8088\r\n"
                             "Server : socket\r\n"
                             "X-Powered-By: native c++\r\n"
                             "Date: %a, %e %b %G %T GMT\r\n";
                             //"Content-Length: 0\r\n\r\n";

        time_t rawtime = (0);
        struct tm gmt_time = {0};
        time(&rawtime);
        gmtime_r(&rawtime, &gmt_time);
        memset(buffer, 0, BUFFERSIZE);
        strftime(buffer, BUFFERSIZE, format.data(), &gmt_time);
        std::string responseHeader(buffer);


        ////logfunc("\"SSLHandler<port>::Servlet", "header: " + responseHeader);


        memset(buffer, 0, BUFFERSIZE);
        auto n = sprintf (buffer, "Content-Length: %d\r\n\r\n", HTMLecho.length() + 1);
        std::string responseBody {buffer};
        //responseBody += HTMLecho;


        /////logfunc("\"SSLHandler<port>::Servlet", "body: " + responseBody);

        std::stringstream responseStream;
        responseStream << responseHeader << responseBody << HTMLecho;

        std::string response {responseStream.str()};

        /////logfunc("\"SSLHandler<port>::Servlet", "full response: " + response);


        //bytes_received = send(sdconn, responseHeader.c_str(), responseHeader.size(), 0);

        if ( !SSL_accept(ssl) )                    /* do SSL-protocol accept */
        {
            std::string error = getSslRuntimeErrors();
            logfunc("\"SSLHandler<port>::Servlet", error);
        }
        else
        {
            ShowCerts(ssl);                                /* get any certificates */
            bytes = SSL_read(ssl, buf, sizeof(buf));    /* get request */

            logfunc("\"SSLHandler<port>::Servlet", "bytes read: " + std::to_string(bytes));

            if ( bytes > 0 )
            {
                buf[bytes] = 0;
               // printf("Client msg: \"%s\"\n", buf);
               std::string  dataRead {buf};
                logfunc("\"SSLHandler<port>::Servlet", "data read: " + dataRead);

                logfunc("\"SSLHandler<port>::Servlet", "writing response to stream");
                //sprintf(reply, HTMLecho.c_str(), buf);            /* construct reply */
                SSL_write(ssl, (void *) response.c_str(), response.length() + 1);
                //SSL_write(ssl, reply, strlen(reply));    /* send reply */
            }
            else
            {
                std::string error = getSslRuntimeErrors();
                logfunc("\"SSLHandler<port>::Servlet", error);
            }
        }

        sd = SSL_get_fd(ssl);                            /* get socket connection */
        SSL_free(ssl);                                    /* release SSL state */
        close(sd);                                        /* close connection */
        logfunc("\"SSLHandler<port>::Servlet", "end");
    }

    template<int port>
    std::string SSLHandler<port>::Make_res_directory(const std::string rootDirectory) {

        fs::path rootdir =  { rootDirectory};
        fs::path resdir = rootdir; // / "res";

        return resdir;
    }

    template<int port>
    void SSLHandler<port>::stop_handler() {
        logfunc("\"SSLHandler<port>::stop_handler", "start");

        if( std::shared_ptr<Epoller> sharedPoller = weakPoller.lock()) {
            logfunc("\"SSLHandler<port>::stop_handler", "weakPoller is defined. will invoke close");
            sharedPoller->close();
        }

        logfunc("\"SSLHandler<port>::stop_handler", "end");
    }
}