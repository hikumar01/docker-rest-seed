#include "RestController.h"
#include <iostream>

int main(int argc, char* argv[]) {
    const int port = 8080; // This port should match with the port in the Dockerfile
    const int num_threads = 1;
    std::cout << "Server running on http://localhost:" << port << std::endl;
    auto rest_controller = RestController::getInstance();

    rest_controller->add_routes(Method::get, "/api/hello", [](const BoostRequest& req, BoostResponse& res) {
        res.result(boost::beast::http::status::ok);
        res.set("Content-Type", "application/json");
        // std::string body = "{\"message\":\"Welcome to the REST API\",\"status\":\"success\"}";
        std::string body = R"({"message": "Welcome to the REST API", "status": "success"})";
        return body;
    });

    rest_controller->add_routes(Method::get, "/status", [](const BoostRequest& req, BoostResponse& res) {
        res.result(boost::beast::http::status::ok);
        res.set("Content-Type", "text/plain");
        std::string body = "API is running smoothly";
        return body;
    });

    try {
        rest_controller->start_server(port, num_threads);
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}
