#ifndef ENGINE_UGI_HPP
#define ENGINE_UGI_HPP

#include <memory>
#include <string>
#include "engine_process.hpp"

class UGIEngine : public ProcessEngine {
   public:
    [[nodiscard]] UGIEngine(const id_type id, const std::string &path, const std::string &parameters);

    [[nodiscard]] UGIEngine(const id_type id,
                            const std::string &path,
                            const std::string &parameters,
                            callback_type recv,
                            callback_type send);

    virtual ~UGIEngine();

    [[nodiscard]] auto is_legal(const Game &, const std::string &) const noexcept -> bool;

    [[nodiscard]] auto is_gameover() const noexcept -> bool;

    void ugi();

    void isready();

    void newgame();

    void quit();

    void stop();

    virtual auto position(const Game &game) -> void override;

    [[nodiscard]] virtual auto go(const SearchSettings &settings) -> std::string override;

    [[nodiscard]] virtual auto query_p1turn() -> bool override;

    [[nodiscard]] virtual auto query_gameover() -> bool override;

    [[nodiscard]] virtual auto query_result() -> GameResult override;

    auto set_option(const std::string &name, const std::string &value) -> void;

   private:
};

[[nodiscard]] auto make_engine(const EngineSettings &settings, const bool debug) -> std::shared_ptr<UGIEngine>;

#endif
