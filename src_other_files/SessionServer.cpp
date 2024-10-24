/**
 * @file SessionServer.cpp
 * @brief A simple HTTP server using Boost.Beast and Boost.Asio libraries.
 *
 * This server handles HTTP GET and POST requests, manages sessions using cookies,
 * and routes requests based on HTTP methods and paths.
 *
 * - GET /: Returns a welcome message.
 * - GET /hello: Returns "Hello, World!".
 * - GET /status: Returns the session data.
 * - POST /update: Updates the session data with the request body.
 *
 * The server generates a random session ID for each new session and stores session data
 * in an unordered map. It uses asynchronous operations for reading requests and writing responses.
 *
 * Classes:
 * - session: Manages individual client connections and handles HTTP requests.
 * - server: Accepts incoming connections and creates session objects.
 *
 * Functions:
 * - generate_session_id: Generates a random session ID.
 * - handle_request: Routes and processes HTTP requests.
 *
 * The server listens on port 8080 and runs on a single-threaded io_context.
 * Testing the Session-Based API
 *
 * You can use curl to test the session functionality:
 * First Request (No session)
 * ```bash
 * curl -v http://localhost:8080/status
 * ```
 * The response will set a session_id cookie.
 *
 * Subsequent Requests (With session) Once you have the session_id from the first request, include it in subsequent requests:
 * ```bash
 * curl -v --cookie "session_id=<session_id>" http://localhost:8080/status
 * ```
 *
 * POST Request to Update Session
 * ```bash
 * curl -X POST --cookie "session_id=<session_id>" -d "New Session Data" http://localhost:8080/update
 * ```
 *
 * This is a basic implementation of session handling in a Boost.Beast HTTP server. The sessions are currently in-memory, but for production use, sessions are typically stored in a database or some persistent storage like Redis.
 */

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <boost/asio/strand.hpp>
#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <unordered_map>
#include <random>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

// Session store to store session-related data
std::unordered_map<std::string, std::string> session_store;

// Function to generate a random session ID
std::string generate_session_id() {
    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    int len = 16;  // Length of the session ID
    std::string session_id;
    session_id.reserve(len);

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<> dist(0, sizeof(alphanum) - 2);

    for (int i = 0; i < len; ++i) {
        session_id += alphanum[dist(rng)];
    }
    return session_id;
}

// Function to handle HTTP requests and route based on HTTP methods and paths
void handle_request(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
    std::string target(req.target());
    std::string session_id;

    // Check if the request contains a session cookie
    if (req.has(http::field::cookie)) {
        std::string cookie_header = req[http::field::cookie];
        std::size_t session_pos = cookie_header.find("session_id=");
        if (session_pos != std::string::npos) {
            session_id = cookie_header.substr(session_pos + 11);  // Extract session ID
        }
    }

    // If no session ID, create a new session
    if (session_id.empty() || session_store.find(session_id) == session_store.end()) {
        session_id = generate_session_id();
        session_store[session_id] = "New Session Data";  // Initialize session data
        res.set(http::field::set_cookie, "session_id=" + session_id + "; Path=/");
    }

    if (req.method() == http::verb::get) {
        if (target == "/") {
            res.result(http::status::ok);
            res.set(http::field::content_type, "text/plain");
            res.body() = "Welcome to the Boost HTTP Server!";
        } else if (target == "/hello") {
            res.result(http::status::ok);
            res.set(http::field::content_type, "text/plain");
            res.body() = "Hello, World!";
        } else if (target == "/status") {
            res.result(http::status::ok);
            res.set(http::field::content_type, "text/plain");
            res.body() = "Session data: " + session_store[session_id];
        } else {
            res.result(http::status::not_found);
            res.body() = "Route not found";
        }
    } else if (req.method() == http::verb::post) {
        if (target == "/update") {
            session_store[session_id] = req.body();  // Update session data with request body
            res.result(http::status::ok);
            res.set(http::field::content_type, "text/plain");
            res.body() = "Session updated!";
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
