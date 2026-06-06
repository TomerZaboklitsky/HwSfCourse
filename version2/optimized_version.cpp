#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <stdint.h>

struct OptimizedRule {
    uint32_t ip_bits;   // 32-bit integer instead of a string
    uint32_t port;      // Packed cleanly next to IP
    uint32_t action;    // 1 for ALLOW, 0 for DENY (Eliminates string branches)
};

struct OptimizedPacket {
    uint32_t src_ip;
    uint32_t port;
};

// Simulated hardware-aware lookup loop
bool process_packets_optimized(const std::vector<OptimizedPacket>& packets, const std::vector<OptimizedRule>& rules) {
    long long allowed_count = 0;
    
    for (const OptimizedPacket& packet : packets) {
        // Continuous, flat memory scan that the prefetcher predicts flawlessly
        for (const OptimizedRule& rule : rules) {
            // Direct register comparisons instead of string manipulation
            if (packet.src_ip == rule.ip_bits && packet.port == rule.port) {
                if (rule.action == 1) {
                    allowed_count++;
                }
                break;
            }
        }
    }
    return allowed_count > 0;
}

int main() {
    const int NUM_RULES = 2000;
    const int NUM_PACKETS = 5000;
    
    uint32_t dummy_ip_bits = 0xC0A86432; // 192.168.100.50 in hex representation
    int target_port = 8080;

    std::cout << "Running Optimized Implementation (100 rounds)..." << std::endl;
    
    // Populate structures
    std::vector<OptimizedRule> rules(NUM_RULES, {0x0A000001, 1234, 0});
    rules[NUM_RULES - 1] = {dummy_ip_bits, (uint32_t)target_port, 1};
    
    std::vector<OptimizedPacket> packets(NUM_PACKETS, {dummy_ip_bits, (uint32_t)target_port});

    // Runs 100x more iterations than naive so perf can gather a distinct sampling signature
    std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
    for (int round = 0; round < 100; ++round) {
        process_packets_optimized(packets, rules);
    }
    std::chrono::time_point<std::chrono::high_resolution_clock> end = std::chrono::high_resolution_clock::now();
    
    std::cout << "Optimized Duration: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() 
              << " ms" << std::endl;
    return 0;
}