#pragma once

#include <boost/beast/http.hpp>
#include <mutex>
#include <string>
#include <unordered_map>

using BoostRequest = boost::beast::http::request<boost::beast::http::string_body>;
using BoostResponse = boost::beast::http::response<boost::beast::http::string_body>;
using HttpHandler = std::function<void(const BoostRequest&, BoostResponse&)>;
using Method = boost::beast::http::verb;

class RestController {
private:
    static std::string defaultTarget;
    static std::shared_ptr<RestController> instance;
    static std::mutex mtx;
    std::unordered_map<Method, std::unordered_map<std::string, HttpHandler>> routes;

public:
    static std::shared_ptr<RestController> getInstance(std::string target = "") {
        std::lock_guard<std::mutex> lock(mtx);
        if (instance == nullptr) {
            instance = std::make_shared<RestController>();
            instance->defaultTarget = target;
        }
        return instance;
    }

    void start_server(const int& port, const int& num_threads);

    void add_routes(const Method& method, const std::string& target, const HttpHandler& handler);

    void handle_request(const BoostRequest& req, BoostResponse& res);

    std::string read_file(const std::string& path);

    std::string get_mime_type(const std::string& path);
};
