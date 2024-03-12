#include "engine_uai.hpp"
#include <utility>
#include <utils.hpp>

[[nodiscard]] UAIEngine::UAIEngine(const id_type id, const std::string &path, const std::string &parameters)
    : ProcessEngine(id, path, parameters) {
}

[[nodiscard]] UAIEngine::UAIEngine(const id_type id,
                                   const std::string &path,
                                   const std::string &parameters,
                                   callback_type recv,
                                   callback_type send)
    : ProcessEngine(id, path, parameters, std::move(recv), std::move(send)) {
}

UAIEngine::~UAIEngine() {
    send("quit");
}

[[nodiscard]] auto UAIEngine::is_gameover() const noexcept -> bool {
    return true;
}

auto UAIEngine::init() -> void {
    send("uai");
    wait_for("uaiok");
}

auto UAIEngine::is_ready() -> void {
    send("isready");
    wait_for("readyok");
}

auto UAIEngine::newgame() -> void {
    send("uainewgame");
}

auto UAIEngine::quit() -> void {
    send("quit");
}

auto UAIEngine::stop() -> void {
    send("stop");
}

auto UAIEngine::set_option(const std::string &name, const std::string &value) -> void {
    send("setoption name " + name + " value " + value);
}

auto UAIEngine::position(const std::string &start_fen, const std::vector<std::string> &move_history) -> void {
    auto msg = std::string();

    if (start_fen.empty() || start_fen == "startpos") {
        msg += "position startpos";
    } else {
        msg += "position fen " + start_fen;
    }

    if (!move_history.empty()) {
        msg += " moves";
        for (const auto &move : move_history) {
            msg += " " + move;
        }
    }

    send(msg);
}

[[nodiscard]] auto UAIEngine::go(const SearchSettings &settings) -> std::string {
    auto movestr = std::string("0000");

    switch (settings.type) {
        case SearchSettings::Type::Time: {
            auto str = std::string();
            str += "go";
            str += " btime " + std::to_string(settings.p1time);
            str += " wtime " + std::to_string(settings.p2time);
            str += " binc " + std::to_string(settings.p1inc);
            str += " winc " + std::to_string(settings.p2inc);
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

[[nodiscard]] auto UAIEngine::query_p1turn() -> bool {
    return false;
}

[[nodiscard]] auto UAIEngine::query_gameover() -> bool {
    return false;
}

[[nodiscard]] auto UAIEngine::query_result() -> std::string {
    return "";
}
