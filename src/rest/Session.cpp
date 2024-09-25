#include "rest/Session.h"
#include <boost/json.hpp>
#include <iostream>

void handle_request(const boost::beast::http::request<boost::beast::http::string_body>& req, boost::beast::http::response<boost::beast::http::string_body>& res) {
    // Get the target path (e.g., "/status")
    std::string target(req.target());

    if (req.method() == boost::beast::http::verb::get) {
        if (target == "/") { // Handle root path
            boost::json::value response_body = {
                {"message", "Welcome to the REST API"},
                {"status", "success"}
            };

            std::string json_string = boost::json::serialize(response_body);

            res.result(boost::beast::http::status::ok);
            res.set(boost::beast::http::field::content_type, "application/json");
            res.body() = json_string;
        } else if (target == "/status") { // Handle /status route
            boost::json::value response_body = {
                {"status", "API is running smoothly"}
            };

            std::string json_string = boost::json::serialize(response_body);

            res.result(boost::beast::http::status::ok);
            res.set(boost::beast::http::field::content_type, "application/json");
            res.body() = json_string;
        } else { // Handle unknown route (404 Not Found)
            res.result(boost::beast::http::status::not_found);
            res.body() = "Route not found";
        }
    } else { // Handle unsupported HTTP method
        res.result(boost::beast::http::status::bad_request);
        res.body() = "Unsupported HTTP method";
    }

    res.prepare_payload();
}

void Session::run() {
    read_request();
}

void Session::read_request() {
    auto self = shared_from_this();
    boost::beast::http::async_read(socket_, buffer_, req_,
        [self](boost::beast::error_code ec, std::size_t bytes_transferred) {
            if (!ec) {
                self->process_request();
            } else {
                std::cerr << "Read error: " << ec.message() << std::endl;
            }
        });
}

void Session::process_request() {
    handle_request(req_, res_);
    write_response();
}

void Session::write_response() {
    auto self = shared_from_this();
    boost::beast::http::async_write(socket_, res_,
        [self](boost::beast::error_code ec, std::size_t bytes_transferred) {
            if (ec) {
                std::cerr << "Write error: " << ec.message() << std::endl;
            }
            boost::beast::error_code shutdown_ec;
            self->socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_send, shutdown_ec);
            if (shutdown_ec) {
                std::cerr << "Shutdown error: " << shutdown_ec.message() << std::endl;
            }
        });
}
