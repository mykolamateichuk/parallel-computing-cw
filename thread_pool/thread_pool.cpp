#include "thread_pool.hpp"

thread_pool::thread_pool(inverted_index_parallel& index, const int thread_count) : m_index(index) {
    for (int i = 0; i < thread_count; i++) {
        m_threads.push_back(std::thread(&thread_pool::routine, this));
    }
}

void thread_pool::destroy(const bool safe_stop) {
    std::unique_lock lock(m_queueMtx);

    if (m_destroyed) return;

    if (!safe_stop) {
        while (!m_queue.empty()) {
            m_queue.pop();
        }
    }

    m_destroyed = true;
    lock.unlock();

    m_taskCv.notify_all();

    for (auto& t : m_threads) {
        t.join();
    }

    m_threads.clear();
}

std::future<void> thread_pool::add_task(task_t&& task, std::vector<std::string> file_names) {
    std::shared_ptr<std::promise<void>> task_promise = std::make_shared<std::promise<void>>();
    std::future<void> task_future = task_promise->get_future();

    std::unique_lock lock(m_queueMtx);
    if (m_destroyed) return task_future;

    m_queue.emplace(
        [task, task_promise, this](inverted_index_parallel& index, std::vector<std::string> file_names) {
            task(index, file_names);
            task_promise->set_value();
        },
        file_names
    );
    lock.unlock();

    m_taskCv.notify_one();

    return task_future;
}

void thread_pool::routine() {
    while (true) {
        std::unique_lock lock(m_queueMtx);
        m_taskCv.wait(lock, [this] { return m_destroyed || !m_queue.empty(); });

        if (m_destroyed && m_queue.empty()) return;

        auto pair = m_queue.front();
        m_queue.pop();

        lock.unlock();

        pair.first(m_index, pair.second);
    }
}
