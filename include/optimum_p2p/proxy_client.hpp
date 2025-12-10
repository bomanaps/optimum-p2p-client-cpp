#pragma once

#include "types.hpp"
#include <string>
#include <memory>

// Include protobuf and gRPC headers for Phase 1 (will optimize in Phase 2)
#include "proxy_stream.grpc.pb.h"
#include <grpcpp/grpcpp.h>

namespace optimum_p2p {

class ProxyClient {
public:
    ProxyClient(const std::string& rest_url, const std::string& grpc_address);
    ~ProxyClient();
    
    // Subscribe via REST API
    bool Subscribe(const std::string& client_id, 
                  const std::string& topic, 
                  double threshold = 0.1);
    
    // Publish via REST API
    bool Publish(const std::string& client_id,
                const std::string& topic,
                const std::string& message);
    
    // Connect gRPC stream
    bool ConnectStream(const std::string& client_id);
    
    // Receive messages from stream
    bool ReceiveMessage(std::string& topic, std::string& message, int timeout_ms = 1000);
    
    // Generate client ID
    static std::string GenerateClientID();
    
private:
    std::string rest_url_;
    std::string grpc_address_;
    std::unique_ptr<proto::ProxyStream::Stub> stub_;
    std::shared_ptr<grpc::Channel> channel_;
    std::unique_ptr<grpc::ClientContext> context_;
    std::unique_ptr<grpc::ClientReaderWriter<proto::ProxyMessage, proto::ProxyMessage>> stream_;
    
    // REST API helpers
    bool PostJSON(const std::string& endpoint, const std::string& json_data);
};

} // namespace optimum_p2p

