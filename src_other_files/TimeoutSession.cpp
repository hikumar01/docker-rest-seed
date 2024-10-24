/**
 * @file TimeoutSession.cpp
 * @brief A simple HTTP server with timeout handling using Boost.Beast and Boost.Asio.
 *
 * This file contains the implementation of a basic HTTP server that handles client requests
 * and includes a timeout mechanism to close connections that take too long to complete.
 *
 * The server listens on port 8080 and responds with a JSON message "Hello, World!" to any
 * incoming HTTP POST request. If a request takes longer than 10 seconds to complete, the
 * server will log a timeout message and close the connection.
 *
 * Dependencies:
 * - Boost.Beast
 * - Boost.Asio
 * - Boost.JSON
 *
 * Usage:
 * - Compile the code with the necessary Boost libraries.
 * - Run the server executable.
 * - Send an HTTP POST request to http://localhost:8080 to test the server.
 * - To simulate a timeout, send an incomplete request and wait for 10 seconds.
 *
 * Example:
 * @code
 * curl -v http://localhost:8080 -X POST --data "incomplete"
 * # wait for 10 seconds, the server should log a timeout message
 * @endcode
 *
 * Classes:
 * - session: Manages an individual client connection, including reading requests,
 *   processing them, and sending responses. It also handles timeouts.
 * - server: Manages the listening socket and accepts incoming client connections.
 *
 * Functions:
 * - session::run(): Starts the session by initiating the timeout timer and reading the request.
 * - session::start_timeout(): Sets a 10-second timeout for the session.
 * - session::cancel_timeout(): Cancels the timeout if the operation completes in time.
 * - session::read_request(): Reads the HTTP request asynchronously.
 * - session::process_request(): Processes the HTTP request and prepares the response.
 * - session::write_response(): Writes the HTTP response to the client.
 * - session::handle_timeout(): Handles the timeout event by closing the connection.
 * - session::handle_error(): Handles errors that occur during asynchronous operations.
 * - server::do_accept(): Accepts incoming client connections asynchronously.
 *
 * @note This example is for educational purposes and may not be suitable for production use.
 *       Proper error handling, security measures, and optimizations should be added for a
 *       production-ready server.
 */

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/json.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace json = boost::json;
using tcp = net::ip::tcp;

class session : public std::enable_shared_from_this<session> {
public:
    session(tcp::socket socket)
        : socket_(std::move(socket)), timer_(socket_.get_executor().context()) {}

    void run() {
        start_timeout();  // Start the timeout timer
        read_request();
    }

private:
    tcp::socket socket_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    http::response<http::string_body> res_;
    net::steady_timer timer_;  // Timer for async timeout handling

    // Set a timeout for the operation
    void start_timeout() {
        timer_.expires_after(std::chrono::seconds(10));  // Set a 10-second timeout
        auto self = shared_from_this();
        timer_.async_wait([self](boost::system::error_code ec) {
            if (!ec) {
                self->handle_timeout();  // If timer expires, handle the timeout
            }
        });
    }

    // Cancel the timeout if the operation finishes in time
    void cancel_timeout() {
        timer_.cancel();
    }

    // Handle request reading
    void read_request() {
        auto self = shared_from_this();
        http::async_read(socket_, buffer_, req_,
            [self](beast::error_code ec, std::size_t bytes_transferred) {
                if (!ec) {
                    self->cancel_timeout();  // Cancel timeout if read completes
                    self->process_request();
                } else {
                    self->handle_error(ec);
                }
            });
    }

    // Process the HTTP request
    void process_request() {
        res_.result(http::status::ok);
        res_.set(http::field::content_type, "application/json");
        json::value jv = {{"message", "Hello, World!"}};
        res_.body() = json::serialize(jv);
        res_.prepare_payload();
        write_response();
    }

    // Write response to the client
    void write_response() {
        auto self = shared_from_this();
        http::async_write(socket_, res_,
            [self](beast::error_code ec, std::size_t bytes_transferred) {
                if (ec) {
                    self->handle_error(ec);
                }
                self->socket_.shutdown(tcp::socket::shutdown_send, ec);
            });
    }

    // Handle a timeout if the timer expires
    void handle_timeout() {
        std::cerr << "Timeout occurred, canceling session." << std::endl;
        beast::error_code ec;
        socket_.shutdown(tcp::socket::shutdown_both, ec);
        socket_.close(ec);  // Close the socket
    }

    // Handle errors (including cancellation due to timeout)
    void handle_error(beast::error_code ec) {
        if (ec != net::error::operation_aborted) {
            std::cerr << "Error: " << ec.message() << std::endl;
        }
        beast::error_code shutdown_ec;
        socket_.shutdown(tcp::socket::shutdown_both, shutdown_ec);
    }
};

class server {
public:
    server(net::io_context& ioc, tcp::endpoint endpoint)
        : acceptor_(ioc) {
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
