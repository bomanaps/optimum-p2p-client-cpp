// Multi-Node Client implementation

#include "optimum_p2p/multi_client.hpp"
#include "optimum_p2p/utils.hpp"
#include <thread>
#include <mutex>
#include <fstream>
#include <sstream>
#include <chrono>
#include <random>
#include <iomanip>

namespace optimum_p2p {

// MultiPublishClient implementation

MultiPublishClient::MultiPublishClient(const std::vector<std::string>& addresses)
    : addresses_(addresses) {
}

MultiPublishClient::~MultiPublishClient() {
    // Cleanup handled by member destructors
}

void MultiPublishClient::PublishAll(const std::string& topic, 
                                   const std::vector<uint8_t>& data,
                                   int count,
                                   std::chrono::milliseconds delay) {
    std::vector<std::thread> threads;
    
    for (const auto& address : addresses_) {
        threads.emplace_back([this, address, topic, data, count, delay]() {
            this->PublishToNode(address, topic, data, count, delay);
        });
    }
    
    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }
}

void MultiPublishClient::PublishToNode(const std::string& address,
                                      const std::string& topic,
                                      const std::vector<uint8_t>& data,
                                      int count,
                                      std::chrono::milliseconds delay) {
    P2PClient client(address);
    
    for (int i = 0; i < count; i++) {
        std::vector<uint8_t> message_data;
        
        if (count == 1) {
            // Single message: add timestamp prefix like Go implementation
            auto now = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            std::string prefix = "[" + std::to_string(now) + " " + std::to_string(data.size()) + "] ";
            message_data.assign(prefix.begin(), prefix.end());
            message_data.insert(message_data.end(), data.begin(), data.end());
        } else {
            // Multiple messages: add random suffix like Go implementation
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 255);
            
            std::vector<uint8_t> random_bytes(4);
            for (auto& b : random_bytes) {
                b = static_cast<uint8_t>(dis(gen));
            }
            
            std::string hex_suffix = HeadHex(random_bytes, 4);
            auto now = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            std::string msg = "[" + std::to_string(now) + " " + std::to_string(hex_suffix.length()) + 
                            "] " + std::to_string(i + 1) + " - " + hex_suffix + " XXX";
            message_data.assign(msg.begin(), msg.end());
        }
        
        if (client.Publish(topic, message_data)) {
            // Write to output file if set
            if (!output_file_.empty()) {
                std::lock_guard<std::mutex> lock(output_mutex_);
                std::ofstream file(output_file_, std::ios::app);
                if (file.is_open()) {
                    std::string hash = SHA256Hex(message_data);
                    file << address << "\t" << message_data.size() << "\t" << hash << "\n";
                    file.flush();
                }
            }
        }
        
        if (delay.count() > 0 && i < count - 1) {
            std::this_thread::sleep_for(delay);
        }
    }
    
    client.Shutdown();
}

void MultiPublishClient::SetOutputFile(const std::string& filename) {
    output_file_ = filename;
}

// MultiSubscribeClient implementation

MultiSubscribeClient::MultiSubscribeClient(const std::vector<std::string>& addresses)
    : addresses_(addresses) {
}

MultiSubscribeClient::~MultiSubscribeClient() {
    // Stop all clients
    for (auto& client : clients_) {
        if (client) {
            client->Shutdown();
        }
    }
}

void MultiSubscribeClient::SubscribeAll(const std::string& topic) {
    // Create clients for each address
    clients_.clear();
    for (const auto& address : addresses_) {
        auto client = std::make_unique<P2PClient>(address);
        if (client->Subscribe(topic)) {
            // Set up message callback
            client->SetMessageCallback([this, address](const P2PMessage& msg) {
                this->HandleMessage(address, msg);
            });
            clients_.push_back(std::move(client));
        }
    }
}

void MultiSubscribeClient::HandleMessage(const std::string& address, const P2PMessage& msg) {
    // Call data callback if set
    if (data_callback_) {
        data_callback_(address, msg);
    }
    
    // Write to data output file if set
    if (!data_output_file_.empty()) {
        std::lock_guard<std::mutex> lock(file_mutex_);
        std::ofstream file(data_output_file_, std::ios::app);
        if (file.is_open()) {
            std::string hash = SHA256Hex(msg.message);
            file << address << "\t" << msg.source_node_id << "\t" 
                 << msg.message.size() << "\t" << hash << "\n";
            file.flush();
        }
    }
}

void MultiSubscribeClient::SetDataCallback(std::function<void(const std::string&, const P2PMessage&)> callback) {
    data_callback_ = callback;
}

void MultiSubscribeClient::SetTraceCallback(std::function<void(const std::string&)> callback) {
    trace_callback_ = callback;
}

void MultiSubscribeClient::SetDataOutputFile(const std::string& filename) {
    data_output_file_ = filename;
}

void MultiSubscribeClient::SetTraceOutputFile(const std::string& filename) {
    trace_output_file_ = filename;
}

} // namespace optimum_p2p

