#include "RestController.h"
#include <iostream>

int main() {
    try {
        const int port = 8080;
        const int num_threads = 1;
        std::cout << "Server running on http://localhost:" << port << std::endl;
        start_server(port, num_threads);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}