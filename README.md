# Optimum P2P Client C++

C/C++ implementation of the mump2p client libraries, ported from the Go implementation.

## Overview

This repository contains the C/C++ port of the mump2p client libraries originally written in Go. Currently in **Phase 1: Test Generation**.

The implementation will provide:

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
├── src/                         # C++ implementation (Phase 2)
│   ├── client.cpp
│   ├── multi_client.cpp
│   ├── proxy_client.cpp
│   └── utils.cpp
├── proto/                       # Protocol buffer definitions
│   ├── p2p_stream.proto
│   ├── proxy_stream.proto
│   └── CMakeLists.txt
├── tests/                       # Test suite (Phase 1)
│   ├── unit/                   # Unit tests
│   ├── integration/            # Integration tests
│   ├── e2e/                    # End-to-end tests
│   ├── comparison/             # Go vs C++ comparison tests
│   └── fixtures/              # Test data
└── go-reference/               # Git submodule to Go implementation
```

## Prerequisites

### Required Dependencies

**Note:** Even for Phase 1, the build system requires Phase 2 dependencies because tests link against the library. These will be used when implementing Phase 2.

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
- **pybind11** >= 2.6 (for Python bindings - Phase 3)

## Building

### Phase 1: Build and Run Tests

**Step 1: Create build directory**
```bash
cd optimum-p2p-client-cpp
mkdir -p build
cd build
```

**Step 2: Configure CMake**
```bash
cmake .. -DBUILD_TESTS=ON
```

**Step 3: Build unit tests**
```bash
make test_utils test_utils_hex
```

**Step 4: Run unit tests**
```bash
./tests/unit/test_utils
./tests/unit/test_utils_hex
```

**Or use CTest:**
```bash
ctest --output-on-failure -R "test_utils"
```

### Phase 2: Build Library (Future)

```bash
mkdir build && cd build
cmake ..
make
```

## Testing (Phase 1)

### Quick Start: Run All Unit Tests

From the repository root:
```bash
cd build
cmake .. -DBUILD_TESTS=ON
make test_utils test_utils_hex
ctest --output-on-failure
```

### Unit Tests (Detailed)

**Build and run individual test:**
```bash
cd build
make test_utils
./tests/unit/test_utils
```

**Run with verbose output:**
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

## Usage (Phase 2 - Future)

Implementation examples will be available after Phase 2 completion. See `PORTING_GUIDELINE.md` for planned API structure.

## Development

### Porting Status

This is an active porting project following a test-driven approach. See `PORTING_GUIDELINE.md` for the complete porting strategy and `PORTING_QUICK_REFERENCE.md` for a quick overview.

### Current Phase

- [x] **Phase 1: Test Generation** - ✅ COMPLETE
- [ ] Phase 2: C/C++ Implementation
- [ ] Phase 3: Python Bindings
- [ ] Phase 4: Integration & Validation

### Phase 1 Status

Phase 1 (Test Generation) is complete with:

- ✅ Comprehensive unit tests for utilities
- ✅ Integration tests for all client types
- ✅ End-to-end test scenarios
- ✅ Comparison tests with Go implementation
- ✅ Test fixtures and expected outputs

Tests are ready to guide Phase 2 implementation.

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
5. Current focus: Phase 2 implementation guided by Phase 1 tests

## License

[Add license information]

## Related Projects

- [optimum-dev-setup-guide](https://github.com/getoptimum/optimum-dev-setup-guide) - Go implementation and setup guide
