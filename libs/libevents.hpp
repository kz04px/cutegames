#ifndef LIBEVENTS_EVENTS_HPP
#define LIBEVENTS_EVENTS_HPP

#include <condition_variable>
#include <functional>
#include <memory>
#include <queue>
#include <unordered_map>

namespace libevents {

struct [[nodiscard]] Event {
    using EventIDType = unsigned int;

    Event() = default;

    Event(const Event &event) = delete;

    Event(const Event &&event) = delete;

    virtual ~Event() = default;

    auto operator=(const Event &rhs) = delete;

    auto operator=(const Event &&rhs) = delete;

    [[nodiscard]] virtual auto id() const noexcept -> EventIDType = 0;
};

class [[nodiscard]] Dispatcher {
   public:
    using listener_type = std::function<void(std::shared_ptr<Event>)>;

    [[nodiscard]] bool empty() const noexcept {
        return event_queue.empty();
    }

    [[nodiscard]] auto size() const noexcept {
        return event_queue.size();
    }

    void register_event_listener(const Event::EventIDType id, const listener_type &func) {
        event_listeners[id].emplace_back(func);
    }

    void post_event(const std::shared_ptr<Event> &event) {
        std::unique_lock<std::mutex> lock(mutex);
        event_queue.push(event);
        cv_work.notify_one();
    }

    void send_event(const std::shared_ptr<Event> &event) {
        const auto iter = event_listeners.find(event->id());

        // No event listeners
        if (iter == event_listeners.end()) {
            return;
        }

        // Tell all the event listeners what's happened
        for (const auto &listener : iter->second) {
            listener(event);
        }
    }

    void send_all() {
        std::unique_lock<std::mutex> lock(mutex);
        while (!event_queue.empty()) {
            const auto event = event_queue.front();
            event_queue.pop();
            lock.unlock();
            send_event(event);
            lock.lock();
        }
    }

    void wait() {
        if (!event_queue.empty()) {
            return;
        }

        std::unique_lock<std::mutex> lock(mutex);
        cv_work.wait(lock, [&] {
            return !event_queue.empty();
        });
    }

    auto clear() noexcept -> void {
        event_queue = {};
    }

   private:
    std::mutex mutex;
    std::condition_variable cv_work;
    std::unordered_map<Event::EventIDType, std::vector<listener_type>> event_listeners;
    std::queue<std::shared_ptr<Event>> event_queue;
};

}  // namespace libevents

#endif
