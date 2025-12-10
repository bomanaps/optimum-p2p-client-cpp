#pragma once

#include "client.hpp"
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <memory>
#include <mutex>

namespace optimum_p2p {

class MultiPublishClient {
public:
    explicit MultiPublishClient(const std::vector<std::string>& addresses);
    ~MultiPublishClient();
    
    // Publish to all nodes concurrently
    void PublishAll(const std::string& topic, 
                   const std::vector<uint8_t>& data,
                   int count = 1,
                   std::chrono::milliseconds delay = std::chrono::milliseconds(0));
    
    // Set output file for logging
    void SetOutputFile(const std::string& filename);
    
private:
    void PublishToNode(const std::string& address,
                      const std::string& topic,
                      const std::vector<uint8_t>& data,
                      int count,
                      std::chrono::milliseconds delay);
    
    std::vector<std::string> addresses_;
    std::string output_file_;
    std::mutex output_mutex_;
};

class MultiSubscribeClient {
public:
    explicit MultiSubscribeClient(const std::vector<std::string>& addresses);
    ~MultiSubscribeClient();
    
    // Subscribe to all nodes concurrently
    void SubscribeAll(const std::string& topic);
    
    // Set callbacks for data and trace output
    void SetDataCallback(std::function<void(const std::string&, const P2PMessage&)> callback);
    void SetTraceCallback(std::function<void(const std::string&)> callback);
    
    // Set output files
    void SetDataOutputFile(const std::string& filename);
    void SetTraceOutputFile(const std::string& filename);
    
private:
    void HandleMessage(const std::string& address, const P2PMessage& msg);
    
    std::vector<std::unique_ptr<P2PClient>> clients_;
    std::vector<std::string> addresses_;
    std::function<void(const std::string&, const P2PMessage&)> data_callback_;
    std::function<void(const std::string&)> trace_callback_;
    std::string data_output_file_;
    std::string trace_output_file_;
    std::mutex file_mutex_;
};

} // namespace optimum_p2p

