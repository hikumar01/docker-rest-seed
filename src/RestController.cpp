#include "RestController.h"
#include "Server.h"
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include <boost/json.hpp>
#include <fstream>
#include <iostream>
#include <string>

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
    res.set(boost::beast::http::field::server, "REST API");
    res.set(boost::beast::http::field::access_control_allow_origin, "*");
    res.set(boost::beast::http::field::access_control_allow_methods, "GET, POST");
    res.set(boost::beast::http::field::access_control_allow_headers, "Content-Type");
    
    const auto& methodIter = routes.find(req.method());
    Method req_method = methodIter != routes.end() ? methodIter->first : Method::unknown;
    std::string target = req.target();
    std::cout << "Request: " << req.method() << " " << req.target() << std::endl;

    if (req_method == Method::get && target.compare("/") == 0)
        target = "/index.html";
    const std::string mime_type = get_mime_type(target);
    const static std::string ui_prefix = "./ui";

    if (mime_type.compare("application/octet-stream") != 0) {
        std::string content = read_file(ui_prefix + target);
        if (content.empty()) {
            res.result(boost::beast::http::status::moved_permanently);
            res.set(boost::beast::http::field::location, "/index.html");
        } else {
            res.result(boost::beast::http::status::ok);
            res.set(boost::beast::http::field::content_type, mime_type);
            res.body() = content;
        }
    } else if (methodIter != routes.end()) {
        auto targetIter = methodIter->second.find(target);
        if (targetIter != methodIter->second.end()) {
            targetIter->second(req, res);
        } else {
            res.result(boost::beast::http::status::not_found);
        }
    } else {
        res.result(boost::beast::http::status::bad_request);
    }
    std::cout << "  Request: " << req.method() << " " << req.target() << " -> Response: " << res.reason() << std::endl; // res[boost::beast::http::field::content_type]

//    if (res.has_content_length() > 0 && res.find(boost::beast::http::field::content_type) != res.end() &&
//        res[boost::beast::http::field::content_type].compare("application/json") == 0) {
//        boost::json::value json_body = boost::json::parse(res.body());
//        res.body() = boost::json::serialize(json_body);
//    }
    res.prepare_payload();
}

std::string RestController::read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string RestController::get_mime_type(const std::string& path) {
    static const std::unordered_map<std::string, std::string> mime_types = {
        {".htm", "text/html"},
        {".html", "text/html"},
        {".php", "text/html"},
        {".css", "text/css"},
        {".txt", "text/plain"},
        {".js", "application/javascript"},
        {".json", "application/json"},
        {".xml", "application/xml"},
        {".swf", "application/x-shockwave-flash"},
        {".flv", "video/x-flv"},
        {".png", "image/png"},
        {".jpe", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".jpg", "image/jpeg"},
        {".gif", "image/gif"},
        {".bmp", "image/bmp"},
        {".ico", "image/vnd.microsoft.icon"},
        {".tiff", "image/tiff"},
        {".tif", "image/tiff"},
        {".svg", "image/svg+xml"},
        {".svgz", "image/svg+xml"}
    };
    
    const auto extension_index = path.rfind('.');
    if (extension_index == std::string::npos) {
        return "application/octet-stream";
    }
    const auto& extension = path.substr(extension_index);
    
    auto it = mime_types.find(extension);
    if (it != mime_types.end()) {
        return it->second;
    }
    return "application/octet-stream";
}
