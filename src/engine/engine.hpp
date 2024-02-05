#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>

enum class [[nodiscard]] EngineProtocol
{
    UGI,
    UAI,
};

struct [[nodiscard]] EngineStatistics {
    int played = 0;
    int win = 0;
    int lose = 0;
    int draw = 0;
    int crash = 0;
    int flagged = 0;
};

struct [[nodiscard]] EngineSettings {
    std::size_t id = 0;
    std::string name;
    std::string path;
    std::string parameters;
    EngineProtocol protocol;
    std::unordered_map<std::string, std::string> options;
};

struct [[nodiscard]] SearchSettings {
    enum class Type : int
    {
        Time = 0,
        Movetime,
        Depth,
        Nodes,
    };

    [[nodiscard]] static auto as_time(const int p1t, const int p2t, const int p1i, const int p2i) -> SearchSettings {
        return SearchSettings{.type = Type::Time, .p1time = p1t, .p2time = p2t, .p1inc = p1i, .p2inc = p2i};
    }

    [[nodiscard]] static auto as_movetime(const int time) -> SearchSettings {
        return SearchSettings{.type = Type::Movetime, .movetime = time};
    }

    [[nodiscard]] static auto as_depth(const int p) -> SearchSettings {
        return SearchSettings{.type = Type::Depth, .ply = p};
    }

    [[nodiscard]] static auto as_nodes(const int n) -> SearchSettings {
        return SearchSettings{.type = Type::Nodes, .nodes = n};
    }

    Type type = Type::Depth;
    int p1time = 0;
    int p2time = 0;
    int p1inc = 0;
    int p2inc = 0;
    int movestogo = 0;
    int movetime = 0;
    int ply = 1;
    int nodes = 0;
};

class Engine {
   public:
    using callback_type = std::function<void(const std::string_view)>;
    using id_type = std::size_t;

    virtual ~Engine() = default;

    [[nodiscard]] auto get_id() const noexcept -> id_type {
        return m_id;
    }

    [[nodiscard]] virtual auto is_running() -> bool = 0;

    [[nodiscard]] virtual auto go(const SearchSettings &) -> std::string = 0;

    [[nodiscard]] virtual auto query_p1turn() -> bool = 0;

    [[nodiscard]] virtual auto query_gameover() -> bool = 0;

    [[nodiscard]] virtual auto query_result() -> std::string = 0;

    virtual auto init() -> void = 0;

    virtual auto is_ready() -> void = 0;

    virtual auto newgame() -> void = 0;

    virtual auto quit() -> void = 0;

    virtual auto stop() -> void = 0;

    virtual auto position(const std::string &, const std::vector<std::string> &) -> void = 0;

    virtual auto set_option(const std::string &, const std::string &) -> void = 0;

   protected:
    [[nodiscard]] Engine() {
    }

    [[nodiscard]] Engine(const id_type id) : m_id(id) {
    }

    [[nodiscard]] Engine(const id_type id, callback_type recv, callback_type send)
        : m_recv(recv), m_send(send), m_id(id) {
    }

    callback_type m_recv = [](const auto) {
    };

    callback_type m_send = [](const auto) {
    };

   private:
    id_type m_id = 0;
};

template <typename T>
[[nodiscard]] auto make_engine(const EngineSettings &settings, const bool debug = false) -> std::shared_ptr<T> {
    if (debug) {
        const auto debug_recv = [](const std::string_view &msg) {
            std::cout << "<recv:" << std::this_thread::get_id() << "> " << msg << "\n";
        };

        const auto debug_send = [](const std::string_view &msg) {
            std::cout << "<send:" << std::this_thread::get_id() << "> " << msg << "\n";
        };

        return std::make_shared<T>(settings.id, settings.path, settings.parameters, debug_recv, debug_send);
    } else {
        return std::make_shared<T>(settings.id, settings.path, settings.parameters);
    }
}

#endif
