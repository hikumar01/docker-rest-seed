#pragma once

#include <boost/asio.hpp>
#include <boost/asio/strand.hpp>

namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class Server {
public:
    Server(net::io_context& ioc, tcp::endpoint endpoint);

private:
    void do_accept();

    tcp::acceptor acceptor_;
};
