#ifndef ENGINE_UGI_HPP
#define ENGINE_UGI_HPP

#include <utils.hpp>
#include "engine_process.hpp"

class UGIEngine : public ProcessEngine {
   public:
    [[nodiscard]] UGIEngine(const id_type id, const std::string &path) : ProcessEngine(id, path) {
    }

    [[nodiscard]] UGIEngine(const id_type id, const std::string &path, callback_type recv, callback_type send)
        : ProcessEngine(id, path, recv, send) {
    }

    ~UGIEngine() {
        send("quit");
    }

    [[nodiscard]] auto is_legal(const Game &, const std::string &) const noexcept -> bool {
        return true;
    }

    [[nodiscard]] auto is_gameover() const noexcept -> bool {
        return true;
    }

    void ugi() {
        send("ugi");
        wait_for("ugiok");
    }

    void isready() {
        send("isready");
        wait_for("readyok");
    }

    void newgame() {
        send("uginewgame");
    }

    void quit() {
        send("quit");
    }

    void stop() {
        send("stop");
    }

    virtual auto position(const Game &game) -> void override {
        auto msg = std::string();

        if (game.start_fen().empty() || game.start_fen() == "startpos") {
            msg += "position startpos";
        } else {
            msg += "position fen " + game.start_fen();
        }

        if (!game.move_history().empty()) {
            msg += " moves";
            for (const auto &move : game.move_history()) {
                msg += " " + move;
            }
        }

        send(msg);
    }

    [[nodiscard]] virtual auto go(const SearchSettings &settings) -> std::string override {
        auto movestr = std::string("0000");

        switch (settings.type) {
            case SearchSettings::Type::Time: {
                auto str = std::string();
                str += "go";
                str += " p1time " + std::to_string(settings.p1time);
                str += " p2time " + std::to_string(settings.p2time);
                str += " p1inc " + std::to_string(settings.p1inc);
                str += " p2inc " + std::to_string(settings.p2inc);
                send(str);
                break;
            }
            case SearchSettings::Type::Movetime:
                send("go movetime " + std::to_string(settings.movetime));
                break;
            case SearchSettings::Type::Depth:
                send("go depth " + std::to_string(settings.ply));
                break;
            case SearchSettings::Type::Nodes:
                send("go nodes " + std::to_string(settings.nodes));
                break;
            default:
                return {};
        }

        wait_for([&movestr](const auto &msg) {
            const auto parts = utils::split(msg);

            if (parts.size() != 2) {
                return false;
            }

            if (parts[0] != "bestmove") {
                return false;
            }

            movestr = parts[1];

            return true;
        });

        return movestr;
    }

    [[nodiscard]] virtual auto query_p1turn() -> bool override {
        send("query p1turn");

        auto is_p1 = false;

        wait_for([&is_p1](const auto &msg) {
            const auto parts = utils::split(msg);

            if (parts.size() != 2) {
                return false;
            }

            if (parts[0] != "response") {
                return false;
            }

            is_p1 = parts[1] == "true";

            return true;
        });

        return is_p1;
    }

    [[nodiscard]] virtual auto query_gameover() -> bool override {
        send("query gameover");

        auto is_gameover = false;

        wait_for([&is_gameover](const auto &msg) {
            const auto parts = utils::split(msg);

            if (parts.size() != 2) {
                return false;
            }

            if (parts[0] != "response") {
                return false;
            }

            is_gameover = parts[1] == "true";

            return true;
        });

        return is_gameover;
    }

    [[nodiscard]] virtual auto query_result() -> GameResult override {
        send("query result");

        auto result = GameResult::None;

        wait_for([&result](const auto &msg) {
            const auto parts = utils::split(msg);

            if (parts.size() != 2) {
                return false;
            }

            if (parts[0] != "response") {
                return false;
            }

            if (parts[1] == "p1win") {
                result = GameResult::Player1Win;
            } else if (parts[1] == "p2win") {
                result = GameResult::Player2Win;
            } else if (parts[1] == "draw") {
                result = GameResult::Draw;
            } else if (parts[1] == "none") {
                result = GameResult::None;
            }

            return true;
        });

        return result;
    }

    auto set_option(const std::string &name, const std::string &value) -> void {
        send("setoption name " + name + " value " + value);
    }

   private:
};

[[nodiscard]] auto make_engine(const EngineSettings &settings, const bool debug) -> std::shared_ptr<UGIEngine>;

#endif
