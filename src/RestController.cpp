#include "RestController.h"
#include "Server.h"
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include <boost/json.hpp>
#include <iostream>

std::shared_ptr<RestController> RestController::instance = nullptr;
std::mutex RestController::mtx;

void RestController::start_server(const int& port, const int& num_threads) {
    try {
        boost::asio::io_context ioc{num_threads};
        boost::asio::ip::tcp::endpoint endpoint{boost::asio::ip::tcp::v4(), static_cast<unsigned short>(port)};

        auto srv = std::make_shared<Server>(ioc, endpoint);

        ioc.run();
    } catch (const std::runtime_error& e) {
        std::cerr << "Runtime Exception: " << e.what() << std::endl;
        throw;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        throw;
    }
}

void RestController::add_routes(const Method& method, const std::string& target, const HttpHandler& handler) {
    auto iter = routes.find(method);
    if (iter != routes.end()) {
        iter->second.emplace(target, handler);
    } else {
        std::unordered_map<std::string, HttpHandler> new_map;
        new_map.emplace(target, handler);
        routes.emplace(method, new_map);
    }
}

void RestController::handle_request(const BoostRequest& req, BoostResponse& res) {
    // response with CORS headers
    res.version(11); // HTTP/1.1
    res.set(boost::beast::http::field::server, "Boost REST API");
    res.set(boost::beast::http::field::content_type, "application/json");
    res.set(boost::beast::http::field::access_control_allow_origin, "*");
    res.set(boost::beast::http::field::access_control_allow_methods, "GET, POST");
    res.set(boost::beast::http::field::access_control_allow_headers, "Content-Type");

    auto methodIter = routes.find(req.method());
    std::string body;

    if (methodIter != routes.end()) {
        auto targetIter = methodIter->second.find(req.target());
        if (targetIter != methodIter->second.end()) {
            std::cout << "Request: " << req.method() << " " << req.target() << std::endl;
            body = targetIter->second(req, res);
        } else {
            res.result(static_cast<boost::beast::http::status>(404));
            res.set("Content-Type", "text/plain");
            body = "Route not found";
            std::cout << "Request: " << req.method() << " " << req.target() << "Route not found." << std::endl;
        }
    } else {
        res.result(static_cast<boost::beast::http::status>(400));
        res.set("Content-Type", "text/plain");
        body = "Unsupported HTTP method";
        std::cout << "Request: " << req.method() << " " << req.target() << "Unsupported HTTP method." << std::endl;
    }

    auto& headers = res.base();
    auto bodySize = body.size();
    if (bodySize > 0 && headers.find("Content-Type") != headers.end() && headers["Content-Type"] == "application/json") {
        boost::json::value json_body = boost::json::parse(body);
        res.body() = boost::json::serialize(json_body);
    } else if (bodySize > 0) {
        res.body() = body;
    }
    res.content_length(bodySize);
    res.prepare_payload();
}
