cmake_minimum_required(VERSION 3.28)

project(rest_api VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find Boost Libraries
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.86 CONFIG REQUIRED COMPONENTS system json)

# Create the executable target
add_executable(rest_api)

# Source files
set(SOURCE_FILES
    src/Server.cpp
    src/Session.cpp
    src/RestController.cpp
    src/compare/Levenshtein.cpp
    src/main.cpp)

# Add sources to the target
target_sources(rest_api PRIVATE
    ${SOURCE_FILES})

# Include directories
target_include_directories(rest_api PRIVATE
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include)

# Link libraries
target_link_libraries(rest_api PRIVATE
    ${Boost_LIBRARIES})

# Compile definitions
target_compile_definitions(rest_api PRIVATE
    BOOST_ALL_NO_LIB
    BOOST_ALL_STATIC_LINK)

# Set runtime output directory
set_target_properties(rest_api PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Organize files into groups
source_group("Source" FILES ${SOURCE_FILES})
source_group("Header" FILES ${CMAKE_SOURCE_DIR}/include/*.h)

# Organize Boost into groups
source_group("Boost" FILES ${Boost_INCLUDE_DIRS}/*.hpp)

# Organize UI files
set(UI_DIR ${CMAKE_SOURCE_DIR}/ui)
file(GLOB UI_FILES "${UI_DIR}/*")
source_group("UI" FILES ${UI_FILES})

# Organize resource files (if applicable)
set(RESOURCE_DIR ${CMAKE_SOURCE_DIR}/resources)
file(GLOB RESOURCE_FILES "${RESOURCE_DIR}/*")
source_group("Resources" FILES ${RESOURCE_FILES})
