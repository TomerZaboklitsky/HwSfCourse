// Compile: g++ -std=c++17 -O0 -o not_optimized not_optimized_version.cpp
// Run:     perf stat ./not_optimized

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <random>

struct User {
    int id;
    std::string name;
    std::string email;
    std::string profile_bio;
};

// Data generation

static std::vector<std::vector<User>> generateDatabase(
    int partitionCount, int usersPerPartition, std::mt19937& gen)
{
    std::uniform_int_distribution<> nameLenDist(8, 20);
    const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::uniform_int_distribution<> charDist(0, chars.size() - 1);

    std::vector<std::vector<User>> db(partitionCount);
    int globalId = 0;

    for (int p = 0; p < partitionCount; ++p) {
        db[p].reserve(usersPerPartition);
        for (int i = 0; i < usersPerPartition; ++i) {
            User u;
            u.id = globalId++;

            int len = nameLenDist(gen);
            for (int j = 0; j < len; ++j)
                u.name += chars[charDist(gen)];

            u.email = u.name + "@university-db.edu";
            u.profile_bio = "Biographical information for user "
                          + std::to_string(u.id)
                          + " containing generic textbook string filler.";
            db[p].push_back(u);
        }
    }
    return db;
}

static std::vector<int> generateQueries(int count, int maxId, std::mt19937& gen)
{
    std::uniform_int_distribution<> dist(0, maxId);
    std::vector<int> queries(count);
    for (int& q : queries)
        q = dist(gen);
    return queries;
}

// Search

static User findLinear(const std::vector<User>& partition, int targetId)
{
    for (const User& u : partition)
        if (u.id == targetId)
            return u;
    return {-1, "", "", ""};
}

// Main

int main()
{
    const int PARTITION_COUNT     = 10;
    const int USERS_PER_PARTITION = 2000;
    const int TOTAL_USERS         = PARTITION_COUNT * USERS_PER_PARTITION;
    const int TOTAL_LOOKUPS       = 5000;
    const int REPEAT              = 10;

    std::mt19937 gen(1337);

    std::cout << "[Naive] Generating " << TOTAL_USERS << " users across "
              << PARTITION_COUNT << " partitions..." << std::endl;
    std::vector<std::vector<User>> db = generateDatabase(PARTITION_COUNT, USERS_PER_PARTITION, gen);

    std::cout << "[Naive] Generating " << TOTAL_LOOKUPS << " queries..." << std::endl;
    std::vector<int> queries = generateQueries(TOTAL_LOOKUPS, TOTAL_USERS - 1, gen);

    std::cout << "[Naive] Running sequential scans (" << REPEAT << " rounds)..." << std::endl;

    int found = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> t0 = std::chrono::high_resolution_clock::now();

    for (int r = 0; r < REPEAT; ++r) {
        for (int q = 0; q < TOTAL_LOOKUPS; ++q) {
            for (int p = 0; p < PARTITION_COUNT; ++p) {
                User match = findLinear(db[p], queries[q]);
                if (match.id != -1) { ++found; break; }
            }
        }
    }

    std::chrono::time_point<std::chrono::high_resolution_clock> t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = t1 - t0;

    std::cout << "\n--- Naive Results ---" << std::endl;
    std::cout << "Runtime : " << elapsed.count() << " s" << std::endl;
    std::cout << "Found   : " << found / REPEAT << " / " << TOTAL_LOOKUPS << " (per round)" << std::endl;

    return 0;
}
