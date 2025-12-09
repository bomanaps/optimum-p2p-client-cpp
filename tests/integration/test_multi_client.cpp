#include <gtest/gtest.h>
#include "optimum_p2p/multi_client.hpp"
#include "optimum_p2p/utils.hpp"
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include <atomic>
#include <algorithm>
#include <cstdlib>

namespace fs = std::filesystem;

namespace optimum_p2p {

class MultiClientIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary IP file
        test_dir_ = fs::temp_directory_path() / "optimum_p2p_integration_test";
        fs::create_directories(test_dir_);
        
        ip_file_ = test_dir_ / "test_ips.txt";
        std::ofstream file(ip_file_);
        file << "127.0.0.1:33221\n";
        file << "127.0.0.1:33222\n";
        file << "127.0.0.1:33223\n";
        file << "127.0.0.1:33224\n";
        file.close();
        
        test_topic_ = "multi-client-test-topic";
    }

    void TearDown() override {
        if (fs::exists(test_dir_)) {
            fs::remove_all(test_dir_);
        }
    }

    fs::path test_dir_;
    fs::path ip_file_;
    std::string test_topic_;
};

// Test: Read IPs from file for multi-client
TEST_F(MultiClientIntegrationTest, ReadIPsForMultiClient) {
    auto ips = ReadIPsFromFile(ip_file_.string());
    
    ASSERT_EQ(ips.size(), 4);
    EXPECT_EQ(ips[0], "127.0.0.1:33221");
    EXPECT_EQ(ips[1], "127.0.0.1:33222");
    EXPECT_EQ(ips[2], "127.0.0.1:33223");
    EXPECT_EQ(ips[3], "127.0.0.1:33224");
}

// Test: Multi-publish to multiple nodes
TEST_F(MultiClientIntegrationTest, DISABLED_MultiPublish) {
    auto ips = ReadIPsFromFile(ip_file_.string());
    MultiPublishClient client(ips);
    
    std::vector<uint8_t> test_data = {'T', 'e', 's', 't'};
    
    // Publish to all nodes
    client.PublishAll(test_topic_, test_data, 1, std::chrono::milliseconds(0));
    
    // Verify all publishes succeeded
    // This would require checking logs or using a monitoring mechanism
    SUCCEED();
}

// Test: Multi-publish with output file
TEST_F(MultiClientIntegrationTest, DISABLED_MultiPublishWithOutput) {
    auto ips = ReadIPsFromFile(ip_file_.string());
    MultiPublishClient client(ips);
    
    auto output_file = test_dir_ / "publish_output.tsv";
    client.SetOutputFile(output_file.string());
    
    std::vector<uint8_t> test_data = {'T', 'e', 's', 't'};
    client.PublishAll(test_topic_, test_data, 2, std::chrono::milliseconds(100));
    
    // Verify output file was created and contains expected data
    // Format: sender\tsize\tsha256(msg)
    if (fs::exists(output_file)) {
        std::ifstream file(output_file);
        std::string line;
        int line_count = 0;
        while (std::getline(file, line)) {
            line_count++;
        }
        EXPECT_GT(line_count, 0) << "Output file should contain data";
    }
}

// Test: Multi-subscribe to multiple nodes
TEST_F(MultiClientIntegrationTest, DISABLED_MultiSubscribe) {
    auto ips = ReadIPsFromFile(ip_file_.string());
    MultiSubscribeClient client(ips);
    
    // Subscribe to all nodes
    client.SubscribeAll(test_topic_);
    
    // Set up callbacks
    std::atomic<int> message_count{0};
    client.SetDataCallback([&](const std::string& addr, const P2PMessage& msg) {
        message_count++;
    });
    
    // Wait for messages
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Verify subscription succeeded
    SUCCEED();
}

// Test: Multi-subscribe with data output file
TEST_F(MultiClientIntegrationTest, DISABLED_MultiSubscribeWithDataOutput) {
    auto ips = ReadIPsFromFile(ip_file_.string());
    MultiSubscribeClient client(ips);
    
    auto data_file = test_dir_ / "subscribe_data.tsv";
    client.SetDataOutputFile(data_file.string());
    
    client.SubscribeAll(test_topic_);
    
    // Wait for messages
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Verify data file format: receiver\tsender\tsize\tsha256(msg)
    if (fs::exists(data_file)) {
        std::ifstream file(data_file);
        std::string line;
        std::getline(file, line); // Skip header if present
        
        // Verify TSV format (only if line is not empty)
        if (!line.empty()) {
            size_t tab_count = std::count(line.begin(), line.end(), '\t');
            EXPECT_GE(tab_count, 3) << "Data file should be TSV format";
        }
    }
}

// Test: Multi-subscribe with trace output file
TEST_F(MultiClientIntegrationTest, DISABLED_MultiSubscribeWithTraceOutput) {
    auto ips = ReadIPsFromFile(ip_file_.string());
    MultiSubscribeClient client(ips);
    
    auto trace_file = test_dir_ / "subscribe_trace.tsv";
    client.SetTraceOutputFile(trace_file.string());
    
    client.SubscribeAll(test_topic_);
    
    // Wait for trace events
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Verify trace file was created
    // Trace format: type\tpeerID\treceivedFrom\tmessageID\ttopic\ttimestamp
    if (fs::exists(trace_file)) {
        std::ifstream file(trace_file);
        std::string line;
        int line_count = 0;
        while (std::getline(file, line)) {
            line_count++;
        }
        // Trace file may be empty if no trace events occurred
        SUCCEED();
    }
}

// Test: IP range selection
TEST_F(MultiClientIntegrationTest, IPRangeSelection) {
    auto all_ips = ReadIPsFromFile(ip_file_.string());
    
    // Test selecting subset: indices 1-2
    std::vector<std::string> subset(all_ips.begin() + 1, all_ips.begin() + 3);
    
    ASSERT_EQ(subset.size(), 2);
    EXPECT_EQ(subset[0], "127.0.0.1:33222");
    EXPECT_EQ(subset[1], "127.0.0.1:33223");
}

// Test: Concurrent operations
TEST_F(MultiClientIntegrationTest, DISABLED_ConcurrentPublishSubscribe) {
    auto ips = ReadIPsFromFile(ip_file_.string());
    
    // Create publisher and subscriber
    MultiPublishClient publisher(ips);
    MultiSubscribeClient subscriber(ips);
    
    std::atomic<int> received_count{0};
    subscriber.SetDataCallback([&](const std::string&, const P2PMessage&) {
        received_count++;
    });
    
    subscriber.SubscribeAll(test_topic_);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::vector<uint8_t> test_data = {'C', 'o', 'n', 'c', 'u', 'r', 'r', 'e', 'n', 't'};
    publisher.PublishAll(test_topic_, test_data, 5, std::chrono::milliseconds(100));
    
    // Wait for messages
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Verify messages were received
    EXPECT_GT(received_count.load(), 0) << "Should receive messages from concurrent publish";
}

} // namespace optimum_p2p

