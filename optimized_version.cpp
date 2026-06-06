// Compile: g++ -std=c++17 -O0 -o optimized optimized_version.cpp
// Run:     perf stat ./optimized

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

// Hash index: only stores (id, record_index) - 8 bytes per slot, cache-friendly.
// Full User data is fetched from main_storage only after a confirmed hit.

struct IndexEntry {
    int id           = -1;
    int record_index = -1;
};

class IndexedDatabase {
    std::vector<User>       main_storage;
    std::vector<IndexEntry> index;
    size_t                  capacity;

    size_t slot(int key) const { return static_cast<size_t>(key) % capacity; }

public:
    explicit IndexedDatabase(size_t expected)
        : capacity(expected * 2)
    {
        index.resize(capacity);
        main_storage.reserve(expected);
    }

    void insert(const User& u)
    {
        main_storage.push_back(u);
        int pos = static_cast<int>(main_storage.size()) - 1;

        size_t i = slot(u.id);
        while (index[i].id != -1)
            i = (i + 1) % capacity;

        index[i] = {u.id, pos};
    }

    bool find(int targetId, User& out) const
    {
        size_t i = slot(targetId);
        while (index[i].id != -1) {
            if (index[i].id == targetId) {
                out = main_storage[index[i].record_index];
                return true;
            }
            i = (i + 1) % capacity;
        }
        return false;
    }
};

// Data generation

static IndexedDatabase generateDatabase(
    int partitionCount, int usersPerPartition, std::mt19937& gen)
{
    const int total = partitionCount * usersPerPartition;
    IndexedDatabase db(total);

    std::uniform_int_distribution<> nameLenDist(8, 20);
    const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::uniform_int_distribution<> charDist(0, chars.size() - 1);

    for (int id = 0; id < total; ++id) {
        User u;
        u.id = id;

        int len = nameLenDist(gen);
        for (int j = 0; j < len; ++j)
            u.name += chars[charDist(gen)];

        u.email       = u.name + "@university-db.edu";
        u.profile_bio = "Biographical information for user "
                      + std::to_string(u.id)
                      + " containing generic textbook string filler.";
        db.insert(u);
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

// Main

int main()
{
    const int PARTITION_COUNT     = 10;
    const int USERS_PER_PARTITION = 2000;
    const int TOTAL_USERS         = PARTITION_COUNT * USERS_PER_PARTITION;
    const int TOTAL_LOOKUPS       = 5000;
    const int REPEAT              = 10000;

    std::mt19937 gen(1337);

    std::cout << "[Optimized] Generating " << TOTAL_USERS << " users into hash index..." << std::endl;
    IndexedDatabase db = generateDatabase(PARTITION_COUNT, USERS_PER_PARTITION, gen);

    std::cout << "[Optimized] Generating " << TOTAL_LOOKUPS << " queries..." << std::endl;
    std::vector<int> queries = generateQueries(TOTAL_LOOKUPS, TOTAL_USERS - 1, gen);

    std::cout << "[Optimized] Running indexed lookups (" << REPEAT << " rounds)..." << std::endl;

    int  found = 0;
    User match;
    std::chrono::time_point<std::chrono::high_resolution_clock> t0 = std::chrono::high_resolution_clock::now();

    for (int r = 0; r < REPEAT; ++r) {
        for (int q = 0; q < TOTAL_LOOKUPS; ++q) {
            if (db.find(queries[q], match))
                ++found;
        }
    }

    std::chrono::time_point<std::chrono::high_resolution_clock> t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = t1 - t0;

    std::cout << "\n--- Optimized Results ---" << std::endl;
    std::cout << "Runtime : " << elapsed.count() << " s" << std::endl;
    std::cout << "Found   : " << found / REPEAT << " / " << TOTAL_LOOKUPS << " (per round)" << std::endl;

    return 0;
}
