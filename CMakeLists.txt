cmake_minimum_required(VERSION 3.15)
project(optimum_p2p_client_cpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_INTEGRATION_TESTS "Build integration tests" OFF)
option(BUILD_E2E_TESTS "Build end-to-end tests" OFF)
option(BUILD_COMPARISON_TESTS "Build comparison tests" OFF)
option(BUILD_PYTHON "Build Python bindings" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_BINARY_DIR}/proto)

# Find dependencies
# Find gRPC first - it will bring in Protobuf as a dependency
# We'll use protoc directly via protobuf::protoc target, avoiding protobuf_generate_cpp
find_package(gRPC REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

# nlohmann_json is header-only, use FetchContent
include(FetchContent)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)
FetchContent_MakeAvailable(json)

# Generate protobuf code
add_subdirectory(proto)

# Library sources
set(SOURCES
    src/client.cpp
    src/utils.cpp
    src/proxy_client.cpp
    src/multi_client.cpp
)

set(HEADERS
    include/optimum_p2p/client.hpp
    include/optimum_p2p/types.hpp
    include/optimum_p2p/utils.hpp
    include/optimum_p2p/proxy_client.hpp
    include/optimum_p2p/multi_client.hpp
)

# Create library
add_library(optimum_p2p_client STATIC ${SOURCES} ${HEADERS})

target_link_libraries(optimum_p2p_client
    PUBLIC
    optimum_proto
    gRPC::grpc++
    protobuf::libprotobuf
    OpenSSL::SSL
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
    CURL::libcurl
)

# Include nlohmann_json headers (FetchContent provides the target)
target_include_directories(optimum_p2p_client
    PUBLIC
    $<BUILD_INTERFACE:${nlohmann_json_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Install rules
install(TARGETS optimum_p2p_client
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/optimum_p2p
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Python bindings
if(BUILD_PYTHON)
    add_subdirectory(python)
endif()

