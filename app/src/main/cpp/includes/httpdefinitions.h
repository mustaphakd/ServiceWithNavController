//
// Created by musta on 1/9/2022.
//

#ifndef SERVICEWITHNAVCONTROLLER_HTTPDEFINITIONS_H
#define SERVICEWITHNAVCONTROLLER_HTTPDEFINITIONS_H

#include <string>
#include <map>
#include "baseService.h"
#include <openssl/ssl.h>
#include <openssl/err.h>


namespace wrsft {
//https://datatracker.ietf.org/doc/html/rfc2616#section-6.1
    //https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
    /**
     * 500 Internal Server Error
A generic error message, given when an unexpected condition was encountered and no more specific message is suitable.[61]
501 Not Implemented
The server either does not recognize the request method, or it lacks the ability to fulfil the request. Usually this implies future availability (e.g., a new feature of a web-service API).[62]
502 Bad Gateway
The server was acting as a gateway or proxy and received an invalid response from the upstream server.[63]
503 Service Unavailable
The server cannot handle the request (because it is overloaded or down for maintenance). Generally, this is a temporary state.[64]

     400 Bad Request
The server cannot or will not process the request due to an apparent client error (e.g., malformed request syntax, size too large, invalid request message framing, or deceptive request routing).[30]
401 Unauthorized (RFC 7235)
Similar to 403 Forbidden, but specifically for use when authentication is required and has failed or has not yet been provided. The response must include a WWW-Authenticate header field containing a challenge applicable to the requested resource. See Basic access authentication and Digest access authentication.[31] 401 semantically means "unauthorised",[32] the user does not have valid authentication credentials for the target resource.
Note: Some sites incorrectly issue HTTP 401 when an IP address is banned from the website (usually the website domain) and that specific address is refused permission to access a website.[citation needed]
402 Payment Required
Reserved for future use. The original intention was that this code might be used as part of some form of digital cash or micropayment scheme, as proposed, for example, by GNU Taler,[33] but that has not yet happened, and this code is not widely used. Google Developers API uses this status if a particular developer has exceeded the daily limit on requests.[34] Sipgate uses this code if an account does not have sufficient funds to start a call.[35] Shopify uses this code when the store has not paid their fees and is temporarily disabled.[36] Stripe uses this code for failed payments where parameters were correct, for example blocked fraudulent payments.[37]
403 Forbidden
The request contained valid data and was understood by the server, but the server is refusing action. This may be due to the user not having the necessary permissions for a resource or needing an account of some sort, or attempting a prohibited action (e.g. creating a duplicate record where only one is allowed). This code is also typically used if the request provided authentication by answering the WWW-Authenticate header field challenge, but the server did not accept that authentication. The request should not be repeated.
404 Not Found
The requested resource could not be found but may be available in the future. Subsequent requests by the client are permissible.

200 OK
Standard response for successful HTTP requests. The actual response will depend on the request method used. In a GET request, the response will contain an entity corresponding to the requested resource. In a POST request, the response will contain an entity describing or containing the result of the action.[8]
201 Created
The request has been fulfilled, resulting in the creation of a new resource.[9]
202 Accepted
The request has been accepted for processing, but the processing has not been completed. The request might or might not be eventually acted upon, and may be disallowed when processing occurs.[10]
203 Non-Authoritative Information (since HTTP/1.1)
The server is a transforming proxy (e.g. a Web accelerator) that received a 200 OK from its origin, but is returning a modified version of the origin's response.[11][12]
204 No Content
The server successfully processed the request, and is not returning any content.[13]

Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRL

     */

    class HttpStatus  : public std::string {
    public:
        static const HttpStatus NotImplemented; //501
        static const HttpStatus BadGateway; // 502
        static const HttpStatus ServiceUnavailable;  //503
        static const HttpStatus BadRequest; //400
        static const HttpStatus Unauthorized; //401
        static const HttpStatus PaymentRequired; //402
        static const HttpStatus Forbidden; //403
        static const HttpStatus NotFound;  //404
        static const HttpStatus OK;  //200
        static const HttpStatus Created;  //201
        static const HttpStatus Accepted; //202
        static const HttpStatus NonAuthoritativeInformation; //203
        static const HttpStatus NoContent; //204

        static HttpStatus getStatus(std::string status);
        static std::string getStatusLine(HttpStatus status);

        inline HttpStatus(std::string str){
            *this = str;
        }

        inline HttpStatus(char *data){
            std::string _data(data);
            *this = _data;
        }

        HttpStatus(int code);

    private:
        const static std::map<HttpStatus, std::string> statusCodeLines;
    };


    class HttpMethods  : public std::string {
    public:
        static const HttpMethods GET;
        static const HttpMethods POST;
        static const HttpMethods DELETE;
        static const HttpMethods PUT;
        static const HttpMethods PATCH;

        static std::string getMethod(std::string method);

        HttpMethods(std::string str){
            *this = str;
        }

        HttpMethods(char *data){
            std::string _data(data);
            *this = _data;
        }
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
    template <typename T>
    class Headers : public std::string{
    public:
        static const T CacheControl;
        static const T Connection;
        static const T Date ;
        static const T Pragma ;
        static const T Trailer ;
        static const T TransferEncoding;
        static const T Upgrade ;
        static const T Via ;
        static const T Warning ;
        //gzip, brotli
        static const T ContentEncoding ;
        static const T ContentLength ;
        /** https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Content-Type
         * Content-Type: text/html; charset=UTF-8
            Content-Type: multipart/form-data; boundary=something
         */

        /**
         * Content-Type: multipart/form-data; boundary=---------------------------974767299852498929531610575

-----------------------------974767299852498929531610575
Content-Disposition: form-data; name="description"

some text
-----------------------------974767299852498929531610575
Content-Disposition: form-data; name="myFile"; filename="foo.txt"
Content-Type: text/plain

(content of the uploaded file foo.txt)
-----------------------------974767299852498929531610575--
         */
        static const T ContentType ;


        Headers(std::string str){
            *this = str;
        }

        Headers(char *data){
            std::string _data(data);
            *this = _data;
        }


       // virtual operator std::string() { return *this; }
        virtual ~Headers(){}

    protected:
       // std::string value;
    };

/**
 * Accept                   ; Section 14.1
 */
    class RequestHeaders : public Headers<RequestHeaders> {
    public:
        static const RequestHeaders AcceptCharset ;
        static const RequestHeaders AcceptEncoding;
        static const RequestHeaders AcceptLanguage;
        static const RequestHeaders Authorization ;
        static const RequestHeaders Expect ;
        static const RequestHeaders From ;
        static const RequestHeaders Host ;
        static const RequestHeaders IfMatch;
        static const RequestHeaders Accept;


        RequestHeaders(std::string str) : Headers(str){
        }

        RequestHeaders(char *data): Headers(data){
        }

        //virtual operator std::string() { return *this; }
       // virtual operator std::string() { return value; }
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
    class ResponseHeaders : public Headers<ResponseHeaders> {
    public:
        static const ResponseHeaders AcceptRanges;
        static const ResponseHeaders Age;
        static const ResponseHeaders ETag;
        static const ResponseHeaders Location;
        static const ResponseHeaders ProxyAuthenticate;
        static const ResponseHeaders RetryAfter;
        static const ResponseHeaders Server;
        static const ResponseHeaders Vary;
        static const ResponseHeaders WWWAuthenticate;

        ResponseHeaders(std::string str) : Headers(str){}

        ResponseHeaders(char *data) : Headers(data){}


    };

    class FileDescriptor {

    public:
        FileDescriptor(int fd);
        FileDescriptor(SSL& fd);
        inline operator int() {return fd;}
        operator SSL*();

    private:
        int fd;
        void* sslFd;
    };

    class HttpMessage :  public wrsft::BaseService {

        /**  https://datatracker.ietf.org/doc/html/rfc2616#page-31
         * servers SHOULD ignore any empty
       line(s) received where a Request-Line is expected.
         */

    public:
        std::vector<std::string> getHeaders() const;
        bool hasHeader() const;


        bool doesHeaderExist(std::string headerKey) const;
        std::string getHeaderValue(std::string headerKey) const;  // get the header value
        void setHeaderValue(std::string headerKey,
                            std::string value); //set header value for response when header(s) has not been flushed to client
        void setHeadersValue(std::map<std::string, std::string> headersValue);

        void writeBody(std::string content,
                       std::string contentType); //with defaylt value for contentType

        std::string getBody() const;

        void sendChunk(FileDescriptor fd, char *data, size_t size);
        void sendHeaders(FileDescriptor fd);
        void sendBody(FileDescriptor fd);

        void setDateHeader(std::string value);
        std::string getCurrentDateForHeader();

    private:
        std::map<std::string, std::string> headers;
        std::string body;
        HttpMessage() = delete;
        bool headerEditable = true;
        bool bodyEditable = true;

        void validateHeaderEditable();
        void validateBodyEditable();

    protected:
        HttpMessage(wrsft::LoggerType logger);
        HttpMessage(wrsft::LoggerType logger, std::map<std::string, std::string> headers);
        HttpMessage(wrsft::LoggerType logger, std::map<std::string, std::string> headers, std::string body);

        virtual HttpStatus getHttpStatus();

        constexpr static size_t MAXHEADERSITEMSIZE = 3 * 1024;
    };

    class HttpRequest : protected HttpMessage {

    public:
        explicit HttpRequest(
                wrsft::LoggerType logger,
                HttpMethods method,
                std::string path,
                std::string version,
                std::map<std::string, std::string> headers,
                std::string body);

        std::string getRequestPath() const;
        float getVersion() const;
        HttpMethods getMethod() const;

    private:
        HttpMethods method;
        std::string path;
        std::string version;
    };

    class HttpResponse : protected HttpMessage {
    public:
        explicit HttpResponse(wrsft::LoggerType logger, FileDescriptor fd);

        void appendHeader(std::string key, std::string value);
        void sendFile(std::string path, std::string contentType);
        void sendBody(std::string data, std::string contentType);
        void appendBody(std::string data);
        void flush(); //httpStatus as parameter
        void sendHeaders();

        void setupStreaming();
        void sendChunk(char *data, size_t size);
        void setStatus(HttpStatus);

    private:
        FileDescriptor fileDescriptor;
        HttpStatus status;

    protected:
        HttpStatus getHttpStatus() override;
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
        inline std::shared_ptr<HttpRequest> getRequest() {return request;}

    private:
        std::shared_ptr<HttpRequest> request = nullptr;
        std::string errorMessage;

        std::string validate_message(std::string message);
    };

    class HttpMessageParser : public wrsft::BaseService{
    public:
        HttpMessageParser(const wrsft::LoggerType logger);
        ParseResult parse(FileDescriptor fd);

    private:
        /**
         * real limits should be 8 k for each headerValue and 100 header max
         */
        const size_t MAXREADSIZE = 15 * 1024;
        const size_t MAXBODYSIZE = 12 * 1024;
        const size_t MAXBUFFERSIZE = 4 * 1024;
        const size_t MaxHeaderValueSize = 1024;
        const char CARRIAGERETURN = '\r';
        const char LINEFEED = '\n';
    };


    class HttpRoutes   {
    public:
        /**
         * return specifies whether the fileDescriptor should be kept/store for further processing
         */
        typedef std::function<bool(const HttpRequest, HttpResponse)> HttpHandlerType;
        static HttpHandlerType getHandler(std::string path);
        static void setHandlers(std::map<std::string, HttpHandlerType> map);  // how to forward and avoid copying

    private:
        const static std::map<std::string, HttpHandlerType> httpHandlers;
    };

}
#endif //SERVICEWITHNAVCONTROLLER_HTTPDEFINITIONS_H
