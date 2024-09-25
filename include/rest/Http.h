#pragma once

#include <boost/beast.hpp>
#include <string>
#include <unordered_map>

class HttpRequest;
class HttpResponse;
using BoostRequest = boost::beast::http::request<boost::beast::http::string_body>;
using BoostResponse = boost::beast::http::response<boost::beast::http::string_body>;
using HttpHandler = std::function<void(const HttpRequest&, HttpResponse&)>;

enum Method {
    GET = 0,
    POST,
    UNKNOWN
};

Method convertBoostMethod(const boost::beast::http::verb& method);

class HttpRequest {
public:
    Method method;
    std::string target;
    int version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    HttpRequest(const BoostRequest& req);
};

class HttpResponse {
public:
    int status_code;
    int version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    HttpResponse() : version{10} {}

    void convertToBoostResponse(BoostResponse& boostResponse);
};
