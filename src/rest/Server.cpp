#include "rest/Server.h"
#include "rest/Session.h"

Server::Server(boost::asio::io_context& ioc, boost::asio::ip::tcp::endpoint endpoint) : acceptor(ioc) {
    boost::system::error_code ec;
    if (acceptor.open(endpoint.protocol(), ec); ec) {
        throw std::runtime_error("Open error: " + ec.message());
    }
    if (acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec); ec) {
        throw std::runtime_error("Set option error: " + ec.message());
    }
    if (acceptor.bind(endpoint, ec); ec) {
        throw std::runtime_error("Bind error: " + ec.message());
    }
    if (acceptor.listen(boost::asio::socket_base::max_listen_connections, ec); ec) {
        throw std::runtime_error("Listen error: " + ec.message());
    }
    do_accept();
}

void Server::do_accept() {
    acceptor.async_accept(
        [this](boost::beast::error_code ec, boost::asio::ip::tcp::socket socket) {
            if (ec) {
                do_accept(); // Retry accepting
                throw std::runtime_error("Accept error: " + ec.message());
            } else {
                std::make_shared<Session>(std::move(socket))->run();
                do_accept(); // Continue accepting new connections
            }
        });
}
