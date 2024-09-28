#include "RestController.h"
#include <iostream>

int main(int argc, char* argv[]) {
    const int port = 8080; // This port should match with the port in the Dockerfile
    const int num_threads = 1;
    std::cout << "Server running on http://localhost:" << port << "." << std::endl;
    auto rest_controller = RestController::getInstance();

    rest_controller->add_routes(Method::get, "/api/hello", [](const BoostRequest& req, BoostResponse& res) {
        res.result(boost::beast::http::status::ok);
        res.set(boost::beast::http::field::content_type, "application/json");
        res.body() = R"({"message": "Welcome to the REST API", "status": "success"})";
    });

    rest_controller->add_routes(Method::get, "/status", [](const BoostRequest& req, BoostResponse& res) {
        res.result(boost::beast::http::status::ok);
        res.set(boost::beast::http::field::content_type, "text/plain");
        res.body() = "API is running smoothly";
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
