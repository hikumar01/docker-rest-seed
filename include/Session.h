#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast.hpp>

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(boost::asio::ip::tcp::socket socket) : socket_(std::move(socket)) {}
    void run();

private:
    void read_request();
    void process_request();
    void write_response();

    boost::asio::ip::tcp::socket socket_;
    boost::beast::flat_buffer buffer_;
    boost::beast::http::request<boost::beast::http::string_body> req_;
    boost::beast::http::response<boost::beast::http::string_body> res_;
};
