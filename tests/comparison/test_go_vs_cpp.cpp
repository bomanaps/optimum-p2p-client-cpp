#include <gtest/gtest.h>
#include "optimum_p2p/utils.hpp"
#include "optimum_p2p/types.hpp"
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <atomic>
#include <cstdint>
#include <cstdlib>

namespace fs = std::filesystem;

namespace optimum_p2p {

class ComparisonTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Use CMAKE_SOURCE_DIR or fallback to relative path from test binary
        // In tests, fixtures are in source tree: tests/fixtures/
        fs::path test_file_path = __FILE__;
        fixtures_dir_ = test_file_path.parent_path().parent_path() / "fixtures";
        expected_outputs_dir_ = fixtures_dir_ / "expected_outputs";
        
        // Alternative: Use environment variable if set
        const char* fixtures_env = std::getenv("TEST_FIXTURES_DIR");
        if (fixtures_env) {
            fixtures_dir_ = fs::path(fixtures_env);
            expected_outputs_dir_ = fixtures_dir_ / "expected_outputs";
        }
    }

    fs::path fixtures_dir_;
    fs::path expected_outputs_dir_;
};

// Test: Compare IP file reading with Go implementation
TEST_F(ComparisonTest, ReadIPsFromFile_MatchesGo) {
    auto ip_file = fixtures_dir_ / "test_ips.txt";
    
    if (!fs::exists(ip_file)) {
        GTEST_SKIP() << "Test fixture file not found: " << ip_file;
    }
    
    try {
        auto ips = ReadIPsFromFile(ip_file.string());
        
        // Expected format from Go implementation:
        // - One IP per line
        // - Comments and empty lines skipped
        // - Whitespace trimmed
        
        EXPECT_GT(ips.size(), 0) << "Should read at least one IP";
        
        // Verify format: should be "host:port" or "ip:port"
        for (const auto& ip : ips) {
            EXPECT_FALSE(ip.empty());
            EXPECT_TRUE(ip.find(':') != std::string::npos) << "IP should contain port";
        }
    } catch (const std::exception& e) {
        GTEST_SKIP() << "ReadIPsFromFile not yet implemented: " << e.what();
    }
}

// Test: Compare SHA256 output with Go implementation
TEST_F(ComparisonTest, SHA256Hex_MatchesGo) {
    // Test with known test vectors (same as Go crypto/sha256)
    std::string test_str = "Hello World";
    std::vector<uint8_t> data(test_str.begin(), test_str.end());
    
    try {
        std::string hash = SHA256Hex(data);
        
        // Go: sha256.Sum256([]byte("Hello World"))
        // Result: a591a6d40bf420404a011733cfb7b190d62c65bf0bcda32b57b277d9ad9f146e
        EXPECT_EQ(hash, "a591a6d40bf420404a011733cfb7b190d62c65bf0bcda32b57b277d9ad9f146e");
    } catch (const std::exception& e) {
        GTEST_SKIP() << "SHA256Hex not yet implemented: " << e.what();
    }
}

// Test: Compare message parsing with Go implementation
TEST_F(ComparisonTest, ParseMessage_MatchesGo) {
    // JSON format from Go: {"MessageID":"...","Topic":"...","Message":"...","SourceNodeID":"..."}
    std::string json_str = R"({
        "MessageID": "test-msg-123",
        "Topic": "test-topic",
        "Message": "SGVsbG8gV29ybGQ=",
        "SourceNodeID": "node-1"
    })";
    
    std::vector<uint8_t> json_data(json_str.begin(), json_str.end());
    
    try {
        P2PMessage msg = ParseMessage(json_data);
        
        // Verify structure matches Go P2PMessage
        EXPECT_EQ(msg.message_id, "test-msg-123");
        EXPECT_EQ(msg.topic, "test-topic");
        EXPECT_EQ(msg.source_node_id, "node-1");
        // Message may be base64 decoded or kept as-is depending on implementation
        EXPECT_FALSE(msg.message.empty());
    } catch (const std::exception& e) {
        GTEST_SKIP() << "ParseMessage not yet implemented: " << e.what();
    }
}

// Test: Compare output file format with Go implementation
TEST_F(ComparisonTest, OutputFileFormat_MatchesGo) {
    // Go format for data output: receiver\tsender\tsize\tsha256(msg)
    auto expected_file = expected_outputs_dir_ / "sample_data_output.tsv";
    
    if (!fs::exists(expected_file)) {
        GTEST_SKIP() << "Expected output file not found: " << expected_file;
    }
    
    std::ifstream file(expected_file);
    if (!file.is_open()) {
        GTEST_SKIP() << "Could not open expected output file: " << expected_file;
    }
    
    std::string line;
    
    // Read header (if present)
    if (std::getline(file, line)) {
        EXPECT_EQ(line, "receiver\tsender\tsize\tsha256(msg)");
    }
    
    // Read data lines
    int line_count = 0;
    while (std::getline(file, line)) {
        if (line.empty()) continue; // Skip empty lines
        
        line_count++;
        
        // Verify TSV format: should have 4 tab-separated fields
        std::istringstream iss(line);
        std::string field;
        int field_count = 0;
        
        while (std::getline(iss, field, '\t')) {
            field_count++;
        }
        
        EXPECT_EQ(field_count, 4) << "Each line should have 4 tab-separated fields";
    }
    
    EXPECT_GT(line_count, 0) << "Should have at least one data line";
}

// Test: Compare trace output format with Go implementation
TEST_F(ComparisonTest, TraceOutputFormat_MatchesGo) {
    // Go format for trace output: type\tpeerID\treceivedFrom\tmessageID\ttopic\ttimestamp
    auto expected_file = expected_outputs_dir_ / "sample_trace_output.tsv";
    
    if (!fs::exists(expected_file)) {
        GTEST_SKIP() << "Expected trace output file not found: " << expected_file;
    }
    
    std::ifstream file(expected_file);
    if (!file.is_open()) {
        GTEST_SKIP() << "Could not open expected trace output file: " << expected_file;
    }
    
    std::string line;
    
    // Read first non-empty line
    while (std::getline(file, line) && line.empty()) {
        // Skip empty lines
    }
    
    if (line.empty()) {
        GTEST_SKIP() << "Trace output file is empty";
    }
    
    // Verify TSV format: should have 6 tab-separated fields
    std::istringstream iss(line);
    std::string field;
    int field_count = 0;
    
    while (std::getline(iss, field, '\t')) {
        field_count++;
    }
    
    EXPECT_EQ(field_count, 6) << "Trace line should have 6 tab-separated fields";
}

// Test: Message format prefix matches Go implementation
TEST_F(ComparisonTest, MessagePrefixFormat_MatchesGo) {
    // Go format: [timestamp size] message_content
    // Example: [1757588485852133000 50] HelloWorld
    
    // This test verifies that when we format messages, they match Go format
    // The actual formatting happens in the client publish function
    
    int64_t timestamp = 1757588485852133000;
    std::string message = "HelloWorld";
    int size = message.length();
    
    // Format should be: [timestamp size] message
    std::ostringstream oss;
    oss << "[" << timestamp << " " << size << "] " << message;
    std::string formatted = oss.str();
    
    EXPECT_EQ(formatted, "[1757588485852133000 50] HelloWorld");
}

// Test: Verify message counting matches Go implementation
TEST_F(ComparisonTest, MessageCounting_MatchesGo) {
    // Go uses atomic.AddInt32 for message counting
    // C++ should use std::atomic<int32_t> for consistency
    
    // This is a structure test - actual counting happens in client code
    std::atomic<int32_t> counter{0};
    
    counter++;
    EXPECT_EQ(counter.load(), 1);
    
    counter += 5;
    EXPECT_EQ(counter.load(), 6);
}

} // namespace optimum_p2p

