# Porting Quick Reference

## Overview
Quick reference guide for porting Go mump2p clients to C/C++ with Python bindings using a test-driven approach.

## Components to Port

### 1. Core Clients
- ✅ **Single P2P Client** (`grpc_p2p_client/cmd/single/main.go`)
  - Subscribe/Publish via gRPC
  - Bidirectional streaming
  - Message parsing

- ✅ **Multi-Publish Client** (`grpc_p2p_client/cmd/multi-publish/main.go`)
  - Concurrent publishing to multiple nodes
  - IP file reading with range selection
  - Output file writing

- ✅ **Multi-Subscribe Client** (`grpc_p2p_client/cmd/multi-subscribe/main.go`)
  - Concurrent subscription to multiple nodes
  - Trace collection
  - Data output files

- ✅ **Proxy Client** (`grpc_proxy_client/proxy_client.go`)
  - REST API (subscribe/publish)
  - gRPC bidirectional streaming
  - Client ID generation

### 2. Shared Utilities (`grpc_p2p_client/shared/`)
- ✅ IP file reading (`ReadIPsFromFile`)
- ✅ SHA256 hashing
- ✅ Hex encoding/decoding
- ✅ Base58 encoding (peer IDs)
- ✅ JSON message parsing
- ✅ Protobuf trace parsing (GossipSub + mump2p)
- ✅ File writing utilities

### 3. Protocol Definitions
- ✅ `proto/p2p_stream.proto` → C++ protobuf
- ✅ `proto/proxy_stream.proto` → C++ protobuf

## Porting Workflow

```
┌─────────────────────────────────────────────────────────┐
│ Phase 1: Test Generation (Week 1-2)                    │
│ ─────────────────────────────────────────────────────── │
│ • Unit tests for utilities                              │
│ • Integration tests with live Go nodes                  │
│ • End-to-end test scenarios                             │
│ • Test fixtures and expected outputs                    │
└─────────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────────┐
│ Phase 2: C/C++ Implementation (Week 3-6)               │
│ ─────────────────────────────────────────────────────── │
│ • Generate protobuf C++ code                            │
│ • Implement core types and utilities                    │
│ • Implement gRPC clients                               │
│ • Implement multi-node clients                         │
│ • Implement proxy client                               │
│ • Run tests continuously                                │
└─────────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────────┐
│ Phase 3: Python Bindings (Week 7-8)                    │
│ ─────────────────────────────────────────────────────── │
│ • Create pybind11 bindings                              │
│ • Create Python wrapper classes                        │
│ • Write Python tests                                    │
│ • Create examples                                       │
└─────────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────────┐
│ Phase 4: Integration & Validation (Week 9-10)          │
│ ─────────────────────────────────────────────────────── │
│ • Run full test suite                                   │
│ • Compare with Go implementation                       │
│ • Performance benchmarking                              │
│ • Documentation                                         │
└─────────────────────────────────────────────────────────┘
```

## Key Dependencies Mapping

| Go Package | C/C++ Library |
|------------|---------------|
| `google.golang.org/grpc` | gRPC C++ |
| `github.com/gogo/protobuf` | Protocol Buffers C++ |
| `crypto/sha256` | OpenSSL |
| `encoding/json` | nlohmann/json |
| `encoding/hex` | Custom implementation |
| `github.com/mr-tron/base58` | libbase58 |
| `github.com/libp2p/go-libp2p-pubsub/pb` | Generated protobuf |

## Test Categories

### Unit Tests (Fast, No Dependencies)
- File I/O operations
- Crypto functions (SHA256, hex, base58)
- Message parsing
- Protocol serialization

### Integration Tests (Require Docker)
- Single client subscribe/publish
- Multi-node concurrent operations
- Proxy client REST + gRPC
- End-to-end workflows

### Comparison Tests
- Output format matching Go
- Behavior verification
- Edge case handling

## Critical Functions to Port

### Utilities
```cpp
std::vector<std::string> ReadIPsFromFile(const std::string& filename);
std::string SHA256Hex(const std::vector<uint8_t>& data);
P2PMessage ParseMessage(const std::vector<uint8_t>& json_data);
void HandleGossipSubTrace(const std::vector<uint8_t>& data);
void HandleOptimumP2PTrace(const std::vector<uint8_t>& data);
```

### Client APIs
```cpp
class P2PClient {
    bool Subscribe(const std::string& topic);
    bool Publish(const std::string& topic, const std::vector<uint8_t>& data);
    bool ReceiveMessage(P2PMessage& msg, std::chrono::milliseconds timeout);
};

class ProxyClient {
    bool Subscribe(const std::string& client_id, const std::string& topic, double threshold);
    bool Publish(const std::string& client_id, const std::string& topic, const std::string& message);
    bool ConnectStream(const std::string& client_id);
};
```

## Build Commands

### C/C++ Build
```bash
mkdir build && cd build
cmake ..
make
make test
```

### Python Build
```bash
cd python
pip install -e .
pytest tests/
```

### Integration Tests
```bash
docker-compose -f docker-compose-optimum.yml up -d
./build/tests/integration/test_single_client
docker-compose -f docker-compose-optimum.yml down
```

## File Structure

```
optimum-p2p-client-cpp/
├── include/optimum_p2p/     # C++ headers
├── src/                      # C++ implementation
├── proto/                    # Protobuf definitions
├── tests/                    # Test suite
│   ├── unit/
│   ├── integration/
│   └── comparison/
├── python/                   # Python bindings
│   ├── bindings/            # pybind11 wrappers
│   └── optimum_p2p/         # Python modules
└── examples/                 # Example code
```

## Testing Checklist

### Phase 1: Tests
- [ ] Unit test framework setup
- [ ] Utility function tests
- [ ] Protocol tests
- [ ] Integration test framework
- [ ] Single client tests
- [ ] Multi-client tests
- [ ] Proxy client tests
- [ ] Test fixtures created

### Phase 2: C/C++
- [ ] Protobuf code generated
- [ ] Core types implemented
- [ ] Utilities implemented
- [ ] Single client implemented
- [ ] Multi-clients implemented
- [ ] Proxy client implemented
- [ ] All tests passing

### Phase 3: Python
- [ ] pybind11 bindings created
- [ ] Python wrappers implemented
- [ ] Python tests passing
- [ ] Examples working

### Phase 4: Integration
- [ ] Full test suite passing
- [ ] Comparison with Go verified
- [ ] Performance benchmarks done
- [ ] Documentation complete

## Common Pitfalls

1. **Protobuf Compatibility**: Ensure C++ protobuf matches Go version
2. **Memory Management**: Use smart pointers, avoid leaks
3. **Thread Safety**: Multi-client uses concurrent connections
4. **Error Handling**: Provide clear error messages
5. **Message Format**: Verify exact format matching Go output

## Quick Start Commands

```bash
# 1. Generate tests first
cd tests && python generate_tests.py

# 2. Build C++ library
mkdir build && cd build
cmake .. && make

# 3. Run tests
ctest --output-on-failure

# 4. Build Python bindings
cd ../python && pip install -e .

# 5. Test Python
python -m pytest tests/
```

## Resources

- **Full Guide**: See `PORTING_GUIDELINE.md`
- **gRPC C++**: https://grpc.io/docs/languages/cpp/
- **pybind11**: https://pybind11.readthedocs.io/
- **Protocol Buffers**: https://developers.google.com/protocol-buffers/docs/cpptutorial

## Timeline Estimate

- **Phase 1** (Tests): 2 weeks
- **Phase 2** (C/C++): 4 weeks  
- **Phase 3** (Python): 2 weeks
- **Phase 4** (Integration): 2 weeks
- **Total**: ~10 weeks

---

**Remember**: Test first, implement second. The tests will guide your implementation and catch regressions early.

