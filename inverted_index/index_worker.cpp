#include "index_worker.hpp"

void index_worker::process() {
    while (!stop_flag.load()) {
        std::unique_lock lock(queue_mtx);
        queue_cv.wait(lock, [this]() { return !upd_queue.empty() || stop_flag.load(); });

        while (!upd_queue.empty()) {
            auto [file_name, content] = upd_queue.front();

            upd_queue.pop();
            lock.unlock();
            index.add(file_name, content);
            lock.lock();
        }
    }
}

index_worker::index_worker(inverted_index_parallel& idx) : index(idx), stop_flag(false) {
    worker = std::thread(&index_worker::process, this);
}

index_worker::~index_worker() {
    stop_flag.store(true);
    queue_cv.notify_all();
    if (worker.joinable()) worker.join();
}

void index_worker::update(const std::string& file_name, const std::string &content) {
    {
        std::lock_guard lock(queue_mtx);
        upd_queue.emplace(file_name, content);
    }
    queue_cv.notify_one();
}
