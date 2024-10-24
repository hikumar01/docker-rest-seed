#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/json.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <mutex>
#include <fstream>
#include <sstream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

// Aliases
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace json = boost::json;
using tcp = boost::asio::ip::tcp;
using clock = std::chrono::steady_clock;

// Session Management with Persistent Storage
class SessionManager {
public:
    SessionManager(const std::string& storageFile)
        : storageFile_(storageFile) {
        loadSessions();
    }

    void addSession(const std::string& sessionId, clock::time_point expiration) {
        std::lock_guard<std::mutex> lock(mutex_);
        sessions_[sessionId] = expiration;
        saveSessions();
    }

    bool isSessionValid(const std::string& sessionId) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = sessions_.find(sessionId);
        if (it != sessions_.end() && it->second > clock::now()) {
            return true;
        }
        return false;
    }

    void cleanupExpiredSessions() {
        std::lock_guard<std::mutex> lock(mutex_);
        auto now = clock::now();
        for (auto it = sessions_.begin(); it != sessions_.end(); ) {
            if (it->second <= now) {
                it = sessions_.erase(it); // Remove expired session
            } else {
                ++it;
            }
        }
        saveSessions();
    }

private:
    std::unordered_map<std::string, clock::time_point> sessions_;
    std::mutex mutex_;
    std::string storageFile_;

    void loadSessions() {
        std::ifstream file(storageFile_);
        if (file) {
            std::string line;
            while (std::getline(file, line)) {
                std::istringstream iss(line);
                std::string sessionId;
                std::chrono::milliseconds::rep expiration;
                if (iss >> sessionId >> expiration) {
                    sessions_[sessionId] = clock::time_point(clock::duration(expiration));
                }
            }
        }
    }

    void saveSessions() {
        std::ofstream file(storageFile_, std::ios::trunc);
        if (file) {
            for (const auto& [sessionId, expiration] : sessions_) {
                file << sessionId << ' ' << expiration.time_since_epoch().count() << '\n';
            }
        }
    }
};

// Advanced Logging
void initLogging() {
    auto file_logger = spdlog::basic_logger_mt("file_logger", "server.log");
    spdlog::set_default_logger(file_logger);
    spdlog::set_level(spdlog::level::info);
}

// Logging function
void log(const std::string& message) {
    spdlog::info(message);
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
        : socket_(std::move(socket)), sessionManager_(sessionManager), timer_(socket.get_executor()) {}

    void start() {
        readRequest();
    }

private:
    tcp::socket socket_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    http::response<http::string_body> res_;
    SessionManager& sessionManager_;
    net::steady_timer timer_;
    std::chrono::seconds timeoutDuration{30}; // Set timeout duration

    void readRequest() {
        auto self = shared_from_this();
        timer_.expires_after(timeoutDuration);
        timer_.async_wait([self](beast::error_code ec) {
            if (!ec) {
                self->handleTimeout();
            }
        });

        http::async_read(socket_, buffer_, req_,
            [self](beast::error_code ec, std::size_t) {
                if (!ec) {
                    self->timer_.cancel(); // Cancel timer on successful read
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

    void handleTimeout() {
        log("Request timed out");
        res_.result(http::status::request_timeout);
        res_.body() = "Request timed out";
        writeResponse();
    }
};

// Server
class Server {
public:
    Server(net::io_context& ioc, tcp::endpoint endpoint, SessionManager& sessionManager)
        : acceptor_(ioc), sessionManager_(sessionManager) {
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
    SessionManager& sessionManager_;

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
        initLogging(); // Initialize logging

        net::io_context ioc{std::thread::hardware_concurrency()}; // Use multiple threads

        SessionManager sessionManager("sessions.txt"); // Persistent storage file
        tcp::endpoint endpoint{tcp::v4(), static_cast<unsigned short>(8080)};
        Server server(ioc, endpoint, sessionManager);

        // Create a thread pool to run the io_context
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
