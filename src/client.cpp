// P2P Client implementation

#include "optimum_p2p/client.hpp"
#include "optimum_p2p/utils.hpp"
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <chrono>
#include <mutex>
#include <queue>
#include <climits>
#include <thread>

namespace optimum_p2p {

P2PClient::P2PClient(const std::string& address) 
    : running_(true) {
    // Create channel arguments with max message sizes (like Go's MaxCallRecvMsgSize/MaxCallSendMsgSize)
    grpc::ChannelArguments args;
    args.SetMaxReceiveMessageSize(INT_MAX);
    args.SetMaxSendMessageSize(INT_MAX);
    
    // Create insecure channel (like Go's insecure.NewCredentials())
    channel_ = grpc::CreateCustomChannel(address, grpc::InsecureChannelCredentials(), args);
    
    if (!channel_) {
        running_ = false;
        return;
    }
    
    // Create stub
    stub_ = proto::CommandStream::NewStub(channel_);
    
    if (!stub_) {
        running_ = false;
        return;
    }
    
    // Create bidirectional stream
    context_ = std::make_unique<grpc::ClientContext>();
    
    stream_ = stub_->ListenCommands(context_.get());
    
    if (!stream_) {
        running_ = false;
        return;
    }
    
    // Start receive thread
    receive_thread_ = std::thread([this]() {
        this->ReceiveLoop();
    });
}

P2PClient::~P2PClient() {
    Shutdown();
}

bool P2PClient::Subscribe(const std::string& topic) {
    if (!stream_ || !running_) {
        return false;
    }
    
    proto::Request request;
    request.set_command(static_cast<int32_t>(Command::SubscribeToTopic));
    request.set_topic(topic);
    
    return stream_->Write(request);
}

bool P2PClient::Publish(const std::string& topic, const std::vector<uint8_t>& data) {
    if (!stream_ || !running_) {
        return false;
    }
    
    proto::Request request;
    request.set_command(static_cast<int32_t>(Command::PublishData));
    request.set_topic(topic);
    request.set_data(data.data(), data.size());
    
    return stream_->Write(request);
}

bool P2PClient::ReceiveMessage(P2PMessage& message, std::chrono::milliseconds timeout) {
    // This is a blocking receive - for non-blocking, use SetMessageCallback
    if (!stream_ || !running_) {
        return false;
    }
    
    proto::Response response;
    if (!stream_->Read(&response)) {
        return false;
    }
    
    // Parse response based on type
    if (response.command() == proto::ResponseType::Message) {
        std::vector<uint8_t> json_data(response.data().begin(), response.data().end());
        message = ParseMessage(json_data);
        return true;
    }
    
    return false;
}

void P2PClient::SetMessageCallback(std::function<void(const P2PMessage&)> callback) {
    message_callback_ = callback;
}

void P2PClient::Shutdown() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    // Close write side of stream
    if (stream_) {
        stream_->WritesDone();
    }
    
    // Wait for receive thread
    if (receive_thread_.joinable()) {
        receive_thread_.join();
    }
    
    // Clean up stream
    if (stream_) {
        grpc::Status status = stream_->Finish();
        stream_.reset();
    }
    
    // Clean up context, stub and channel
    context_.reset();
    stub_.reset();
    channel_.reset();
}

void P2PClient::ReceiveLoop() {
    proto::Response response;
    
    while (running_) {
        if (!stream_->Read(&response)) {
            // Stream closed or error
            break;
        }
        
        // Handle different response types
        if (response.command() == proto::ResponseType::Message) {
            std::vector<uint8_t> json_data(response.data().begin(), response.data().end());
            P2PMessage msg = ParseMessage(json_data);
            
            // Call callback if set
            if (message_callback_) {
                message_callback_(msg);
            }
        } else if (response.command() == proto::ResponseType::MessageTraceGossipSub) {
            std::vector<uint8_t> trace_data(response.data().begin(), response.data().end());
            HandleGossipSubTrace(trace_data, false, nullptr);
        } else if (response.command() == proto::ResponseType::MessageTraceMumP2P) {
            std::vector<uint8_t> trace_data(response.data().begin(), response.data().end());
            HandleOptimumP2PTrace(trace_data, false, nullptr);
        }
    }
}

} // namespace optimum_p2p

