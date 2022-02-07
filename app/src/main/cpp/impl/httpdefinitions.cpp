//
// Created by musta on 1/23/2022.
//

#include "httpdefinitions.h"
#include <string>
#include <unistd.h>
#include <sstream>
#include "stringHelper.h"


const std::string wrsft::HttpMethods::GET = "GET";
const std::string wrsft::HttpMethods::POST = "POST";
const std::string wrsft::HttpMethods::DELETE = "DELETE";
const std::string wrsft::HttpMethods::PUT = "PUT";
const std::string wrsft::HttpMethods::PATCH = "PATCH";

/**
 * size limit reached
 */
const std::string wrsft::ParseResult::OverFlow = "overflow";
const std::string wrsft::ParseResult::InvalidFormat = "format";
const std::string wrsft::ParseResult::Failed = "failed";


const std::string wrsft::Headers::CacheControl = "Cache-Control";
const std::string wrsft::Headers::Connection = "Connection";
const std::string wrsft::Headers::Date = "Date";
const std::string wrsft::Headers::Pragma = "Pragma";
const std::string wrsft::Headers::Trailer = "Trailer";
const std::string wrsft::Headers::TransferEncoding = "TransferEncoding";
const std::string wrsft::Headers::Upgrade = "Upgrade";
const std::string wrsft::Headers::Via = "Via";
const std::string wrsft::Headers::Warning = "Warning";
const std::string wrsft::Headers::ContentEncoding = "Content-Encoding";
const std::string wrsft::Headers::ContentLength = "Content-Length";

const std::string wrsft::RequestHeaders::AcceptCharset = "Accept-Charset";
const std::string wrsft::RequestHeaders::AcceptEncoding = "Accept-Encoding";
const std::string wrsft::RequestHeaders::AcceptLanguage = "Accept-Language";
const std::string wrsft::RequestHeaders::Authorization = "Authorization";
const std::string wrsft::RequestHeaders::Expect = "Expect";
const std::string wrsft::RequestHeaders::From = "From";
const std::string wrsft::RequestHeaders::Host = "Host";
const std::string wrsft::RequestHeaders::IfMatch = "If-Match";
const std::string wrsft::RequestHeaders::Accept = "Accept";


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

    ParseResult HttpMessageParser::Parse(int fd) {
        int bytesRead = 0;
        char buffer[HttpMessageParser::MAXREADSIZE];
        int offset = 0;

        memset(buffer, 0, HttpMessageParser::MAXREADSIZE);

        while ((bytesRead = read(fd, &buffer[offset], HttpMessageParser::MAXREADSIZE - offset)) > 0 )
        {
            offset += bytesRead;

            if(offset >= HttpMessageParser::MAXREADSIZE)
                return ParseResult(nullptr, ParseResult::OverFlow);
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
        if(requestLine.empty())
            return ParseResult(nullptr, ParseResult::InvalidFormat);
        // method path protocol/version
        std::vector<std::string> requestLineHeaderValue = split(requestLine , " ");

        if(requestLineHeaderValue.size() != 3)
        {
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

                    if(carriage != CARRIAGERETURN )
                        return ParseResult(nullptr, ParseResult::InvalidFormat);

                }

                //end of headers reached.
                break;
            }


            //break line into key value pair and add it to map or some structre for class/object to construct later.
            // header : value \r\n
            std::vector<std::string> headerValue = split(line , headerValueDelimiter);
            auto segmentLength = headerValue.size();

            if(segmentLength != 2)
                return ParseResult(nullptr, ParseResult::InvalidFormat);

            auto header = headerValue[0];
            headerValues.emplace(header, headerValue[1]);

            // detect if content-length header value is key; then there is a body that will follow
            if(header == RequestHeaders::ContentLength)
            {
                std::stringstream numberStream(headerValue[1]);
                numberStream >> bodyLength;
            }

            line.empty();
        }

        if(headerValues.size() < 1)
        {
            return ParseResult(nullptr, ParseResult::InvalidFormat);
        }
    }

}