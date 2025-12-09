#pragma once

#include "types.hpp"
#include <string>
#include <vector>
#include <functional>
#include <chrono>

namespace optimum_p2p {

// Read IP addresses from a file (one per line, comments start with #)
std::vector<std::string> ReadIPsFromFile(const std::string& filename);

// Compute SHA256 hash and return as hex string
std::string SHA256Hex(const std::vector<uint8_t>& data);

// Parse JSON message data into P2PMessage structure
P2PMessage ParseMessage(const std::vector<uint8_t>& json_data);

// Handle GossipSub trace events
void HandleGossipSubTrace(const std::vector<uint8_t>& data, 
                         bool write_trace = false,
                         std::function<void(const std::string&)> trace_callback = nullptr);

// Handle mump2p trace events
void HandleOptimumP2PTrace(const std::vector<uint8_t>& data,
                          bool write_trace = false,
                          std::function<void(const std::string&)> trace_callback = nullptr);

// Write data to file (used for output files)
// Takes a callback that provides data lines until it returns empty string
void WriteToFile(const std::string& filename, 
                const std::string& header,
                std::function<std::string()> data_source);

// Helper function: Get hex representation of first n bytes (for debugging)
std::string HeadHex(const std::vector<uint8_t>& data, size_t n);

} // namespace optimum_p2p

