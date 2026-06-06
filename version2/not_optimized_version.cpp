#include <iostream>
#include <vector>
#include <string>
#include <chrono>

struct NaiveRule {
    std::string ip_string;      // Heavy string object
    std::string action;         // "ALLOW" or "DENY"
    int port;
};

struct NaivePacket {
    std::string src_ip;         // Heavy string object
    int port;
};

// Simulated unoptimized linear lookup loop
bool process_packets_naive(const std::vector<NaivePacket>& packets, const std::vector<NaiveRule>& rules) {
    long long allowed_count = 0;
    
    for (const NaivePacket& packet : packets) {
        for (const NaiveRule& rule : rules) {
            // Severe Cache Pollution: Comparing heavy strings byte-by-byte
            if (packet.src_ip == rule.ip_string && packet.port == rule.port) {
                if (rule.action == "ALLOW") {
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
    
    std::string dummy_ip_str = "192.168.100.50";
    int target_port = 8080;

    std::cout << "Running Naive Baseline (100 rounds)..." << std::endl;
    
    // Populate structures
    std::vector<NaiveRule> rules(NUM_RULES, {"10.0.0.1", "DENY", 1234});
    rules[NUM_RULES - 1] = {dummy_ip_str, "ALLOW", target_port}; // Match at the end to force full scan
    
    std::vector<NaivePacket> packets(NUM_PACKETS, {dummy_ip_str, target_port});

    // Loop rounds for profiling visibility
    std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
    for (int round = 0; round < 100; ++round) {
        process_packets_naive(packets, rules);
    }
    std::chrono::time_point<std::chrono::high_resolution_clock> end = std::chrono::high_resolution_clock::now();
    
    std::cout << "Naive Duration: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() 
              << " ms" << std::endl;

    return 0;
}