#pragma once

#include <rest/Http.h>
#include <mutex>
#include <unordered_map>

class RestController {
private:
    static std::shared_ptr<RestController> instance;
    static std::mutex mtx;
    std::unordered_map<Method, std::unordered_map<std::string, HttpHandler>> routes;

public:
    static std::shared_ptr<RestController> getInstance() {
        std::lock_guard<std::mutex> lock(mtx);
        if (instance == nullptr) {
            instance = std::make_shared<RestController>();
        }
        return instance;
    }

    void start_server(const int& port, const int& num_threads);

    void add_routes(const Method& method, const std::string& target, const HttpHandler& handler);

    void handle_request(const BoostRequest& req, BoostResponse& res);
};
