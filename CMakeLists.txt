cmake_minimum_required(VERSION 3.28)

project(boost_rest_api VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.86 CONFIG REQUIRED COMPONENTS system json)

add_executable(boost_rest_api)

target_sources(boost_rest_api PRIVATE
    main.cpp)

target_include_directories(boost_rest_api PRIVATE
    ${Boost_INCLUDE_DIRS})

target_link_libraries(boost_rest_api PRIVATE
    ${Boost_LIBRARIES})

target_compile_definitions(boost_rest_api PRIVATE
    BOOST_ALL_NO_LIB
    BOOST_ALL_STATIC_LINK)

set_target_properties(boost_rest_api PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
