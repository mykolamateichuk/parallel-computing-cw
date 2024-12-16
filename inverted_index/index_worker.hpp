#ifndef PARALLEL_COMPUTING_CW_INDEX_UPDATER_HPP
#define PARALLEL_COMPUTING_CW_INDEX_UPDATER_HPP

#include <queue>
#include <thread>

#include "inverted_index.hpp"

class index_worker {
private:
    inverted_index_parallel& index;
    std::queue<std::pair<std::string, std::string>> upd_queue;
    std::mutex queue_mtx;
    std::condition_variable queue_cv;
    std::atomic<bool> stop_flag;
    std::thread worker;

    void process();

public:
    index_worker(inverted_index_parallel& idx);
    ~index_worker();

    void update(const std::string& file_name, const std::string& content);
};

#endif //PARALLEL_COMPUTING_CW_INDEX_UPDATER_HPP
