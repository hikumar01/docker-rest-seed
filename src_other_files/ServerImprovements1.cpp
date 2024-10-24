#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/json.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <mutex>
#include <fstream>

// Aliases
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace json = boost::json;
using tcp = boost::asio::ip::tcp;

// Session Management
class SessionManager {
public:
    // Add a session with a timeout
    void addSession(const std::string& sessionId, std::chrono::steady_clock::time_point expiration) {
        std::lock_guard<std::mutex> lock(mutex_);
        sessions_[sessionId] = expiration;
    }

    // Check session validity
    bool isSessionValid(const std::string& sessionId) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = sessions_.find(sessionId);
        if (it != sessions_.end() && it->second > std::chrono::steady_clock::now()) {
            return true;
        }
        return false;
    }

    // Clean expired sessions
    void cleanupExpiredSessions() {
        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::steady_clock::now();
        for (auto it = sessions_.begin(); it != sessions_.end(); ) {
            if (it->second <= now) {
                it = sessions_.erase(it); // Remove expired session
            } else {
                ++it;
            }
        }
    }

private:
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> sessions_;
    std::mutex mutex_;
};

// Logging
void log(const std::string& message) {
    std::ofstream logFile("server.log", std::ios_base::app);
    logFile << message << std::endl;
}

// HTTP Request Handler
void handleRequest(const http::request<http::string_body>& req, http::response<http::string_body>& res, SessionManager& sessionManager) {
    // Logging request
    log("Handling request: " + req.target().to_string());

    std::string sessionId = req[http::field::cookie];
    if (!sessionId.empty() && sessionManager.isSessionValid(sessionId)) {
        log("Session valid: " + sessionId);
    } else {
        log("Invalid or no session: " + sessionId);
    }

    // Handle different HTTP methods and routes
    std::string target(req.target());
    if (req.method() == http::verb::get) {
        if (target == "/") {
            json::value response_body = {{"message", "Welcome to the REST API"}, {"status", "success"}};
            std::string json_string = json::serialize(response_body);
            res.result(http::status::ok);
            res.set(http::field::content_type, "application/json");
            res.body() = json_string;
        } else if (target == "/hello") {
            json::value response_body = {{"message", "Hello, World!"}, {"status", "success"}};
            std::string json_string = json::serialize(response_body);
            res.result(http::status::ok);
            res.set(http::field::content_type, "application/json");
            res.body() = json_string;
        } else if (target == "/status") {
            json::value response_body = {{"status", "API is running smoothly"}};
            std::string json_string = json::serialize(response_body);
            res.result(http::status::ok);
            res.set(http::field::content_type, "application/json");
            res.body() = json_string;
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

// Session-based async timeouts and response handling
class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, SessionManager& sessionManager)
        : socket_(std::move(socket)), sessionManager_(sessionManager) {}

    void start() {
        readRequest();
    }

private:
    tcp::socket socket_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    http::response<http::string_body> res_;
    SessionManager& sessionManager_;

    void readRequest() {
        auto self = shared_from_this();
        http::async_read(socket_, buffer_, req_,
            [self](beast::error_code ec, std::size_t) {
                if (!ec) {
                    self->processRequest();
                } else {
                    log("Read error: " + ec.message());
                }
            });
    }

    void processRequest() {
        handleRequest(req_, res_, sessionManager_);
        writeResponse();
    }

    void writeResponse() {
        auto self = shared_from_this();
        http::async_write(socket_, res_,
            [self](beast::error_code ec, std::size_t) {
                if (ec) {
                    log("Write error: " + ec.message());
                }
                beast::error_code shutdown_ec;
                self->socket_.shutdown(tcp::socket::shutdown_send, shutdown_ec);
                if (shutdown_ec) {
                    log("Shutdown error: " + shutdown_ec.message());
                }
            });
    }
};

// Server
class Server {
public:
    Server(net::io_context& ioc, tcp::endpoint endpoint)
        : acceptor_(ioc), sessionManager_() {
        boost::system::error_code ec;
        acceptor_.open(endpoint.protocol(), ec);
        if (ec) {
            log("Open error: " + ec.message());
            return;
        }
        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if (ec) {
            log("Set option error: " + ec.message());
            return;
        }
        acceptor_.bind(endpoint, ec);
        if (ec) {
            log("Bind error: " + ec.message());
            return;
        }
        acceptor_.listen(net::socket_base::max_listen_connections, ec);
        if (ec) {
            log("Listen error: " + ec.message());
            return;
        }
        doAccept();
    }

private:
    tcp::acceptor acceptor_;
    SessionManager sessionManager_;

    void doAccept() {
        acceptor_.async_accept(
            [this](beast::error_code ec, tcp::socket socket) {
                if (ec) {
                    log("Accept error: " + ec.message());
                } else {
                    std::make_shared<Session>(std::move(socket), sessionManager_)->start();
                }
                doAccept(); // Continue accepting new connections
            });
    }
};

int main() {
    try {
        net::io_context ioc{std::thread::hardware_concurrency()}; // Use multiple threads

        tcp::endpoint endpoint{tcp::v4(), static_cast<unsigned short>(8080)};
        Server server(ioc, endpoint);

        std::vector<std::thread> threads;
        for (std::size_t i = 0; i < std::thread::hardware_concurrency(); ++i) {
            threads.emplace_back([&ioc] {
                ioc.run();
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }
    } catch (const std::exception& e) {
        log("Exception: " + std::string(e.what()));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
