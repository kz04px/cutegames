#ifndef ENGINE_UAI_HPP
#define ENGINE_UAI_HPP

#include <string>
#include "engine_process.hpp"

class UAIEngine final : public ProcessEngine {
   public:
    [[nodiscard]] UAIEngine(const id_type id, const std::string &path, const std::string &parameters);

    [[nodiscard]] UAIEngine(const id_type id,
                            const std::string &path,
                            const std::string &parameters,
                            callback_type recv,
                            callback_type send);

    virtual ~UAIEngine();

    [[nodiscard]] auto is_gameover() const noexcept -> bool;

    virtual auto init() -> void override;

    virtual auto is_ready() -> void override;

    virtual auto newgame() -> void override;

    virtual auto quit() -> void override;

    virtual auto stop() -> void override;

    virtual auto position(const std::string &start_fen, const std::vector<std::string> &move_history) -> void override;

    virtual auto set_option(const std::string &name, const std::string &value) -> void override;

    [[nodiscard]] virtual auto go(const SearchSettings &settings) -> std::string override;

    [[nodiscard]] virtual auto query_p1turn() -> bool override;

    [[nodiscard]] virtual auto query_gameover() -> bool override;

    [[nodiscard]] virtual auto query_result() -> std::string override;

   private:
};

#endif
