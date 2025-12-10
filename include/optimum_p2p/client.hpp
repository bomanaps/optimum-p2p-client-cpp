#pragma once

#include "types.hpp"
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <memory>
#include <thread>
#include <atomic>

// Include protobuf and gRPC headers for Phase 1 (will optimize in Phase 2)
#include "p2p_stream.grpc.pb.h"
#include <grpcpp/grpcpp.h>

namespace optimum_p2p {

class P2PClient {
public:
    explicit P2PClient(const std::string& address);
    ~P2PClient();
    
    // Subscribe to topic
    bool Subscribe(const std::string& topic);
    
    // Publish message
    bool Publish(const std::string& topic, const std::vector<uint8_t>& data);
    
    // Receive messages (blocking)
    bool ReceiveMessage(P2PMessage& message, std::chrono::milliseconds timeout);
    
    // Non-blocking message reception via callback
    void SetMessageCallback(std::function<void(const P2PMessage&)> callback);
    
    // Graceful shutdown
    void Shutdown();

private:
    void ReceiveLoop(); // Internal receive loop running in separate thread
    
    std::unique_ptr<proto::CommandStream::Stub> stub_;
    std::shared_ptr<grpc::Channel> channel_;
    std::unique_ptr<grpc::ClientContext> context_;
    std::unique_ptr<grpc::ClientReaderWriter<proto::Request, proto::Response>> stream_;
    std::thread receive_thread_;
    std::atomic<bool> running_;
    std::function<void(const P2PMessage&)> message_callback_;
};

} // namespace optimum_p2p

