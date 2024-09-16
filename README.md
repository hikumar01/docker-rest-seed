# Rest API
This project is a simple HTTP program built using Boost.Beast and Boost.Asio libraries. It demonstrates handling HTTP requests and responses in a modern C++ environment.

## Prerequisites
- C++17 compatible compiler
- CMake `3.28` or higher
- wget
- Docker (optional, for containerized deployment)

## Building the Project
### Using CMake
Run [setup.sh](./setup.sh)

### Using Docker
1. Build the Docker image:
    - With Cache:
        ```sh
        docker build -t rest_api .
        ```
    - Without Cache:
        ```sh
        docker build --no-cache -t rest_api .
        ```
2. Run the Docker container:
    ```sh
    docker run -p 8080:80 rest_api
    ```

## Testing the API's
Once the container is running, you can test it by sending a request:
```bash
curl http://localhost:8080
```

Expected response:
```json
{
    "message": "Welcome to the REST API",
    "status": "success"
}
```

### Quick - Testing the API's
```bash
docker build -t rest_api . && docker run -p 8080:80 rest_api
echo -n "\n\nGET http://localhost:8080:\n" && curl http://localhost:8080; echo -n "\n\nGET http://localhost:8080/status:\n" && curl http://localhost:8080/status; echo -n "\n\nGET http://localhost:8080/error:\n" && curl http://localhost:8080/error; echo -n "\n\nPOST http://localhost:8080:\n" && curl -X POST http://localhost:8080
ab -n 300 -c 30 http://127.0.0.1:8080/
```

## Docker commands
* Stop Docker commands using docker name
    ```sh
    docker stop <docker_name>
    ```
* Stop & Delete Docker commands using docker id
    ```sh
    docker ps
    docker stop <docker_id>
    docker rm <docker_id>
    ```
* Stop & Delete all dockers
    ```sh
    docker stop $(docker ps -q)
    docker rm $(docker ps -aq)
    ```

## Boost Library
Boost version [1.86.0](https://archives.boost.io/release/1.86.0/source/boost_1_86_0.tar.gz) is downloaded from [Boost's official site](https://www.boost.org).

### Linking Directory
- Compiler include paths: `<boost path>/boost_1_86_0`.
- Linker library paths: `<boost path>/boost_1_86_0/stage/lib`.

## Using the docker image
### Create a custom local docker iamge
To build or identify the Docker image, use the following command:
```bash
docker build -t cpp_rest_api:latest .
```
### Export the Image
To export the image, use the following command:
```bash
docker save -o cpp_rest_api.tar cpp_rest_api:latest
```

### Transfer or Copy the Exported Image
After creating the tar file (cpp_rest_api.tar), you can transfer it to another machine via USB drive or network transfer (like scp, rsync)

### Consume a custom local docker iamge
To import the Docker image on another machine, use the following command:
```bash
docker load -i cpp_rest_api.tar
```
To verify that the image is loaded, use the following command:
```bash
docker images
```

## Benchmarking
To benchmark the application, you can use ApacheBench with the following command:
```bash
ab -n 300 -c 30 http://127.0.0.1:8080/
```
