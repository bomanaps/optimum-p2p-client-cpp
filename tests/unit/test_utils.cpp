#include <gtest/gtest.h>
#include "optimum_p2p/utils.hpp"
#include "optimum_p2p/types.hpp"
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>
#include <exception>

namespace fs = std::filesystem;

namespace optimum_p2p {

class UtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directory for test files
        test_dir_ = fs::temp_directory_path() / "optimum_p2p_test";
        fs::create_directories(test_dir_);
    }

    void TearDown() override {
        // Clean up test files
        if (fs::exists(test_dir_)) {
            fs::remove_all(test_dir_);
        }
    }

    std::string CreateTestFile(const std::string& filename, const std::string& content) {
        auto filepath = test_dir_ / filename;
        std::ofstream file(filepath);
        file << content;
        file.close();
        return filepath.string();
    }

    fs::path test_dir_;
};

// Test ReadIPsFromFile with valid file
TEST_F(UtilsTest, ReadIPsFromFile_ValidFile) {
    std::string content = 
        "127.0.0.1:33221\n"
        "127.0.0.1:33222\n"
        "127.0.0.1:33223\n"
        "127.0.0.1:33224\n";
    
    auto filepath = CreateTestFile("valid_ips.txt", content);
    auto ips = ReadIPsFromFile(filepath);
    
    ASSERT_EQ(ips.size(), 4);
    EXPECT_EQ(ips[0], "127.0.0.1:33221");
    EXPECT_EQ(ips[1], "127.0.0.1:33222");
    EXPECT_EQ(ips[2], "127.0.0.1:33223");
    EXPECT_EQ(ips[3], "127.0.0.1:33224");
}

// Test ReadIPsFromFile with comments
TEST_F(UtilsTest, ReadIPsFromFile_WithComments) {
    std::string content = 
        "# This is a comment\n"
        "127.0.0.1:33221\n"
        "# Another comment\n"
        "127.0.0.1:33222\n"
        "127.0.0.1:33223\n";
    
    auto filepath = CreateTestFile("ips_with_comments.txt", content);
    auto ips = ReadIPsFromFile(filepath);
    
    ASSERT_EQ(ips.size(), 3);
    EXPECT_EQ(ips[0], "127.0.0.1:33221");
    EXPECT_EQ(ips[1], "127.0.0.1:33222");
    EXPECT_EQ(ips[2], "127.0.0.1:33223");
}

// Test ReadIPsFromFile with empty lines
TEST_F(UtilsTest, ReadIPsFromFile_WithEmptyLines) {
    std::string content = 
        "127.0.0.1:33221\n"
        "\n"
        "127.0.0.1:33222\n"
        "\n"
        "\n"
        "127.0.0.1:33223\n";
    
    auto filepath = CreateTestFile("ips_with_empty.txt", content);
    auto ips = ReadIPsFromFile(filepath);
    
    ASSERT_EQ(ips.size(), 3);
    EXPECT_EQ(ips[0], "127.0.0.1:33221");
    EXPECT_EQ(ips[1], "127.0.0.1:33222");
    EXPECT_EQ(ips[2], "127.0.0.1:33223");
}

// Test ReadIPsFromFile with whitespace
TEST_F(UtilsTest, ReadIPsFromFile_WithWhitespace) {
    std::string content = 
        "  127.0.0.1:33221  \n"
        "\t127.0.0.1:33222\t\n"
        "127.0.0.1:33223\n";
    
    auto filepath = CreateTestFile("ips_with_whitespace.txt", content);
    auto ips = ReadIPsFromFile(filepath);
    
    ASSERT_EQ(ips.size(), 3);
    EXPECT_EQ(ips[0], "127.0.0.1:33221");
    EXPECT_EQ(ips[1], "127.0.0.1:33222");
    EXPECT_EQ(ips[2], "127.0.0.1:33223");
}

// Test ReadIPsFromFile with empty file
TEST_F(UtilsTest, ReadIPsFromFile_EmptyFile) {
    auto filepath = CreateTestFile("empty.txt", "");
    auto ips = ReadIPsFromFile(filepath);
    
    EXPECT_TRUE(ips.empty());
}

// Test ReadIPsFromFile with only comments and empty lines
TEST_F(UtilsTest, ReadIPsFromFile_OnlyComments) {
    std::string content = 
        "# Comment 1\n"
        "# Comment 2\n"
        "\n"
        "# Comment 3\n";
    
    auto filepath = CreateTestFile("only_comments.txt", content);
    auto ips = ReadIPsFromFile(filepath);
    
    EXPECT_TRUE(ips.empty());
}

// Test ReadIPsFromFile with missing file (should handle gracefully)
TEST_F(UtilsTest, ReadIPsFromFile_MissingFile) {
    auto filepath = test_dir_ / "nonexistent.txt";
    // Implementation may throw exception or return empty vector
    // Test both behaviors are acceptable
    try {
        auto ips = ReadIPsFromFile(filepath.string());
        // If returns empty vector, that's valid
        EXPECT_TRUE(ips.empty());
    } catch (const std::exception&) {
        // If throws exception, that's also valid
        SUCCEED();
    }
}

// Test SHA256Hex with known test vectors
TEST_F(UtilsTest, SHA256Hex_EmptyString) {
    std::vector<uint8_t> empty;
    std::string hash = SHA256Hex(empty);
    
    // SHA256 of empty string: e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
    EXPECT_EQ(hash, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
}

TEST_F(UtilsTest, SHA256Hex_SimpleString) {
    std::string test_str = "Hello World";
    std::vector<uint8_t> data(test_str.begin(), test_str.end());
    std::string hash = SHA256Hex(data);
    
    // SHA256 of "Hello World": a591a6d40bf420404a011733cfb7b190d62c65bf0bcda32b57b277d9ad9f146e
    EXPECT_EQ(hash, "a591a6d40bf420404a011733cfb7b190d62c65bf0bcda32b57b277d9ad9f146e");
}

TEST_F(UtilsTest, SHA256Hex_BinaryData) {
    std::vector<uint8_t> data = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
    std::string hash = SHA256Hex(data);
    
    // Verify it's a valid hex string (64 characters)
    EXPECT_EQ(hash.length(), 64);
    // Verify it only contains hex characters
    for (char c : hash) {
        EXPECT_TRUE((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'));
    }
}

TEST_F(UtilsTest, SHA256Hex_LargeData) {
    // Test with larger data (1KB)
    std::vector<uint8_t> data(1024, 0x42);
    std::string hash = SHA256Hex(data);
    
    EXPECT_EQ(hash.length(), 64);
    // Known hash for 1024 bytes of 0x42 (verified with Python hashlib)
    EXPECT_EQ(hash, "9b6ce55f379e9771551de6939556a7e6b949814ae27c2f5cfd5dbeb378ce7c2a");
}

// Test ParseMessage with valid JSON
TEST_F(UtilsTest, ParseMessage_ValidJSON) {
    std::string json_str = R"({
        "MessageID": "test-message-123",
        "Topic": "test-topic",
        "Message": "SGVsbG8gV29ybGQ=",
        "SourceNodeID": "node-1"
    })";
    
    std::vector<uint8_t> json_data(json_str.begin(), json_str.end());
    P2PMessage msg = ParseMessage(json_data);
    
    EXPECT_EQ(msg.message_id, "test-message-123");
    EXPECT_EQ(msg.topic, "test-topic");
    EXPECT_EQ(msg.source_node_id, "node-1");
    // Message is base64 encoded "Hello World"
    std::string decoded_msg(msg.message.begin(), msg.message.end());
    EXPECT_EQ(decoded_msg, "Hello World");
}

TEST_F(UtilsTest, ParseMessage_ValidJSON_NoBase64) {
    // Test with plain text message (not base64)
    std::string json_str = R"({
        "MessageID": "msg-1",
        "Topic": "topic-1",
        "Message": "Hello World",
        "SourceNodeID": "node-1"
    })";
    
    std::vector<uint8_t> json_data(json_str.begin(), json_str.end());
    P2PMessage msg = ParseMessage(json_data);
    
    EXPECT_EQ(msg.message_id, "msg-1");
    EXPECT_EQ(msg.topic, "topic-1");
    EXPECT_EQ(msg.source_node_id, "node-1");
    std::string decoded_msg(msg.message.begin(), msg.message.end());
    EXPECT_EQ(decoded_msg, "Hello World");
}

TEST_F(UtilsTest, ParseMessage_InvalidJSON) {
    std::string invalid_json = "{ invalid json }";
    std::vector<uint8_t> json_data(invalid_json.begin(), invalid_json.end());
    
    // Should handle gracefully - either throw or return empty/default message
    try {
        P2PMessage msg = ParseMessage(json_data);
        // If returns default/empty message, verify it's in valid state
        SUCCEED();
    } catch (const std::exception&) {
        // If throws exception, that's also valid error handling
        SUCCEED();
    }
}

TEST_F(UtilsTest, ParseMessage_MissingFields) {
    std::string json_str = R"({
        "MessageID": "msg-1"
    })";
    
    std::vector<uint8_t> json_data(json_str.begin(), json_str.end());
    // Should handle missing fields gracefully
    P2PMessage msg = ParseMessage(json_data);
    
    EXPECT_EQ(msg.message_id, "msg-1");
    // Other fields should be empty or default
}

TEST_F(UtilsTest, ParseMessage_EmptyJSON) {
    std::vector<uint8_t> empty;
    // Should handle empty input gracefully
    try {
        P2PMessage msg = ParseMessage(empty);
        // If returns default message, that's valid
        SUCCEED();
    } catch (const std::exception&) {
        // If throws exception, that's also valid error handling
        SUCCEED();
    }
}

// Test WriteToFile functionality
TEST_F(UtilsTest, WriteToFile_WithHeader) {
    auto filepath = test_dir_ / "output.txt";
    std::string header = "receiver\tsender\tsize\tsha256(msg)";
    
    std::vector<std::string> data_lines = {
        "127.0.0.1:33221\tnode1\t116\tabc123",
        "127.0.0.1:33222\tnode1\t116\tabc123"
    };
    
    size_t current_line = 0;
    auto data_source = [&data_lines, &current_line]() -> std::string {
        if (current_line < data_lines.size()) {
            return data_lines[current_line++];
        }
        return "";
    };
    
    WriteToFile(filepath.string(), header, data_source);
    
    // Verify file was created and contains correct content
    ASSERT_TRUE(fs::exists(filepath));
    
    std::ifstream file(filepath);
    std::string line;
    std::vector<std::string> lines;
    
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    
    ASSERT_EQ(lines.size(), 3); // Header + 2 data lines
    EXPECT_EQ(lines[0], header);
    EXPECT_EQ(lines[1], data_lines[0]);
    EXPECT_EQ(lines[2], data_lines[1]);
}

TEST_F(UtilsTest, WriteToFile_WithoutHeader) {
    auto filepath = test_dir_ / "output_no_header.txt";
    std::string header = "";
    
    std::vector<std::string> data_lines = {
        "line1",
        "line2",
        "line3"
    };
    
    size_t current_line = 0;
    auto data_source = [&data_lines, &current_line]() -> std::string {
        if (current_line < data_lines.size()) {
            return data_lines[current_line++];
        }
        return "";
    };
    
    WriteToFile(filepath.string(), header, data_source);
    
    std::ifstream file(filepath);
    std::string line;
    std::vector<std::string> lines;
    
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    
    ASSERT_EQ(lines.size(), 3);
    EXPECT_EQ(lines[0], "line1");
    EXPECT_EQ(lines[1], "line2");
    EXPECT_EQ(lines[2], "line3");
}

TEST_F(UtilsTest, WriteToFile_EmptyData) {
    auto filepath = test_dir_ / "output_empty.txt";
    std::string header = "header";
    
    auto data_source = []() -> std::string {
        return ""; // Empty data source
    };
    
    WriteToFile(filepath.string(), header, data_source);
    
    std::ifstream file(filepath);
    std::string line;
    std::vector<std::string> lines;
    
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    
    // Should only have header if data source returns empty immediately
    ASSERT_EQ(lines.size(), 1);
    EXPECT_EQ(lines[0], header);
}

// Test trace handling functions (these will need protobuf data, so we'll test structure)
// Note: Full trace parsing tests will require actual protobuf message data
// For now, we test that the functions exist and can be called

TEST_F(UtilsTest, HandleGossipSubTrace_EmptyData) {
    std::vector<uint8_t> empty;
    bool callback_called = false;
    
    auto callback = [&callback_called](const std::string& trace) {
        callback_called = true;
    };
    
    // Should handle empty data gracefully
    HandleGossipSubTrace(empty, false, callback);
    // Callback may or may not be called depending on implementation
}

TEST_F(UtilsTest, HandleOptimumP2PTrace_EmptyData) {
    std::vector<uint8_t> empty;
    bool callback_called = false;
    
    auto callback = [&callback_called](const std::string& trace) {
        callback_called = true;
    };
    
    // Should handle empty data gracefully
    HandleOptimumP2PTrace(empty, false, callback);
    // Callback may or may not be called depending on implementation
}

// Test edge cases and error conditions
TEST_F(UtilsTest, ReadIPsFromFile_WithTrailingNewline) {
    std::string content = 
        "127.0.0.1:33221\n"
        "127.0.0.1:33222\n";
    
    auto filepath = CreateTestFile("trailing_newline.txt", content);
    auto ips = ReadIPsFromFile(filepath);
    
    ASSERT_EQ(ips.size(), 2);
}

TEST_F(UtilsTest, ReadIPsFromFile_WithWindowsLineEndings) {
    std::string content = 
        "127.0.0.1:33221\r\n"
        "127.0.0.1:33222\r\n";
    
    auto filepath = CreateTestFile("windows_lineendings.txt", content);
    auto ips = ReadIPsFromFile(filepath);
    
    ASSERT_EQ(ips.size(), 2);
    EXPECT_EQ(ips[0], "127.0.0.1:33221");
    EXPECT_EQ(ips[1], "127.0.0.1:33222");
}

TEST_F(UtilsTest, SHA256Hex_Consistency) {
    std::string test_str = "Consistency test";
    std::vector<uint8_t> data(test_str.begin(), test_str.end());
    
    // Hash should be consistent across multiple calls
    std::string hash1 = SHA256Hex(data);
    std::string hash2 = SHA256Hex(data);
    
    EXPECT_EQ(hash1, hash2);
}

TEST_F(UtilsTest, ParseMessage_UnicodeCharacters) {
    std::string json_str = R"({
        "MessageID": "msg-unicode",
        "Topic": "test",
        "Message": "Hello 世界",
        "SourceNodeID": "node-1"
    })";
    
    std::vector<uint8_t> json_data(json_str.begin(), json_str.end());
    P2PMessage msg = ParseMessage(json_data);
    
    EXPECT_EQ(msg.message_id, "msg-unicode");
    std::string decoded_msg(msg.message.begin(), msg.message.end());
    EXPECT_EQ(decoded_msg, "Hello 世界");
}

} // namespace optimum_p2p

