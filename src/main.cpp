#include "rest/RestController.h"
#include <iostream>

int main(int argc, char* argv[]) {
    const int port = 80; // This port should match with the port in the Dockerfile
    const int num_threads = 1;
    std::cout << "Server running on http://localhost:" << port << std::endl;
    try {
        start_server(port, num_threads);
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}
