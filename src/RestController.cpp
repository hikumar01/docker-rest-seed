#include "Server.h"
#include "Session.h"
#include <boost/asio.hpp>
#include <iostream>

void start_server(const int& port, const int& num_threads) {
    try {
        boost::asio::io_context ioc{num_threads};
        boost::asio::ip::tcp::endpoint endpoint{boost::asio::ip::tcp::v4(), static_cast<unsigned short>(port)};

        auto srv = std::make_shared<Server>(ioc, endpoint);

        ioc.run();
    } catch (const std::runtime_error& e) {
        std::cerr << "Runtime Exception: " << e.what() << std::endl;
        throw;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        throw;
    }
}
