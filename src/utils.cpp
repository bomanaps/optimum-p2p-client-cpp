// Utility functions implementation

#include "optimum_p2p/utils.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <openssl/sha.h>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <cctype>

// Base64 decoding helper
std::vector<uint8_t> base64_decode(const std::string& encoded) {
    const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::vector<uint8_t> result;
    int val = 0, valb = -8;
    
    for (unsigned char c : encoded) {
        if (c == '=') break;
        if (chars.find(c) == std::string::npos) continue;
        
        val = (val << 6) + chars.find(c);
        valb += 6;
        
        if (valb >= 0) {
            result.push_back((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    
    return result;
}

namespace optimum_p2p {

std::vector<std::string> ReadIPsFromFile(const std::string& filename) {
    std::vector<std::string> ips;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        return ips; // Return empty vector if file can't be opened
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        ips.push_back(line);
    }
    
    return ips;
}

std::string SHA256Hex(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        // Return SHA256 of empty string: e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Final(hash, &sha256);
        
        std::ostringstream oss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
        }
        return oss.str();
    }
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data.data(), data.size());
    SHA256_Final(hash, &sha256);
    
    std::ostringstream oss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    
    return oss.str();
}

P2PMessage ParseMessage(const std::vector<uint8_t>& json_data) {
    P2PMessage msg;
    
    try {
        std::string json_str(json_data.begin(), json_data.end());
        nlohmann::json j = nlohmann::json::parse(json_str);
        
        // Extract fields
        if (j.contains("MessageID")) {
            msg.message_id = j["MessageID"].get<std::string>();
        }
        if (j.contains("Topic")) {
            msg.topic = j["Topic"].get<std::string>();
        }
        if (j.contains("SourceNodeID")) {
            msg.source_node_id = j["SourceNodeID"].get<std::string>();
        }
        
        // Handle Message field - can be base64 encoded string or plain string
        if (j.contains("Message")) {
            if (j["Message"].is_string()) {
                std::string message_str = j["Message"].get<std::string>();
                
                // Simple heuristic: if string contains only base64 chars and has '=' or is multiple of 4,
                // try decoding as base64
                bool try_base64 = false;
                if (!message_str.empty()) {
                    bool all_base64_chars = true;
                    for (char c : message_str) {
                        if (!std::isalnum(c) && c != '+' && c != '/' && c != '=') {
                            all_base64_chars = false;
                            break;
                        }
                    }
                    // Try base64 if it looks like base64 (has = or is multiple of 4) and contains + or /
                    if (all_base64_chars && (message_str.find('=') != std::string::npos || 
                        (message_str.find('+') != std::string::npos || message_str.find('/') != std::string::npos) ||
                        message_str.length() % 4 == 0)) {
                        try_base64 = true;
                    }
                }
                
                if (try_base64) {
                    std::vector<uint8_t> decoded = base64_decode(message_str);
                    // Use decoded if it's not empty and shorter than original (base64 expands data)
                    if (!decoded.empty() && decoded.size() < message_str.size()) {
                        msg.message = decoded;
                    } else {
                        // Treat as plain text
                        msg.message.assign(message_str.begin(), message_str.end());
                    }
                } else {
                    // Treat as plain text
                    msg.message.assign(message_str.begin(), message_str.end());
                }
            }
        }
    } catch (const nlohmann::json::exception&) {
        // Return empty message on parse error
    }
    
    return msg;
}

void HandleGossipSubTrace(const std::vector<uint8_t>& data, 
                         bool write_trace,
                         std::function<void(const std::string&)> trace_callback) {
    // For Phase 2, this is a placeholder
    // Full implementation requires protobuf parsing of GossipSub trace events
    // This will be implemented when trace handling is needed
    if (trace_callback) {
        std::string hex_preview = HeadHex(data, 64);
        trace_callback("[GossipSub Trace] " + hex_preview + "...");
    }
}

void HandleOptimumP2PTrace(const std::vector<uint8_t>& data,
                          bool write_trace,
                          std::function<void(const std::string&)> trace_callback) {
    // For Phase 2, this is a placeholder
    // Full implementation requires protobuf parsing of mump2p trace events
    // This will be implemented when trace handling is needed
    if (trace_callback) {
        std::string hex_preview = HeadHex(data, 64);
        trace_callback("[mump2p Trace] " + hex_preview + "...");
    }
}

void WriteToFile(const std::string& filename, 
                const std::string& header,
                std::function<std::string()> data_source) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return;
    }
    
    // Write header if provided
    if (!header.empty()) {
        file << header << "\n";
    }
    
    // Write data lines until data_source returns empty string
    std::string line;
    while (!(line = data_source()).empty()) {
        file << line << "\n";
        file.flush(); // Flush after each line like Go implementation
    }
}

std::string HeadHex(const std::vector<uint8_t>& data, size_t n) {
    size_t len = std::min(data.size(), n);
    std::ostringstream oss;
    
    for (size_t i = 0; i < len; i++) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
    }
    
    return oss.str();
}

} // namespace optimum_p2p


