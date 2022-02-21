//
// Created by musta on 1/23/2022.
//

#include "httpdefinitions.h"
#include <string>
#include <unistd.h>
#include <sstream>
#include "stringHelper.h"
#include <fs/filesystem.hpp>


namespace fs = ghc::filesystem;

const wrsft::HttpStatus wrsft::HttpStatus::NotImplemented = "501";
const wrsft::HttpStatus wrsft::HttpStatus::BadGateway = "502";
const wrsft::HttpStatus wrsft::HttpStatus::ServiceUnavailable = "503";
const wrsft::HttpStatus wrsft::HttpStatus::BadRequest = "400";
const wrsft::HttpStatus wrsft::HttpStatus::Unauthorized = "401";
const wrsft::HttpStatus wrsft::HttpStatus::PaymentRequired = "402";
const wrsft::HttpStatus wrsft::HttpStatus::Forbidden = "403";
const wrsft::HttpStatus wrsft::HttpStatus::NotFound = "404";
const wrsft::HttpStatus wrsft::HttpStatus::OK = "200";
const wrsft::HttpStatus wrsft::HttpStatus::Created = "201";
const wrsft::HttpStatus wrsft::HttpStatus::Accepted = "202";
const wrsft::HttpStatus wrsft::HttpStatus::NonAuthoritativeInformation = "203";
const wrsft::HttpStatus wrsft::HttpStatus::NoContent = "204";


const std::map<wrsft::HttpStatus, std::string> wrsft::HttpStatus::statusCodeLines = {
        {wrsft::HttpStatus::NotImplemented, "501 Not implemented"},
        {wrsft::HttpStatus::BadGateway, "502 Bad Gateway"},
        {wrsft::HttpStatus::ServiceUnavailable, "503 Service Unavailable"},
        {wrsft::HttpStatus::BadRequest, "400 Bad Request"},
        {wrsft::HttpStatus::Unauthorized, "401 Unauthorized"},
        {wrsft::HttpStatus::PaymentRequired, "402 Payment Required"},
        {wrsft::HttpStatus::Forbidden, "403 Forbidden"},
        {wrsft::HttpStatus::NotFound, "404 Not Found"},
        {wrsft::HttpStatus::OK, "200 OK"},
        {wrsft::HttpStatus::Created, "201 Created"},
        {wrsft::HttpStatus::Accepted, "202 Accepted"},
        {wrsft::HttpStatus::NonAuthoritativeInformation, "203 Non Authoritative Information"},
        {wrsft::HttpStatus::NoContent, "204 No Content"}
};

std::string wrsft::HttpStatus::getStatusLine(wrsft::HttpStatus status) {
    auto value =  wrsft::HttpStatus::statusCodeLines.at(status);
    return value;
}

wrsft::HttpStatus wrsft::HttpStatus::getStatus(std::string status) {
    auto found =  wrsft::HttpStatus::statusCodeLines.find(status) != wrsft::HttpStatus::statusCodeLines.end();

    if(!found)
        throw std::invalid_argument("Invalid status: " + status);

    return status;
}

wrsft::HttpStatus::HttpStatus(int code) : wrsft::HttpStatus::HttpStatus(std::to_string(code)){
}


const wrsft::HttpMethods wrsft::HttpMethods::GET = "GET";
const wrsft::HttpMethods wrsft::HttpMethods::POST = "POST";
const wrsft::HttpMethods wrsft::HttpMethods::DELETE = "DELETE";
const wrsft::HttpMethods wrsft::HttpMethods::PUT = "PUT";
const wrsft::HttpMethods wrsft::HttpMethods::PATCH = "PATCH";

std::string wrsft::HttpMethods::getMethod(std::string method) {

    std::transform(method.begin(), method.end(), method.begin(),
                   [](unsigned char c){ return std::tolower(c); } // correct
    );

    if(method == "post") return wrsft::HttpMethods::POST;
    if(method == "delete") return wrsft::HttpMethods::DELETE;
    if(method == "put") return wrsft::HttpMethods::PUT;
    if(method == "patch") return wrsft::HttpMethods::PATCH;
    if(method == "get") return wrsft::HttpMethods::GET;

    return method;
}

/**
 * size limit reached
 */
const std::string wrsft::ParseResult::OverFlow = "overflow";
const std::string wrsft::ParseResult::InvalidFormat = "format";
const std::string wrsft::ParseResult::Failed = "failed";

template <typename T>
const T wrsft::Headers<T>::CacheControl = "Cache-Control";
template <typename T>
const T wrsft::Headers<T>::Connection = "Connection";
template <typename T>
const T wrsft::Headers<T>::Date = "Date";
template <typename T>
const T wrsft::Headers<T>::Pragma = "Pragma";
template <typename T>
const T wrsft::Headers<T>::Trailer = "Trailer";
template <typename T>
const T wrsft::Headers<T>::TransferEncoding = "TransferEncoding";
template <typename T>
const T wrsft::Headers<T>::Upgrade = "Upgrade";
template <typename T>
const T wrsft::Headers<T>::Via = "Via";
template <typename T>
const T wrsft::Headers<T>::Warning = "Warning";
template <typename T>
const T wrsft::Headers<T>::ContentEncoding = "Content-Encoding";
template <typename T>
const T wrsft::Headers<T>::ContentLength = "Content-Length";
template <typename T>
const T wrsft::Headers<T>::ContentType = "Content-Type";

const wrsft::RequestHeaders wrsft::RequestHeaders::AcceptCharset = "Accept-Charset";
const wrsft::RequestHeaders wrsft::RequestHeaders::AcceptEncoding = "Accept-Encoding";
const wrsft::RequestHeaders wrsft::RequestHeaders::AcceptLanguage = "Accept-Language";
const wrsft::RequestHeaders wrsft::RequestHeaders::Authorization = "Authorization";
const wrsft::RequestHeaders wrsft::RequestHeaders::Expect = "Expect";
const wrsft::RequestHeaders wrsft::RequestHeaders::From = "From";
const wrsft::RequestHeaders wrsft::RequestHeaders::Host = "Host";
const wrsft::RequestHeaders wrsft::RequestHeaders::IfMatch = "If-Match";
const wrsft::RequestHeaders wrsft::RequestHeaders::Accept = "Accept";


const wrsft::ResponseHeaders wrsft::ResponseHeaders::AcceptRanges = "Accept-Ranges";
const wrsft::ResponseHeaders wrsft::ResponseHeaders::Age = "Age";
const wrsft::ResponseHeaders wrsft::ResponseHeaders::ETag = "ETag";
const wrsft::ResponseHeaders wrsft::ResponseHeaders::Location = "Location";
const wrsft::ResponseHeaders wrsft::ResponseHeaders::ProxyAuthenticate = "Proxy-Authenticate";
const wrsft::ResponseHeaders wrsft::ResponseHeaders::RetryAfter = "Retry-After";
const wrsft::ResponseHeaders wrsft::ResponseHeaders::Server = "Server";
const wrsft::ResponseHeaders wrsft::ResponseHeaders::Vary = "Vary";
const wrsft::ResponseHeaders wrsft::ResponseHeaders::WWWAuthenticate = "WWW-Authenticate";



namespace wrsft {
    ParseResult::ParseResult(std::shared_ptr<HttpRequest> request, std::string msg) : request {request}, errorMessage{validate_message(msg)} {

    }

    std::string ParseResult::validate_message(std::string message) {

        if(message.empty()) return  message;

        if(message == ParseResult::OverFlow ||
            message == ParseResult::InvalidFormat ||
            message == ParseResult::Failed) return message;

        throw std::invalid_argument("Invalid argument.");
    }

    std::string ParseResult::getErrorMessage() const {
        return errorMessage;
    }

    ParseResult::operator bool() const {

        if(! errorMessage.empty()) return  false;

        if(request == nullptr) return false;

        return true;
    }

    //////////////
    //HttpMessageParser
    ///////////////

    HttpMessageParser::HttpMessageParser(const wrsft::LoggerType logger) : BaseService(logger){}

    ParseResult HttpMessageParser::parse(FileDescriptor fd) {
        logfunc("HttpMessageParser::Parse", "start");
        int bytesRead = 0;
        char buffer[HttpMessageParser::MAXREADSIZE];
        int offset = 0;

        memset(buffer, 0, HttpMessageParser::MAXREADSIZE);
//bytes = SSL_read(ssl, buf, sizeof(buf));    /* get request  todo**************************/
        while ((bytesRead = SSL_read(fd, &buffer[offset], HttpMessageParser::MAXREADSIZE - offset)) > 0 )
        {
            offset += bytesRead;

            if(offset >= HttpMessageParser::MAXREADSIZE)
                return ParseResult(nullptr, ParseResult::OverFlow);
        }

        if(offset < 1) {
            logfunc("HttpMessageParser::Parse",
                    "RequestLine is empty");
            return ParseResult(nullptr, ParseResult::InvalidFormat);
        }

        std::string message(buffer, offset);
        std::stringstream sstream(message);
        std::string line;
        std::string requestLine;
        std::string headerValueDelimiter = ":";
        std::map<std::string, std::string> headerValues;
        int bodyLength = 0;

        ////////////////////////
        /// processing headers
        ///////////////////////
        sstream.getline( &requestLine[0], HttpMessageParser::MaxHeaderValueSize, LINEFEED );

        // very first line should not be empty.
        if(requestLine.empty()) {
            logfunc("HttpMessageParser::Parse",
                    "RequestLine header is empty{ " + requestLine +
                    "} . early exit. end.");
            return ParseResult(nullptr, ParseResult::InvalidFormat);
        }
        // method path protocol/version
        std::vector<std::string> requestLineHeaderValue = split(requestLine , " ");

        if(requestLineHeaderValue.size() != 3)
        {
            logfunc("HttpMessageParser::Parse", "RequestLine header not properly formatted { "+requestLine  +"} . early exit. end.");
            return ParseResult(nullptr, ParseResult::InvalidFormat);
        }

        //
        //while(sstream.getline( &line[0], HttpMessageParser::MaxHeaderValueSize, LINEFEED ))
        while( getline(sstream, line, LINEFEED ))
        {
            auto length = line.length();

            if(length == 0) // empty line before next two crlf linez
            {
                for(auto i = 0; i <= 1; i ++)
                {
                    char carriage = sstream.peek();
                    getline(sstream, line, LINEFEED );

                    if(carriage != CARRIAGERETURN ) {
                        logfunc("HttpMessageParser::Parse", "End of headers section not properly terminated. early exit. end.");
                        return ParseResult(nullptr, ParseResult::InvalidFormat);
                    }

                }

                //end of headers reached.
                break;
            }

            //break line into key value pair and add it to map or some structre for class/object to construct later.
            // header : value \r\n
            std::vector<std::string> headerValue = split(line , headerValueDelimiter);
            auto segmentLength = headerValue.size();

            if(segmentLength != 2) {
                logfunc("HttpMessageParser::Parse", "Wrong header format. early exit. end.");
                return ParseResult(nullptr, ParseResult::InvalidFormat);
            }

            auto header = headerValue[0];
            headerValues.emplace(header, headerValue[1]);

            // detect if content-length header value is key; then there is a body that will follow
            if(header == RequestHeaders::ContentLength)
            {
                logfunc("HttpMessageParser::Parse", "request has body content of length: " + headerValue[1] );
                std::stringstream numberStream(headerValue[1]);
                numberStream >> bodyLength;
            }

            line.empty();
        }

        if(headerValues.size() < 1)
        {
            logfunc("HttpMessageParser::Parse", "no header found. early exit. end.");
            return ParseResult(nullptr, ParseResult::InvalidFormat);
        }

        logfunc("HttpMessageParser::Parse", "Constructing request object. "  );
        auto request = std::make_shared<HttpRequest>(logfunc,
                            HttpMethods::getMethod(requestLineHeaderValue[0]),
                            requestLineHeaderValue[1],
                            requestLineHeaderValue[2],
                            headerValues,
                            sstream.str());

        logfunc("HttpMessageParser::Parse", "End. HhttpRequest ctr... "  );
        return ParseResult(request);
    }


    //////////////
    //HttpRequest
    ///////////////

    HttpRequest::HttpRequest(wrsft::LoggerType logger, HttpMethods method, std::string path,
                             std::string version, std::map<std::string, std::string> headers,
                             std::string body) : HttpMessage(logger, headers, body), method{method}, path{path}, version{version} {

    }

    std::string HttpRequest::getRequestPath() const {
        return path;
    }

    float HttpRequest::getVersion() const {
        return atof(version.c_str());
    }

    HttpMethods HttpRequest::getMethod() const {
        return method;
    }

    //////////////
    //HttpMessage
    ///////////////

    HttpMessage::HttpMessage(wrsft::LoggerType logger) : BaseService(logger) {}

    HttpMessage::HttpMessage(wrsft::LoggerType logger, std::map<std::string, std::string> headers) :
            BaseService(logger), headers{headers}, headerEditable{false}{}

    HttpMessage::HttpMessage(wrsft::LoggerType logger, std::map<std::string, std::string> headers,
                             std::string body) :
            BaseService(logger), headers{headers}, body{body}, headerEditable{false}, bodyEditable{false} {}

    std::vector<std::string> HttpMessage::getHeaders() const {
        return std::vector<std::string>();
    }

    bool HttpMessage::hasHeader() const {
        return headers.size() > 0;
    }

    std::string HttpMessage::getHeaderValue(std::string headerKey) const {
        if(doesHeaderExist(headerKey)) {
            return headers.at(headerKey);
        }

        return std::string();
    }

    void HttpMessage::setHeaderValue(std::string headerKey, std::string value) {
        validateHeaderEditable();
        headers[headerKey] = value;
    }

    void HttpMessage::setHeadersValue(std::map<std::string, std::string> headersValue) {
        validateHeaderEditable();

        for (auto item : headersValue)
        {
            headers[item.first] = item.second;
        }
    }

    void HttpMessage::writeBody(std::string content, std::string contentType) {
        logfunc("HttpMessage::writeBody", "start");
        validateBodyEditable();
        validateHeaderEditable();

        logfunc("HttpMessage::writeBody", "updating content-type: " + contentType);
        headers[wrsft::Headers<ResponseHeaders>::ContentType] = contentType;

        logfunc("HttpMessage::writeBody", "updating entire body");
        body = content;

        logfunc("HttpMessage::writeBody", "updating content-length: " + content.size());
        headers[wrsft::Headers<ResponseHeaders>::ContentLength] = content.size();

        logfunc("HttpMessage::writeBody", "end");
    }

    std::string HttpMessage::getBody() const {
        return body;
    }

    void HttpMessage::sendChunk(FileDescriptor fd, char *data, size_t size) {
        logfunc("HttpMessage::sendChunk", "start");
        SSL_write(fd, data, size);
        logfunc("HttpMessage::sendChunk", "end");
    }

    bool HttpMessage::doesHeaderExist(std::string headerKey) const {
        return headers.find(headerKey) != headers.end();
    }

    void HttpMessage::validateHeaderEditable() {
        if (!headerEditable) throw std::invalid_argument("Headers already sent or uneditable.");
    }

    void HttpMessage::validateBodyEditable() {
        if (!bodyEditable) throw std::invalid_argument("body already sent or uneditable.");
    }

    void HttpMessage::sendHeaders(FileDescriptor fd) {
        logfunc("HttpMessage::sendHeaders", "start");
        validateHeaderEditable();

        // if date header not set, set it
        if(!doesHeaderExist(wrsft::Headers<wrsft::ResponseHeaders>::Date )) {
            setDateHeader(getCurrentDateForHeader());
        }

        //send status line
        auto statusLine = "HTTP/1.1" +  wrsft::HttpStatus::getStatusLine(getHttpStatus()) +  "\r\n";
        SSL_write(fd, statusLine.c_str(), statusLine.size() );

        for(auto header : headers) {
            auto keyValue =  header.first + ": " + header.second + "\r\n";
            SSL_write(fd, keyValue.c_str(), keyValue.size() );
        }

        std::string endOfHeaders = "\n\r\n\r\n";
        SSL_write(fd, endOfHeaders.c_str(), endOfHeaders.size() );
        headerEditable = false;
        logfunc("HttpMessage::sendHeaders", "end");
    }

    std::string HttpMessage::getCurrentDateForHeader() {
        logfunc("HttpMessage::getCurrentDateForHeader", "start");
        char buffer[MAXHEADERSITEMSIZE] = { 0 };

        std::string format = " %a, %e %b %G %T GMT";

        time_t rawtime = (0);
        struct tm gmt_time = {0};
        time(&rawtime);
        gmtime_r(&rawtime, &gmt_time);
        memset(buffer, 0, MAXHEADERSITEMSIZE);
        strftime(buffer, MAXHEADERSITEMSIZE, format.data(), &gmt_time);
        std::string dateHeader(buffer);
        logfunc("HttpMessage::getCurrentDateForHeader", "Date: => " + dateHeader);
        logfunc("HttpMessage::getCurrentDateForHeader", "end");
        return dateHeader;
    }

    void HttpMessage::setDateHeader(std::string value) {
        logfunc("HttpMessage::setDateHeader", "start - value: " + value);

        if(value.empty()) {
            throw std::invalid_argument("provide valid date for date header");
        }

        setHeaderValue(wrsft::Headers<wrsft::ResponseHeaders>::Date , value);

        logfunc("HttpMessage::setDateHeader", "end");
    }


    void HttpMessage::sendBody(FileDescriptor fd) {
        logfunc("HttpMessage::sendBody", "start");

        if(headerEditable) {
            sendHeaders(fd);
        }

        logfunc("HttpMessage::sendBody", "body write starting...");
        SSL_write(fd, body.c_str(), body.size());
        logfunc("HttpMessage::sendBody", "end");
    }

    HttpStatus HttpMessage::getHttpStatus() {
        return wrsft::HttpStatus::OK;
    }



    //////////////
    //HttpResponse  ?? At what point in time do we determine status code
    ///////////////


    HttpResponse::HttpResponse(wrsft::LoggerType logger, FileDescriptor fd) :
    HttpMessage(logger),
    fileDescriptor{fd},
    status {wrsft::HttpStatus::OK}{
        logfunc("HttpResponse::HttpResponse", "ctr");
    }

    void HttpResponse::appendHeader(std::string key, std::string value) {
        logfunc("HttpResponse::appendHeader", "start");
        setHeaderValue(key, value);
        logfunc("HttpResponse::appendHeader", "end");
    }

    void HttpResponse::sendFile(std::string path, std::string contentType) {
        logfunc("HttpResponse::sendFile", "start.  file path: " + path);

        fs::ifstream ifs(path.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

        if(ifs.fail())
        {
            logfunc("HttpResponse::sendFile", "failed to open file");
            throw std::invalid_argument(path);
        }

        fs::ifstream::pos_type fileSize = ifs.tellg();
        auto contentSize = static_cast<size_t >(fileSize);

        if (fileSize < 0) {
            logfunc("HttpResponse::sendFile", "file is empty. end");
            return;
        }


        std::string str;
        str.assign((std::istreambuf_iterator<char>(ifs)),
                   std::istreambuf_iterator<char>());

        writeBody(str, contentType);

        logfunc("HttpResponse::sendFile", "end");
    }

    /**
     * Method differs from base implementation which actually flush out data to ntwrk destination.
     * Calling this method sets payload content and contentType. To send message to ntwrk destination, call flush method
     * @param data
     * @param contentType
     */
    void HttpResponse::sendBody(std::string data, std::string contentType) {
        logfunc("HttpResponse::sendBody", "start");
        HttpMessage::writeBody(data, contentType);
        logfunc("HttpResponse::sendBody", "end");
    }

    void HttpResponse::appendBody(std::string data) {
        logfunc("HttpResponse::appendBody", "start");
        auto body = getBody();
        body += data;

        auto contentType = getHeaderValue(wrsft::Headers<ResponseHeaders>::ContentType);

        HttpMessage::writeBody(body, contentType);
        logfunc("HttpResponse::appendBody", "end");
    }

    void HttpResponse::flush() {
        logfunc("HttpResponse::flush", "start");
        HttpMessage::sendBody(fileDescriptor);
        logfunc("HttpResponse::flush", "end");
    }

    void HttpResponse::sendHeaders() {
        logfunc("HttpResponse::sendHeaders", "start");
        HttpMessage::sendHeaders(fileDescriptor);
        logfunc("HttpResponse::sendHeaders", "end");
    }

    void HttpResponse::setupStreaming() {
        logfunc("HttpResponse::setupStreaming", "start");
        logfunc("HttpResponse::setupStreaming", "Not Implemented!  \n\tDepends on actual data and sendChunk intrinsics.");
        logfunc("HttpResponse::setupStreaming", "end");
    }

    void HttpResponse::sendChunk(char *data, size_t size) {
        logfunc("HttpResponse::sendChunk", "start");
        HttpMessage::sendChunk(fileDescriptor, data, size);
        logfunc("HttpResponse::sendChunk", "end");
    }

    void HttpResponse::setStatus(wrsft::HttpStatus value) {
        status = value;
    }

    HttpStatus HttpResponse::getHttpStatus() {
        return status;
    }

    FileDescriptor::FileDescriptor(int fd) : fd{fd} {

    }

    FileDescriptor::operator SSL *() {
        return (SSL*)sslFd; //.get();
    }

    FileDescriptor::FileDescriptor(SSL& fd) : sslFd{&fd}{

    }

}