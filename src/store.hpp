#ifndef CUTEGAMES_STORE_HPP
#define CUTEGAMES_STORE_HPP

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

template <typename T>
class Store {
   public:
    using entry_ptr_type = std::shared_ptr<T>;

    /*
    class Wrapper {
       public:
        [[nodiscard]] Wrapper() = default;

        [[nodiscard]] Wrapper(const key_type id, entry_ptr_type ptr, Store<T> *store)
            : m_id(id), m_ptr(ptr), m_store(store) {
        }

        ~Wrapper() {
            if (m_store) {
                m_store->release(m_id, m_ptr);
            }
        }

        [[nodiscard]] std::shared_ptr<T> operator*() {
            return m_ptr;
        }

        [[nodiscard]] auto operator->() -> entry_ptr_type {
            return m_ptr;
        }

        [[nodiscard]] operator bool() const noexcept {
            return m_ptr != nullptr;
        }

        //void release() {
        //    m_store->release(m_id, m_ptr);
        //}

       private:
        key_type m_id = 0;
        entry_ptr_type m_ptr;
        Store<T> *m_store;
    };
    */

    [[nodiscard]] Store(const std::size_t capacity) : m_capacity(capacity) {
    }

    [[nodiscard]] auto get(const std::function<bool(const entry_ptr_type &)> &func) -> std::optional<entry_ptr_type> {
        std::scoped_lock lock(m_mutex);

        auto iter = std::ranges::find_if(m_cache, func);

        // Entry not found
        if (iter == m_cache.end()) {
            return {};
        }

        // Entry found
        auto ptr = *iter;
        m_cache.erase(iter);
        return ptr;
    }

    [[nodiscard]] auto get() -> std::optional<entry_ptr_type> {
        std::scoped_lock lock(m_mutex);

        // Entry not found
        if (m_cache.empty()) {
            return {};
        }

        // Entry found
        auto ptr = m_cache.back();
        m_cache.pop_back();
        return ptr;
    }

    [[nodiscard]] auto size() const noexcept -> std::size_t {
        std::scoped_lock lock(m_mutex);
        return m_cache.size();
    }

    [[nodiscard]] auto capacity() const noexcept -> std::size_t {
        std::scoped_lock lock(m_mutex);
        return m_capacity;
    }

    [[nodiscard]] auto full() const noexcept -> bool {
        std::scoped_lock lock(m_mutex);
        return m_cache.size() == m_capacity;
    }

    [[nodiscard]] auto empty() const noexcept -> bool {
        std::scoped_lock lock(m_mutex);
        return m_cache.empty();
    }

    auto remove_oldest() -> void {
        m_cache.erase(m_cache.begin());
    }

    auto release(std::shared_ptr<T> obj) -> bool {
        std::scoped_lock lock(m_mutex);
        if (m_capacity == 0) {
            return false;
        }

        auto cleared = false;
        if (m_cache.size() == m_capacity) {
            m_cache.erase(m_cache.begin());
            cleared = true;
        }
        m_cache.emplace_back(obj);
        return cleared;
    }

    auto clear() noexcept -> void {
        m_cache.clear();
    }

   private:
    mutable std::mutex m_mutex;
    std::size_t m_capacity = 0;
    std::vector<entry_ptr_type> m_cache;
};

#endif
