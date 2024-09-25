#include "rest/Http.h"
#include <boost/json.hpp>

Method convertBoostMethod(const boost::beast::http::verb& method) {
    switch (method) {
        case boost::beast::http::verb::get:
            return Method::GET;
        case boost::beast::http::verb::post:
            return Method::POST;
        default:
            return Method::UNKNOWN;
    }
}

HttpRequest::HttpRequest(const BoostRequest& req) {
    method = convertBoostMethod(req.method());
    target = req.target();
    version = req.version();
    for (const auto& field : req) {
        headers[field.name_string()] = field.value();
    }
    body = req.body();
}

void HttpResponse::convertToBoostResponse(BoostResponse& boostResponse) {
    boostResponse.result(static_cast<boost::beast::http::status>(status_code));
    for (const auto& header : headers) {
        boostResponse.set(header.first, header.second);
    }
    auto bodySize = body.size();
    if (bodySize > 0) {
        if (headers.find("Content-Type") != headers.end() && headers["Content-Type"] == "application/json") {
            boost::json::value json_body = boost::json::parse(body);
            boostResponse.body() = boost::json::serialize(json_body);
        } else {
            boostResponse.body() = body;
        }
    }
    boostResponse.content_length(bodySize);
    boostResponse.prepare_payload();
}
