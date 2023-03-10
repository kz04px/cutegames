#include "engine_ugi.hpp"
#include <utils.hpp>

[[nodiscard]] UGIEngine::UGIEngine(const id_type id, const std::string &path) : ProcessEngine(id, path) {
}

[[nodiscard]] UGIEngine::UGIEngine(const id_type id, const std::string &path, callback_type recv, callback_type send)
    : ProcessEngine(id, path, recv, send) {
}

UGIEngine::~UGIEngine() {
    send("quit");
}

[[nodiscard]] auto UGIEngine::is_legal(const Game &, const std::string &) const noexcept -> bool {
    return true;
}

[[nodiscard]] auto UGIEngine::is_gameover() const noexcept -> bool {
    return true;
}

void UGIEngine::ugi() {
    send("ugi");
    wait_for("ugiok");
}

void UGIEngine::isready() {
    send("isready");
    wait_for("readyok");
}

void UGIEngine::newgame() {
    send("uginewgame");
}

void UGIEngine::quit() {
    send("quit");
}

void UGIEngine::stop() {
    send("stop");
}

auto UGIEngine::position(const Game &game) -> void {
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

[[nodiscard]] auto UGIEngine::go(const SearchSettings &settings) -> std::string {
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

[[nodiscard]] auto UGIEngine::query_p1turn() -> bool {
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

[[nodiscard]] auto UGIEngine::query_gameover() -> bool {
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

[[nodiscard]] auto UGIEngine::query_result() -> GameResult {
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

auto UGIEngine::set_option(const std::string &name, const std::string &value) -> void {
    send("setoption name " + name + " value " + value);
}
