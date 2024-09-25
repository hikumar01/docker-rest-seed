#include "rest/RestController.h"
#include "rest/Server.h"
#include <boost/asio.hpp>
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
    const HttpRequest request(req);
    HttpResponse response;
    auto methodIter = routes.find(request.method);
    if (methodIter != routes.end()) {
        auto targetIter = methodIter->second.find(request.target);
        if (targetIter != methodIter->second.end()) {
            targetIter->second(request, response);
        } else {
            response.status_code = 404;
            response.headers["Content-Type"] = "text/plain";
            response.body = "Route not found";
        }
    } else {
        response.status_code = 400;
        response.headers["Content-Type"] = "text/plain";
        response.body = "Unsupported HTTP method";
    }
    response.convertToBoostResponse(res);
}
