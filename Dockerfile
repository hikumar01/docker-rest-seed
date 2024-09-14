FROM alpine:3.20

# Setting up system
RUN apk add --no-cache g++ make cmake

# Setting up the project
WORKDIR /server
COPY CMakeLists.txt .
COPY main.cpp .

# Compiling the project
WORKDIR /server/build
RUN cmake -DCMAKE_BUILD_TYPE=Release ..
RUN cmake --build . --verbose

# Clean up
WORKDIR /server
RUN rm -rf build main.cpp CMakeLists.txt

# Running the server
EXPOSE 80
CMD ["./server"]
