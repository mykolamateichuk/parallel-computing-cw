#ifndef PARALLEL_COMPUTING_CW_INVERTED_INDEX_HPP
#define PARALLEL_COMPUTING_CW_INVERTED_INDEX_HPP

#include <iostream>
#include <unordered_map>
#include <set>

#include <shared_mutex>
#include <sstream>

using inverted_index = std::unordered_map<std::string, std::set<std::string>>;

class inverted_index_parallel {
private:
    inverted_index index;
    mutable std::shared_mutex index_mutex;

public:
    void add(const std::string& file_name, const std::string& content);
    std::set<std::string> find(const std::string& term) const;
};

#endif //PARALLEL_COMPUTING_CW_INVERTED_INDEX_HPP
