cmake_minimum_required(VERSION 3.28)

project(rest_api VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.86 CONFIG REQUIRED COMPONENTS system json)

add_executable(rest_api)

target_sources(rest_api PRIVATE
    src/rest/Server.cpp
    src/rest/Session.cpp
    src/rest/RestController.cpp
    src/main.cpp)

target_include_directories(rest_api PRIVATE
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(rest_api PRIVATE ${Boost_LIBRARIES})

target_compile_definitions(rest_api PRIVATE BOOST_ALL_NO_LIB BOOST_ALL_STATIC_LINK)
set_target_properties(rest_api PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
