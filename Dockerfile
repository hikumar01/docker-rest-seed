FROM alpine:3.20

# Setting up system
ARG APP_DIR=/app
RUN apk add --no-cache g++ make cmake wget linux-headers

# Compiling boost
WORKDIR ${APP_DIR}
RUN wget -c --progress=bar:force https://archives.boost.io/release/1.86.0/source/boost_1_86_0.tar.gz
RUN mkdir -p boost
RUN tar -xzf boost_1_86_0.tar.gz -C boost
WORKDIR ${APP_DIR}/boost/boost_1_86_0
RUN ./bootstrap.sh
RUN ./b2 link=static --with-system --with-json
# RUN ./b2 link=static --with-system --with-json install --prefix=/usr/local
# RUN ./b2 link=shared --with-system --with-json install --prefix=/usr/local

# Setting up the project
WORKDIR ${APP_DIR}
COPY CMakeLists.txt .
COPY src/ src/

# Compiling the project
WORKDIR ${APP_DIR}/build
RUN cmake -DBOOST_ROOT=../boost/boost_1_86_0/stage -DCMAKE_BUILD_TYPE=Release ..
# RUN cmake -DCMAKE_BUILD_TYPE=Release ..
RUN cmake --build . --verbose

# Clean up
WORKDIR ${APP_DIR}
RUN rm -rf boost build boost_1_86_0.tar.gz src CMakeLists.txt

# Running the server
EXPOSE 80
CMD ["./bin/server"]
