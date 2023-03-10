#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
   public:
    using job_type = std::function<void()>;
    using size_type = std::queue<job_type>::size_type;

    [[nodiscard]] ThreadPool() noexcept = default;

    [[nodiscard]] explicit ThreadPool(const std::size_t num_workers) noexcept {
        set_threads(num_workers);
    }

    ~ThreadPool() {
        clear();
    }

    auto clear() noexcept -> void {
        m_stop = true;

        m_cv_work.notify_all();

        for (auto &t : m_workers) {
            if (t.joinable()) {
                t.join();
            }
        }

        m_work_queue = {};
    }

    [[nodiscard]] auto size() const noexcept -> size_type {
        return m_work_queue.size();
    }

    auto set_threads(const std::size_t num_threads) noexcept -> void {
        m_workers.clear();
        for (std::size_t t = 0; t < num_threads; ++t) {
            m_workers.emplace_back(&ThreadPool::worker, this);
        }
    }

    auto add_job(job_type job) -> void {
        std::unique_lock<std::mutex> lock(mutex_);
        m_work_queue.push(job);
        m_cv_work.notify_one();
    }

    auto wait() noexcept -> void {
        std::mutex asd;
        std::unique_lock<std::mutex> lock(asd);
        m_cv_finished.wait(lock, [&] {
            return m_work_queue.empty() && m_num_busy_workers == 0;
        });
    }

   private:
    auto worker() -> void {
        while (true) {
            std::unique_lock<std::mutex> lock(mutex_);
            m_cv_work.wait(lock, [&] {
                return !m_work_queue.empty() || m_stop;
            });

            // Stop or get work
            if (m_stop) {
                break;
            } else if (!m_work_queue.empty()) {
                auto job = m_work_queue.front();
                m_work_queue.pop();
                m_num_busy_workers++;

                // Release lock
                lock.unlock();

                // Do work
                job();

                lock.lock();
                m_num_busy_workers--;
                m_cv_finished.notify_all();
                lock.unlock();
            }
        }
    }

    std::mutex mutex_;
    std::condition_variable m_cv_work;
    std::condition_variable m_cv_finished;
    std::vector<std::thread> m_workers;
    std::queue<job_type> m_work_queue;
    std::atomic_uint m_num_busy_workers;
    bool m_stop = false;
};

#endif
