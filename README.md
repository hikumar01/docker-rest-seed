# Rest Server Seed
This project is a simple C++ project in docker image.

## Prerequisites
- C++17 compatible compiler
- CMake 3.28 or higher
- Docker (optional, for containerized deployment)

## Building the Project
### Using CMake
Run [build.sh](./build.sh)

### Using Docker
1. Build the Docker image:
    - With Cache:
        ```sh
        docker build -t rest_seed .
        ```
    - Without Cache:
        ```sh
        docker build --no-cache -t rest_seed .
        ```
2. Run the Docker container:
    ```sh
    docker run -p 8080:80 rest_seed
    ```

## Docker commands
* Stop Docker commands
    ```sh
    docker ps
    docker stop <docker_id>
    ```
