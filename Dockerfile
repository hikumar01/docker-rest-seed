FROM alpine:3.20

# Setting up system
RUN apk add --no-cache g++ make cmake linux-headers

# Compiling boost
WORKDIR /server
COPY boost_1_86_0.tar.gz .
RUN mkdir -p boost
RUN tar -xzf boost_1_86_0.tar.gz -C boost
WORKDIR /server/boost/boost_1_86_0
RUN ./bootstrap.sh
# RUN ./b2 link=static --with-system --with-json
RUN ./b2 link=static --with-system --with-json install
# RUN ./b2 link=shared --with-system --with-json install

# Setting up the project
WORKDIR /server
COPY CMakeLists.txt .
COPY main.cpp .

# Compiling the project
WORKDIR /server/build
RUN cmake -DBOOST_ROOT=../boost/boost_1_86_0/stage -DCMAKE_BUILD_TYPE=Release ..
# RUN cmake -DCMAKE_BUILD_TYPE=Release ..
RUN cmake --build . --verbose

# Clean up
WORKDIR /server
RUN rm -rf boost build boost_1_86_0.tar.gz src main.cpp CMakeLists.txt

# Running the server
EXPOSE 80
CMD ["./server"]
