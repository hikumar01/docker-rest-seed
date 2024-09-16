# Rest API
This project is a simple HTTP program built using Boost.Beast and Boost.Asio libraries. It demonstrates handling HTTP requests and responses in a modern C++ environment.

## Prerequisites
- C++17 compatible compiler
- CMake `3.28` or higher
- wget
- Docker (optional, for containerized deployment)

## Building the Project
### Using CMake
Run the [setup.sh](./setup.sh) script.

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
    docker run -p 8080:80 -d rest_api
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
docker build -t rest_api . && docker run -p 8080:80 -d rest_api;
echo -n "\n\nGET http://localhost:8080:\n" && curl http://localhost:8080;
echo -n "\n\nGET http://localhost:8080/status:\n" && curl http://localhost:8080/status;
echo -n "\n\nGET http://localhost:8080/error:\n" && curl http://localhost:8080/error;
echo -n "\n\nPOST http://localhost:8080:\n" && curl -X POST http://localhost:8080
```

## Docker commands
* Running a container from an image in attached mode:
    ```sh
    docker run -p 8080:80 <image_name>
    ```
* Running a container from an image in detached mode:
    ```sh
    docker run -p 8080:80 -d <image_name>
    ```
* Attaching a terminal to a detached container:
    ```sh
    docker attach <container_id_or_name>
    ```
    To exit the attached session, press `Ctrl + P`, followed by `Ctrl + Q`.
    Avoid pressing `Ctrl + C`, as it will inadvertently stop the container itself.
* Executing a command in a container without attaching:
    ```sh
    docker exec -it <container_id_or_name> /bin/sh
    ```
* Listing all containers:
    ```sh
    docker ps -a
    ```
* Starting a container:
    ```sh
    docker start <container_id_or_name>
    ```
* Stopping a container:
    ```sh
    docker stop <container_id_or_name>
    ```
* Deleting a container:
    ```sh
    docker rm <container_id_or_name>
    ```
* Stopping and deleting all containers:
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
### Create a custom local docker image
To build or identify the Docker image, use the following command:
```bash
docker build -t cpp_rest_api:latest .
```
### Export the image
To export the image, use the following command:
```bash
docker save -o cpp_rest_api.tar cpp_rest_api:latest
```

### Transfer or Copy the Exported image
After creating the tar file (cpp_rest_api.tar), you can transfer it to another machine via USB drive or network transfer (like scp, rsync)

### Consume a custom local docker image
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
