/**
 * @file BasicServer.cpp
 * @brief A basic HTTP server using Boost.Beast and Boost.Asio libraries.
 *
 * This server handles HTTP GET and POST requests. It supports the following routes:
 * - GET /: Returns a welcome message.
 * - GET /hello: Returns "Hello, World!".
 * - POST /submit: Acknowledges receipt of a POST request.
 * - Any other route or unsupported HTTP method results in an appropriate error response.
 *
 * The server is implemented using asynchronous I/O operations provided by Boost.Asio.
 * It listens on port 8080 and creates a new session for each incoming connection.
 * Each session reads the request, processes it, and writes the response back to the client.
 *
 * Main components:
 * - handle_request: Function to process incoming HTTP requests and generate responses.
 * - session: Class to manage individual client connections and handle request/response lifecycle.
 * - server: Class to accept incoming connections and create sessions.
 *
 * The main function initializes the server and runs the I/O context to start handling requests.
 *
 * # Test GET requests
 * curl http://localhost:8080/
 * curl http://localhost:8080/hello
 *
 * # Test POST request
 * curl -X POST http://localhost:8080/submit
 *
 */

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <boost/asio/strand.hpp>
#include <iostream>
#include <string>
#include <memory>
#include <thread>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

void handle_request(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
    std::string target(req.target());

    if (req.method() == http::verb::get) {
        if (target == "/") {
            res.result(http::status::ok);
            res.set(http::field::content_type, "text/plain");
            res.body() = "Welcome to the Boost HTTP Server!";
        } else if (target == "/hello") {
            res.result(http::status::ok);
            res.set(http::field::content_type, "text/plain");
            res.body() = "Hello, World!";
        } else {
            res.result(http::status::not_found);
            res.body() = "Route not found";
        }
    } else if (req.method() == http::verb::post) {
        if (target == "/submit") {
            res.result(http::status::ok);
            res.set(http::field::content_type, "text/plain");
            res.body() = "POST request received!";
        } else {
            res.result(http::status::not_found);
            res.body() = "Route not found";
        }
    } else {
        res.result(http::status::bad_request);
        res.body() = "Unsupported HTTP method";
    }

    res.prepare_payload();
}

class session : public std::enable_shared_from_this<session> {
public:
    session(tcp::socket socket) : socket_(std::move(socket)) {}

    void run() {
        read_request();
    }

private:
    tcp::socket socket_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    http::response<http::string_body> res_;

    void read_request() {
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

    void process_request() {
        handle_request(req_, res_);
        write_response();
    }

    void write_response() {
        auto self = shared_from_this();
        http::async_write(socket_, res_,
            [self](beast::error_code ec, std::size_t bytes_transferred) {
                if (ec) {
                    std::cerr << "Write error: " << ec.message() << std::endl;
                }
                self->socket_.shutdown(tcp::socket::shutdown_send, ec);
            });
    }
};

class server {
public:
    server(net::io_context& ioc, tcp::endpoint endpoint) : acceptor_(ioc) {
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

private:
    tcp::acceptor acceptor_;

    void do_accept() {
        acceptor_.async_accept(
            [this](beast::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::make_shared<session>(std::move(socket))->run();
                } else {
                    std::cerr << "Accept error: " << ec.message() << std::endl;
                }
                do_accept();
            });
    }
};

int main() {
    try {
        const int port = 8080;
        net::io_context ioc{1};
        tcp::endpoint endpoint{tcp::v4(), static_cast<unsigned short>(port)};

        std::make_shared<server>(ioc, endpoint);

        std::cout << "Server running on http://localhost:" << port << std::endl;

        ioc.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return 0;
}
