#include "inverted_index.hpp"

void inverted_index_parallel::add(const std::string& file_name, const std::string& content) {
    std::istringstream stream(content);
    std::string word;

    while (stream >> word) {
        std::unique_lock lock(index_mutex);
        index[word].insert(file_name);
    }
}


std::set<std::string> inverted_index_parallel::find(const std::string& term) const {
    std::shared_lock lock(index_mutex);
    if (index.find(term) != index.end()) {
        return index.at(term);
    }
    return {};
}
