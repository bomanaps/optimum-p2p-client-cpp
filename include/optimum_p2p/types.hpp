#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace optimum_p2p {

// Command represents possible operations that sidecar may perform with p2p node
enum class Command : int32_t {
    Unknown = 0,
    PublishData = 1,
    SubscribeToTopic = 2,
    UnSubscribeToTopic = 3
};

// ResponseType defines the type of response returned by the node
enum class ResponseType : int32_t {
    Unknown = 0,
    Message = 1,
    MessageTraceMumP2P = 2,
    MessageTraceGossipSub = 3
};

// P2PMessage represents a message structure used in P2P communication
struct P2PMessage {
    std::string message_id;
    std::string topic;
    std::vector<uint8_t> message;
    std::string source_node_id;
};

} // namespace optimum_p2p

