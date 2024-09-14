# Rest Server Seed
This project is a simple C++ project in docker image. This has added boost support.

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

## Boost
Download Boost version `1.86.0` from [Boost's official site](https://www.boost.org/users/history/version_1_86_0.html) and place the file (`boost_1_86_0.tar.gz`) in the root directory of the repository. Update the [build.sh](./build.sh) script and the [Dockerfile](./Dockerfile) to reflect this change.

### Boost linking directory
- Compiler include paths: `<boost path>/boost_1_86_0`.
- Linker library paths: `<boost path>/boost_1_86_0/stage/lib`.
