# Optimum P2P Client C++

C/C++ implementation of the mump2p client libraries, ported from the Go implementation.

## Overview

This repository contains the C/C++ port of the mump2p client libraries originally written in Go.

The implementation provides:

- **P2P Client**: Direct connection to P2P nodes via gRPC
- **Multi-Node Clients**: Concurrent publishing and subscription across multiple nodes
- **Proxy Client**: REST API and gRPC streaming client for proxy connections

## Repository Structure

```text
optimum-p2p-client-cpp/
├── CMakeLists.txt              # Root CMake configuration
├── README.md                    # This file
├── PORTING_GUIDELINE.md         # Detailed porting guide
├── PORTING_QUICK_REFERENCE.md   # Quick reference
├── .gitignore                   # Git ignore rules
├── .gitmodules                  # Git submodule configuration
├── include/                     # C++ header files
│   └── optimum_p2p/
│       ├── client.hpp
│       ├── multi_client.hpp
│       ├── proxy_client.hpp
│       ├── types.hpp
│       └── utils.hpp
├── src/                         # C++ implementation
│   ├── client.cpp
│   ├── multi_client.cpp
│   ├── proxy_client.cpp
│   └── utils.cpp
├── proto/                       # Protocol buffer definitions
│   ├── p2p_stream.proto
│   ├── proxy_stream.proto
│   └── CMakeLists.txt
├── tests/                       # Test suite
│   ├── unit/                   # Unit tests
│   ├── integration/            # Integration tests
│   ├── e2e/                    # End-to-end tests
│   ├── comparison/             # Go vs C++ comparison tests
│   └── fixtures/              # Test data
└── go-reference/               # Git submodule to Go implementation
```

## Prerequisites

### Required Dependencies

- **CMake** >= 3.15
- **C++17** compatible compiler (GCC 7+, Clang 5+)
- **Google Test** >= 1.10 (for testing)
- **gRPC** >= 1.40
- **Protocol Buffers** >= 3.15
- **OpenSSL** >= 1.1
- **nlohmann/json** >= 3.9
- **libcurl** >= 7.70

### Optional Dependencies

- **libbase58**: Base58 encoding
- **spdlog**: Logging library
- **Catch2**: Alternative testing framework
- **pybind11** >= 2.6 (for Python bindings)

## Building

### Build and Run Tests

#### Step 1: Create build directory

```bash
cd optimum-p2p-client-cpp
mkdir -p build
cd build
```

#### Step 2: Configure CMake

```bash
cmake .. -DBUILD_TESTS=ON
```

#### Step 3: Build unit tests

```bash
make test_utils test_utils_hex
```

#### Step 4: Run unit tests

```bash
./tests/unit/test_utils
./tests/unit/test_utils_hex
```

Or use CTest:

```bash
ctest --output-on-failure -R "test_utils"
```

### Build Library

```bash
mkdir build && cd build
cmake ..
make
```

## Testing

### Quick Start: Run All Unit Tests

From the repository root:

```bash
cd build
cmake .. -DBUILD_TESTS=ON
make test_utils test_utils_hex
ctest --output-on-failure
```

### Unit Tests (Detailed)

Build and run individual test:

```bash
cd build
make test_utils
./tests/unit/test_utils
```

Run with verbose output:

```bash
./tests/unit/test_utils --gtest_color=yes --gtest_output=xml
```

### Integration Tests (Requires Docker)

Integration tests require Docker and the P2P nodes to be running:

```bash
# Start P2P nodes (from go-reference submodule)
cd go-reference
docker-compose -f docker-compose-optimum.yml up -d

# Build and run integration tests
cd ../build
cmake .. -DBUILD_INTEGRATION_TESTS=ON
make test_single_client test_multi_client test_proxy_client

# Run tests (remove DISABLED_ prefix from test names first)
./tests/integration/test_single_client

# Stop nodes
cd ../go-reference
docker-compose -f docker-compose-optimum.yml down
```

### End-to-End Tests

```bash
cd build
cmake .. -DBUILD_E2E_TESTS=ON
make test_full_workflow
./tests/e2e/test_full_workflow
```

### Comparison Tests

```bash
cd build
cmake .. -DBUILD_COMPARISON_TESTS=ON
make test_go_vs_cpp
./tests/comparison/test_go_vs_cpp
```

## Usage

### C++ Example

```cpp
#include "optimum_p2p/client.hpp"

int main() {
    optimum_p2p::P2PClient client("127.0.0.1:33221");
    
    // Subscribe to topic
    client.Subscribe("mytopic");
    
    // Set message callback
    client.SetMessageCallback([](const optimum_p2p::P2PMessage& msg) {
        std::cout << "Received: " << std::string(msg.message.begin(), msg.message.end()) << std::endl;
    });
    
    // Publish message
    std::vector<uint8_t> data = {'H', 'e', 'l', 'l', 'o'};
    client.Publish("mytopic", data);
    
    // Keep running
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    client.Shutdown();
    return 0;
}
```

## Development

This project follows a test-driven development approach. See `PORTING_GUIDELINE.md` for the complete porting strategy and `PORTING_QUICK_REFERENCE.md` for a quick overview.

## Reference Implementation

The Go reference implementation is available as a Git submodule in `go-reference/`. This allows:

- Running comparison tests
- Referencing the original implementation
- Using Go clients for integration testing

To initialize the submodule:

```bash
git submodule update --init --recursive
```

## Contributing

1. Follow the test-driven approach outlined in `PORTING_GUIDELINE.md`
2. Ensure all tests pass before submitting
3. Maintain compatibility with the Go implementation
4. Follow C++17 best practices

## Related Projects

- [optimum-dev-setup-guide](https://github.com/getoptimum/optimum-dev-setup-guide) - Go implementation and setup guide
