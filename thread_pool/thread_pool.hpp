#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <queue>
#include <thread>
#include <iostream>
#include <shared_mutex>
#include <condition_variable>
#include <functional>
#include <future>

#include "../inverted_index/inverted_index.hpp"

class thread_pool {
public:
    using task_t = std::function<void(inverted_index_parallel& index, std::vector<std::string>)>;

    explicit thread_pool(inverted_index_parallel& index, int thread_count);
    ~thread_pool() { destroy(); }

    void destroy(bool safe_stop = true);

    std::future<void> add_task(task_t&& task, std::vector<std::string> file_names);
private:
    void routine();

    mutable std::shared_mutex m_queueMtx;
    mutable std::condition_variable_any m_taskCv;

    std::vector<std::thread> m_threads;
    std::queue<std::pair<task_t, std::vector<std::string>>> m_queue;
    inverted_index_parallel& m_index;

    bool m_destroyed = false;
};

#endif // THREAD_POOL_HPP
