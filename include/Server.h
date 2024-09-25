#pragma once

#include <boost/asio.hpp>

class Server {
public:
    Server(boost::asio::io_context& ioc, boost::asio::ip::tcp::endpoint endpoint);

private:
    void do_accept();

    boost::asio::ip::tcp::acceptor acceptor;
};
