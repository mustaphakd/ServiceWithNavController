//
// Created by musta on 1/9/2022.
//

#ifndef SERVICEWITHNAVCONTROLLER_HTTPDEFINITIONS_H
#define SERVICEWITHNAVCONTROLLER_HTTPDEFINITIONS_H

#include <string>
#include <map>


namespace wrsft {

    class HttpMethods {
    public:
        static const std::string GET;
        static const std::string POST;
        static const std::string DELETE;
        static const std::string PUT;
        static const std::string PATCH;
    };
/**
 * generic-message = start-line
                          *(message-header CRLF)
                          CRLF
                          [ message-body ]
        start-line      = Request-Line | Status-Line
 */

// https://stackoverflow.com/questions/28828957/enum-to-string-in-modern-c11-c14-c17-and-future-c20
// https://stackoverflow.com/questions/644629/base-enum-class-inheritance
    class Headers {
    public:
        static const std::string CacheControl;
        static const std::string Connection;
        static const std::string Date ;
        static const std::string Pragma ;
        static const std::string Trailer ;
        static const std::string TransferEncoding;
        static const std::string Upgrade ;
        static const std::string Via ;
        static const std::string Warning ;
        //gzip, brotli
        static const std::string ContentEncoding ;
        static const std::string ContentLength ;



    };

/**
 * Accept                   ; Section 14.1
 */
    class RequestHeaders : public Headers {
    public:
        static const std::string AcceptCharset ;
        static const std::string AcceptEncoding;
        static const std::string AcceptLanguage;
        static const std::string Authorization ;
        static const std::string Expect ;
        static const std::string From ;
        static const std::string Host ;
        static const std::string IfMatch;
        static const std::string Accept;
    };

/**
 *
                  //-- response
                   Accept-Ranges           ; Section 14.5
                   | Age                     ; Section 14.6
                   | ETag                    ; Section 14.19
                   | Location                ; Section 14.30
                   | Proxy-Authenticate
                   | Retry-After             ; Section 14.37
                   | Server                  ; Section 14.38
                   | Vary                    ; Section 14.44
                   | WWW-Authenticate
 */
    class ResponseHeaders : public Headers {
    public:
        const std::string AcceptRanges = "Accept-Ranges";
        const std::string Age = "Age";
        const std::string ETag = "ETag";
        const std::string Location = "Location";
        const std::string ProxyAuthenticate = "Proxy-Authenticate";
        const std::string RetryAfter = "Retry-After";
        const std::string Server = "Server";
        const std::string Vary = "Vary";
        const std::string WWWAuthenticate = "WWW-Authenticate";

    };


    class HttpMessage {

        /**  https://datatracker.ietf.org/doc/html/rfc2616#page-31
         * servers SHOULD ignore any empty
       line(s) received where a Request-Line is expected.
         */

    public:
        std::vector<std::string> getHeaders();

        std::string getHeaderValue(std::string headerKey);  // get the header value
        void setHeaderValue(std::string headerKey,
                            std::string value); //set header value for response when header(s) has not been flushed to client
        void setHeadersValue(std::map<std::string, std::string> headersValue);

        void writeBody(std::string content,
                       std::string contentType); //with defaylt value for contentType
        std::string readBody();

        void sendChunk(char *data, size_t size);

    private:
        std::map<std::string, std::string> headers;
    };

    class HttpRequest : HttpMessage {

    public:
        explicit HttpRequest()

    private:

    };

    class HttpResponse : HttpMessage {

    };

    class ParseResult {
    public:
        static const std::string OverFlow;
        static const std::string InvalidFormat;
        static const std::string Failed;

        ParseResult(std::shared_ptr<HttpRequest> request,
                    std::string message = std::string(""));

        explicit operator bool() const; //returns false if request is null_Ptr
        std::string getErrorMessage() const;

    private:
        std::shared_ptr<HttpRequest> request = nullptr;
        std::string errorMessage;

        std::string validate_message(std::string message);
    };

    class HttpMessageParser {
    public:

        ParseResult Parse(int fd);

    private:
        /**
         * real limits should be 8 k for each headerValue and 100 header max
         */
        const size_t MAXREADSIZE = 15 * 1024;
        const size_t MAXHEADERSIZE = 3 * 1024;
        const size_t MAXBODYSIZE = 12 * 1024;
        const size_t MAXBUFFERSIZE = 4 * 1024;
        const size_t MaxHeaderValueSize = 1024;
        const char CARRIAGERETURN = '\r';
        const char LINEFEED = '\n';
    };
}
#endif //SERVICEWITHNAVCONTROLLER_HTTPDEFINITIONS_H
