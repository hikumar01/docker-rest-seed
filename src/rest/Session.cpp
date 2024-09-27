#include "rest/Session.h"
#include "rest/RestController.h"
#include <boost/json.hpp>
#include <iostream>

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
    RestController::getInstance()->handle_request(req_, res_);
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
