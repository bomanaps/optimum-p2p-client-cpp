// Proxy Client implementation

#include "optimum_p2p/proxy_client.hpp"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <random>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <climits>

namespace optimum_p2p {

// CURL write callback for response data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    size_t total_size = size * nmemb;
    data->append((char*)contents, total_size);
    return total_size;
}

ProxyClient::ProxyClient(const std::string& rest_url, const std::string& grpc_address)
    : rest_url_(rest_url), grpc_address_(grpc_address) {
    // Initialize CURL (thread-safe in modern versions)
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

ProxyClient::~ProxyClient() {
    // Clean up stream if active
    if (stream_) {
        stream_->WritesDone();
        stream_->Finish();
        stream_.reset();
    }
    
    // Clean up context, stub and channel
    context_.reset();
    stub_.reset();
    channel_.reset();
    
    curl_global_cleanup();
}

bool ProxyClient::Subscribe(const std::string& client_id, 
                           const std::string& topic, 
                           double threshold) {
    nlohmann::json payload;
    payload["client_id"] = client_id;
    payload["topic"] = topic;
    payload["threshold"] = threshold;
    
    std::string json_str = payload.dump();
    std::string endpoint = rest_url_ + "/api/v1/subscribe";
    
    return PostJSON(endpoint, json_str);
}

bool ProxyClient::Publish(const std::string& client_id,
                        const std::string& topic,
                        const std::string& message) {
    nlohmann::json payload;
    payload["client_id"] = client_id;
    payload["topic"] = topic;
    payload["message"] = message;
    
    std::string json_str = payload.dump();
    std::string endpoint = rest_url_ + "/api/v1/publish";
    
    return PostJSON(endpoint, json_str);
}

bool ProxyClient::ConnectStream(const std::string& client_id) {
    // Create channel arguments with max message sizes
    grpc::ChannelArguments args;
    args.SetMaxReceiveMessageSize(INT_MAX);
    args.SetMaxSendMessageSize(INT_MAX);
    // Set large window sizes for high throughput (1GB)
    args.SetInt("grpc.initial_receive_window_size", 1024 * 1024 * 1024);
    args.SetInt("grpc.initial_connection_window_size", 1024 * 1024 * 1024);
    
    // Create insecure channel
    channel_ = grpc::CreateCustomChannel(grpc_address_, grpc::InsecureChannelCredentials(), args);
    
    if (!channel_) {
        return false;
    }
    
    // Create stub
    stub_ = proto::ProxyStream::NewStub(channel_);
    
    if (!stub_) {
        return false;
    }
    
    // Create bidirectional stream
    context_ = std::make_unique<grpc::ClientContext>();
    stream_ = stub_->ClientStream(context_.get());
    
    if (!stream_) {
        return false;
    }
    
    // Send client ID
    proto::ProxyMessage msg;
    msg.set_client_id(client_id);
    
    return stream_->Write(msg);
}

bool ProxyClient::ReceiveMessage(std::string& topic, std::string& message, int timeout_ms) {
    if (!stream_) {
        return false;
    }
    
    proto::ProxyMessage msg;
    
    // For timeout, we'd need async reading, but for simplicity, just try to read
    // In a real implementation, you'd use async API or a separate thread
    if (stream_->Read(&msg)) {
        topic = msg.topic();
        message.assign(msg.message().begin(), msg.message().end());
        return true;
    }
    
    return false;
}

std::string ProxyClient::GenerateClientID() {
    // Generate random client ID like Go implementation: "client_" + hex(4 bytes)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    std::vector<uint8_t> random_bytes(4);
    for (auto& b : random_bytes) {
        b = static_cast<uint8_t>(dis(gen));
    }
    
    std::ostringstream oss;
    oss << "client_";
    for (auto b : random_bytes) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    }
    
    return oss.str();
}

bool ProxyClient::PostJSON(const std::string& endpoint, const std::string& json_data) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return false;
    }
    
    std::string response_data;
    
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    
    CURLcode res = curl_easy_perform(curl);
    
    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return (res == CURLE_OK && response_code >= 200 && response_code < 300);
}

} // namespace optimum_p2p

