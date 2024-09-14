cmake_minimum_required(VERSION 3.28)

project(rest_seed VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.86 CONFIG REQUIRED COMPONENTS system json)
add_definitions(-DBOOST_ALL_NO_LIB)
add_definitions(-DBOOST_ALL_STATIC_LINK)

add_executable(server src/server.cpp)

target_include_directories(server PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(server PRIVATE ${Boost_LIBRARIES})

set_target_properties(server PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
