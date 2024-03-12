#ifndef ENGINE_UGI_HPP
#define ENGINE_UGI_HPP

#include <string>
#include "engine_process.hpp"

class [[nodiscard]] UGIEngine final : public ProcessEngine {
   public:
    [[nodiscard]] UGIEngine(const id_type id, const std::string &path, const std::string &parameters);

    [[nodiscard]] UGIEngine(const id_type id,
                            const std::string &path,
                            const std::string &parameters,
                            callback_type recv,
                            callback_type send);

    ~UGIEngine() override;

    [[nodiscard]] auto is_gameover() const noexcept -> bool;

    auto init() -> void override;

    auto is_ready() -> void override;

    auto newgame() -> void override;

    auto quit() -> void override;

    auto stop() -> void override;

    auto position(const std::string &start_fen, const std::vector<std::string> &move_history) -> void override;

    auto set_option(const std::string &name, const std::string &value) -> void override;

    [[nodiscard]] auto go(const SearchSettings &settings) -> std::string override;

    [[nodiscard]] auto query_p1turn() -> bool override;

    [[nodiscard]] auto query_gameover() -> bool override;

    [[nodiscard]] auto query_result() -> std::string override;
};

#endif
