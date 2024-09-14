#include "Server.h"
#include "Session.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <boost/asio/strand.hpp>
#include <boost/json.hpp>
#include <iostream>
#include <string>
#include <thread>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace json = boost::json;
using tcp = boost::asio::ip::tcp;

void handle_request(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
    // Get the target path (e.g., "/status")
    std::string target(req.target());

    if (req.method() == http::verb::get) {
        if (target == "/") { // Handle root path
            json::value response_body = {
                {"message", "Welcome to the REST API"},
                {"status", "success"}
            };

            std::string json_string = json::serialize(response_body);

            res.result(http::status::ok);
            res.set(http::field::content_type, "application/json");
            res.body() = json_string;
        } else if (target == "/status") { // Handle /status route
            json::value response_body = {
                {"status", "API is running smoothly"}
            };

            std::string json_string = json::serialize(response_body);

            res.result(http::status::ok);
            res.set(http::field::content_type, "application/json");
            res.body() = json_string;
        } else { // Handle unknown route (404 Not Found)
            res.result(http::status::not_found);
            res.body() = "Route not found";
        }
    } else { // Handle unsupported HTTP method
        res.result(http::status::bad_request);
        res.body() = "Unsupported HTTP method";
    }

    res.prepare_payload();
}

Session::Session(tcp::socket socket) : socket_(std::move(socket)) {}

void Session::run() {
    read_request();
}

void Session::read_request() {
    auto self = shared_from_this();
    http::async_read(socket_, buffer_, req_,
        [self](beast::error_code ec, std::size_t bytes_transferred) {
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
    http::async_write(socket_, res_,
        [self](beast::error_code ec, std::size_t bytes_transferred) {
            if (ec) {
                std::cerr << "Write error: " << ec.message() << std::endl;
            }
            beast::error_code shutdown_ec;
            self->socket_.shutdown(tcp::socket::shutdown_send, shutdown_ec);
            if (shutdown_ec) {
                std::cerr << "Shutdown error: " << shutdown_ec.message() << std::endl;
            }
        });
}

Server::Server(net::io_context& ioc, tcp::endpoint endpoint) : acceptor_(ioc) {
    boost::system::error_code ec;
    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
        std::cerr << "Open error: " << ec.message() << std::endl;
        return;
    }
    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) {
        std::cerr << "Set option error: " << ec.message() << std::endl;
        return;
    }
    acceptor_.bind(endpoint, ec);
    if (ec) {
        std::cerr << "Bind error: " << ec.message() << std::endl;
        return;
    }
    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
        std::cerr << "Listen error: " << ec.message() << std::endl;
        return;
    }
    do_accept();
}

void Server::do_accept() {
    acceptor_.async_accept(
        [this](beast::error_code ec, tcp::socket socket) {
            if (ec) {
                std::cerr << "Accept error: " << ec.message() << std::endl;
                do_accept(); // Retry accepting
            } else {
                std::make_shared<Session>(std::move(socket))->run();
                do_accept(); // Continue accepting new connections
            }
        });
}

void start_server(const int& port, const int& num_threads) {
    try {
        net::io_context ioc{num_threads};
        tcp::endpoint endpoint{tcp::v4(), static_cast<unsigned short>(port)};

        auto srv = std::make_shared<Server>(ioc, endpoint);

        ioc.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        throw;
    }
}
