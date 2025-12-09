#include <gtest/gtest.h>
#include "optimum_p2p/client.hpp"
#include "optimum_p2p/multi_client.hpp"
#include "optimum_p2p/proxy_client.hpp"
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <cstdlib>

namespace optimum_p2p {

class EndToEndTest : public ::testing::Test {
protected:
    void SetUp() override {
        p2p_address_ = std::getenv("P2P_TEST_ADDRESS") ? 
                      std::getenv("P2P_TEST_ADDRESS") : 
                      "127.0.0.1:33221";
        rest_url_ = std::getenv("PROXY_REST_URL") ? 
                   std::getenv("PROXY_REST_URL") : 
                   "http://localhost:8081";
        grpc_address_ = std::getenv("PROXY_GRPC_ADDRESS") ? 
                       std::getenv("PROXY_GRPC_ADDRESS") : 
                       "localhost:50051";
        test_topic_ = "e2e-test-topic";
    }

    std::string p2p_address_;
    std::string rest_url_;
    std::string grpc_address_;
    std::string test_topic_;
};

// Test: Full P2P workflow - Subscribe on node 1, Publish from node 2
TEST_F(EndToEndTest, DISABLED_FullP2PWorkflow) {
    // This test requires Docker P2P nodes to be running
    // Start nodes: docker-compose -f docker-compose-optimum.yml up -d
    
    // Step 1: Subscribe on node 1
    P2PClient subscriber(p2p_address_);
    ASSERT_TRUE(subscriber.Subscribe(test_topic_));
    
    std::atomic<bool> message_received{false};
    P2PMessage received_msg;
    
    subscriber.SetMessageCallback([&](const P2PMessage& msg) {
        received_msg = msg;
        message_received = true;
    });
    
    // Wait for subscription to be established
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Step 2: Publish from node 2 (or same node)
    P2PClient publisher(p2p_address_);
    std::string test_message = "E2E test message";
    std::vector<uint8_t> message_data(test_message.begin(), test_message.end());
    ASSERT_TRUE(publisher.Publish(test_topic_, message_data));
    
    // Step 3: Verify message received
    auto start = std::chrono::steady_clock::now();
    while (!message_received && 
           (std::chrono::steady_clock::now() - start) < std::chrono::seconds(10)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    EXPECT_TRUE(message_received) << "Message not received within timeout";
    if (message_received) {
        EXPECT_EQ(received_msg.topic, test_topic_);
        std::string received_content(received_msg.message.begin(), received_msg.message.end());
        EXPECT_TRUE(received_content.find(test_message) != std::string::npos);
    }
}

// Test: Multi-node stress test
TEST_F(EndToEndTest, DISABLED_MultiNodeStressTest) {
    // Connect to 4+ nodes
    std::vector<std::string> addresses = {
        "127.0.0.1:33221",
        "127.0.0.1:33222",
        "127.0.0.1:33223",
        "127.0.0.1:33224"
    };
    
    // Create subscribers on all nodes
    std::vector<std::unique_ptr<P2PClient>> subscribers;
    std::atomic<int> total_received{0};
    
    for (const auto& addr : addresses) {
        auto client = std::make_unique<P2PClient>(addr);
        if (client->Subscribe(test_topic_)) {
            client->SetMessageCallback([&](const P2PMessage&) {
                total_received++;
            });
            subscribers.push_back(std::move(client));
        }
    }
    
    // Wait for subscriptions
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // Publish 100+ messages concurrently
    MultiPublishClient publisher(addresses);
    std::vector<uint8_t> test_data = {'S', 't', 'r', 'e', 's', 's'};
    publisher.PublishAll(test_topic_, test_data, 25, std::chrono::milliseconds(10));
    
    // Wait for delivery
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // Verify delivery statistics
    EXPECT_GT(total_received.load(), 0) << "Should receive at least some messages";
    
    // Cleanup
    for (auto& client : subscribers) {
        client->Shutdown();
    }
}

// Test: Proxy workflow - Subscribe via REST, Connect gRPC, Publish via REST
TEST_F(EndToEndTest, DISABLED_ProxyWorkflow) {
    ProxyClient client(rest_url_, grpc_address_);
    
    // Step 1: Subscribe via REST API
    ASSERT_TRUE(client.Subscribe(client.client_id, test_topic_, 0.7));
    
    // Step 2: Connect gRPC stream
    ASSERT_TRUE(client.ConnectStream(client.client_id));
    
    // Step 3: Publish via REST
    std::string test_message = "Proxy E2E test";
    ASSERT_TRUE(client.Publish(client.client_id, test_topic_, test_message));
    
    // Step 4: Verify delivery on stream
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    std::string received_topic, received_message;
    bool received = client.ReceiveMessage(received_topic, received_message, 5000);
    
    EXPECT_TRUE(received) << "Message should be received on gRPC stream";
    if (received) {
        EXPECT_EQ(received_topic, test_topic_);
        EXPECT_EQ(received_message, test_message);
    }
}

// Test: Cross-protocol test - Publish via P2P, Receive via Proxy
TEST_F(EndToEndTest, DISABLED_CrossProtocolTest) {
    // This test verifies interoperability between P2P and Proxy
    
    // Step 1: Subscribe via Proxy
    ProxyClient proxy_client(rest_url_, grpc_address_);
    ASSERT_TRUE(proxy_client.Subscribe(proxy_client.client_id, test_topic_, 0.7));
    ASSERT_TRUE(proxy_client.ConnectStream(proxy_client.client_id));
    
    // Step 2: Publish via P2P client
    P2PClient p2p_client(p2p_address_);
    std::string test_message = "Cross-protocol test";
    std::vector<uint8_t> message_data(test_message.begin(), test_message.end());
    ASSERT_TRUE(p2p_client.Publish(test_topic_, message_data));
    
    // Step 3: Verify message received via Proxy
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    
    std::string received_topic, received_message;
    bool received = proxy_client.ReceiveMessage(received_topic, received_message, 5000);
    
    EXPECT_TRUE(received) << "Message published via P2P should be received via Proxy";
}

// Test: Message format consistency
TEST_F(EndToEndTest, DISABLED_MessageFormatConsistency) {
    // This test compares message format between Go and C++ implementations
    
    P2PClient client(p2p_address_);
    ASSERT_TRUE(client.Subscribe(test_topic_));
    
    std::atomic<bool> received{false};
    P2PMessage msg;
    
    client.SetMessageCallback([&](const P2PMessage& m) {
        msg = m;
        received = true;
    });
    
    // Publish message
    std::string test_msg = "Format test";
    std::vector<uint8_t> data(test_msg.begin(), test_msg.end());
    ASSERT_TRUE(client.Publish(test_topic_, data));
    
    // Wait for message
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    if (received) {
        // Verify message structure matches Go implementation
        EXPECT_FALSE(msg.message_id.empty());
        EXPECT_EQ(msg.topic, test_topic_);
        EXPECT_FALSE(msg.message.empty());
    }
}

} // namespace optimum_p2p

