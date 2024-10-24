/**
 * @file PersistentSession.cpp
 * @brief A simple HTTP server with persistent session management using Boost.Beast and Boost.Asio.
 *
 * This file contains the implementation of a basic HTTP server that manages sessions using cookies.
 * The server generates a unique session ID for each new session and stores session data in an
 * unordered_map. The session ID is sent to the client as a cookie, and subsequent requests from
 * the client include this session ID to retrieve the session data.
 *
 * Dependencies:
 * - Boost.Beast
 * - Boost.Asio
 * - Boost.JSON
 *
 * Namespaces:
 * - beast: Alias for boost::beast
 * - http: Alias for boost::beast::http
 * - net: Alias for boost::asio
 * - json: Alias for boost::json
 *
 * Functions:
 * - std::string generate_session_id(): Generates a unique session ID using a random number generator.
 * - std::string parse_session_id(const http::request<http::string_body>& req): Parses the session ID
 *   from the request cookies.
 * - void handle_request(const http::request<http::string_body>& req, http::response<http::string_body>& res):
 *   Handles the HTTP request, manages the session, and prepares the HTTP response.
 *
 * Classes:
 * - class session: Manages an individual session, including reading requests and writing responses.
 * - class server: Manages the server, including accepting new connections and creating session objects.
 *
 * Main Function:
 * - int main(): Entry point of the application. Initializes the server and runs the I/O context.
 */

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <iostream>
#include <unordered_map>
#include <random>
#include <chrono>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace json = boost::json;
using tcp = net::ip::tcp;

std::unordered_map<std::string, json::value> session_store; // Session store

// Generate a unique session ID
std::string generate_session_id() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    return std::to_string(dis(gen));
}

// Parse cookies from the request headers
std::string parse_session_id(const http::request<http::string_body>& req) {
    auto cookie_header = req[http::field::cookie];
    std::string session_id;

    if (!cookie_header.empty()) {
        std::string cookies = cookie_header.to_string();
        size_t pos = cookies.find("session_id=");
        if (pos != std::string::npos) {
            session_id = cookies.substr(pos + 11); // Skip "session_id="
            size_t semicolon_pos = session_id.find(';');
            if (semicolon_pos != std::string::npos) {
                session_id = session_id.substr(0, semicolon_pos); // Extract the session ID
            }
        }
    }
    return session_id;
}

// Handle a request and respond based on the session
void handle_request(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
    std::string session_id = parse_session_id(req);

    if (session_id.empty() || session_store.find(session_id) == session_store.end()) {
        // No session or invalid session, generate a new one
        session_id = generate_session_id();
        session_store[session_id] = {{"status", "new"}};

        res.set(http::field::set_cookie, "session_id=" + session_id + "; HttpOnly");
    }

    // Retrieve session data
    auto& session_data = session_store[session_id];

    // Prepare the JSON response
    json::value response_body = {
        {"message", "Hello, this is your session"},
        {"session_id", session_id},
        {"session_data", session_data}
    };

    std::string json_string = json::serialize(response_body);

    res.result(http::status::ok);
    res.set(http::field::content_type, "application/json");
    res.body() = json_string;
    res.prepare_payload();
}

class session : public std::enable_shared_from_this<session> {
public:
    session(tcp::socket socket)
        : socket_(std::move(socket)) {}

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
                beast::error_code shutdown_ec;
                self->socket_.shutdown(tcp::socket::shutdown_send, shutdown_ec);
                if (shutdown_ec) {
                    std::cerr << "Shutdown error: " << shutdown_ec.message() << std::endl;
                }
            });
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
