# Porting Guide: Go to C/C++ with Python Bindings

## Overview

This document provides a comprehensive step-by-step guide for porting the Go-based mump2p client libraries (`grpc_p2p_client` and `grpc_proxy_client`) to C/C++ with Python bindings. The approach follows a test-driven development methodology: **first generate comprehensive tests, then use those tests to guide the C/C++ implementation**.

## Table of Contents

1. [Understanding the Codebase](#understanding-the-codebase)
2. [Porting Strategy](#porting-strategy)
3. [Phase 1: Test Generation](#phase-1-test-generation)
4. [Phase 2: C/C++ Core Implementation](#phase-2-cc-core-implementation)
5. [Phase 3: Python Bindings](#phase-3-python-bindings)
6. [Phase 4: Integration & Validation](#phase-4-integration--validation)
7. [Testing Strategy](#testing-strategy)
8. [Dependencies & Tools](#dependencies--tools)
9. [Project Structure](#project-structure)

---

## Understanding the Codebase

### Components to Port

#### 1. **P2P Client (`grpc_p2p_client/`)**
   - **Single Node Client** (`cmd/single/main.go`)
     - Subscribe to topics via gRPC
     - Publish messages to topics
     - Handle bidirectional streaming
     - Parse and display messages with timestamps
   
   - **Multi-Publish Client** (`cmd/multi-publish/main.go`)
     - Concurrent publishing to multiple nodes
     - IP file reading and range selection
     - Random message generation
     - SHA256 hash computation
     - Optional output file writing
   
   - **Multi-Subscribe Client** (`cmd/multi-subscribe/main.go`)
     - Concurrent subscription to multiple nodes
     - Message and trace collection
     - TSV file output for data and traces
     - Graceful shutdown handling

#### 2. **Proxy Client (`grpc_proxy_client/`)**
   - REST API integration (subscribe/publish endpoints)
   - gRPC bidirectional streaming
   - Client ID generation
   - Message publishing loop with delays

#### 3. **Shared Utilities (`grpc_p2p_client/shared/`)**
   - **types.go**: Command enums and message structures
   - **utils.go**: 
     - IP file reading
     - Message response handling
     - GossipSub trace parsing
     - mump2p trace parsing
     - File writing utilities
     - SHA256 hashing

#### 4. **Protocol Definitions**
   - `proto/p2p_stream.proto`: P2P node gRPC service
   - `proto/proxy_stream.proto`: Proxy gRPC service

### Key Dependencies (Go → C/C++ Mapping)

| Go Package | C/C++ Equivalent |
|-----------|-------------------|
| `google.golang.org/grpc` | gRPC C++ library |
| `github.com/gogo/protobuf/proto` | Protocol Buffers C++ |
| `crypto/sha256` | OpenSSL SHA256 or system crypto |
| `encoding/json` | RapidJSON or nlohmann/json |
| `encoding/hex` | Custom hex encoding/decoding |
| `github.com/mr-tron/base58` | Base58 library (libbase58) |
| `github.com/libp2p/go-libp2p-pubsub/pb` | Generated protobuf C++ code |
| `github.com/libp2p/go-libp2p/core/peer` | Custom peer ID handling |

---

## Porting Strategy

### Test-Driven Approach

1. **Phase 1: Test Generation** (Week 1-2)
   - Generate comprehensive test suites covering all functionality
   - Create integration tests that work with live Go implementation
   - Document expected behaviors and edge cases

2. **Phase 2: C/C++ Core** (Week 3-6)
   - Implement core functionality guided by tests
   - Port protocol definitions (protobuf)
   - Implement gRPC client wrappers
   - Port utility functions

3. **Phase 3: Python Bindings** (Week 7-8)
   - Create Python C extension or use pybind11
   - Expose C/C++ APIs to Python
   - Create Pythonic wrapper classes

4. **Phase 4: Integration** (Week 9-10)
   - Run full test suite against C/C++ implementation
   - Validate Python bindings
   - Performance benchmarking
   - Documentation

---

*[Full content continues - this is a summary version. See the original file for complete details]*

**Estimated Timeline:**
- Phase 1 (Tests): 2 weeks
- Phase 2 (C/C++): 4 weeks
- Phase 3 (Python): 2 weeks
- Phase 4 (Integration): 2 weeks
- **Total: ~10 weeks**

Good luck with the porting effort!

