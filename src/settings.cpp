#include "settings.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <termcolor/termcolor.hpp>

auto print_settings(const MatchSettings &settings) noexcept -> void {
    std::cout << "Match settings loaded:\n";
    std::cout << "- threads " << settings.num_threads << "\n";
    std::cout << "- games " << settings.num_games << "\n";
    std::cout << "- store size " << settings.engine_store_size << "\n";
    switch (settings.timecontrol.type) {
        case SearchSettings::Type::Time:
            std::cout << "- tc " << settings.timecontrol.p1time << "+" << settings.timecontrol.p1inc << "ms\n";
            break;
        case SearchSettings::Type::Movetime:
            std::cout << "- tc " << settings.timecontrol.movetime << "ms\n";
            break;
        case SearchSettings::Type::Depth:
            std::cout << "- tc " << settings.timecontrol.ply << "ply\n";
            break;
        case SearchSettings::Type::Nodes:
            std::cout << "- tc " << settings.timecontrol.nodes << "nodes\n";
            break;
    }
    std::cout << "- openings_path " << settings.openings_path << "\n";
    std::cout << "- timeoutbuffer " << settings.adjudication.timeoutbuffer << "ms\n";
    std::cout << "- maxfullmoves " << settings.adjudication.maxfullmoves << "\n";
    std::cout << "- update_frequency " << settings.update_frequency << "\n";
    std::cout << "- debug " << settings.debug << "\n";
    std::cout << "- repeat " << settings.repeat << "\n";
    std::cout << "- recover " << settings.recover << "\n";
    std::cout << "- verbose " << settings.verbose << "\n";
}

[[nodiscard]] auto read_json(const std::string &path) -> nlohmann::json {
    std::ifstream i(path);

    if (!i.is_open()) {
        throw std::invalid_argument("Could not open settings file " + path);
    }

    try {
        nlohmann::json json;
        i >> json;
        return json;
    } catch (nlohmann::json::exception &e) {
        throw e;
    } catch (...) {
        throw "Failure parsing .json";
    }
}

[[nodiscard]] auto get_settings(const std::string &path) -> MatchSettings {
    const auto json = read_json(path);
    auto settings = MatchSettings();

    std::unordered_map<std::string, std::string> engine_options;

    for (const auto &[key, value] : json.items()) {
        if (key == "games") {
            settings.num_games = value.get<int>();
        } else if (key == "concurrency") {
            settings.num_threads = value.get<int>();
        } else if (key == "ratinginterval") {
            settings.update_frequency = value.get<int>();
        } else if (key == "debug") {
            settings.debug = value.get<bool>();
        } else if (key == "recover") {
            settings.recover = value.get<bool>();
        } else if (key == "verbose") {
            settings.verbose = value.get<bool>();
        } else if (key == "protocol") {
            for (const auto &[a, b] : value.items()) {
                if (a == "askturn") {
                    settings.protocol.ask_turn = b.get<bool>();
                }
            }
        } else if (key == "adjudication") {
            for (const auto &[a, b] : value.items()) {
                if (a == "timeoutbuffer") {
                    settings.adjudication.timeoutbuffer = b.get<int>();
                } else if (a == "maxfullmoves") {
                    settings.adjudication.maxfullmoves = b.get<int>();
                }
            }
        } else if (key == "timecontrol") {
            for (const auto &[a, b] : value.items()) {
                // Search types
                if (a == "type") {
                    if (b == "clock") {
                        settings.timecontrol.type = SearchSettings::Type::Time;
                    } else if (b == "movetime") {
                        settings.timecontrol.type = SearchSettings::Type::Movetime;
                    } else if (b == "depth") {
                        settings.timecontrol.type = SearchSettings::Type::Depth;
                    } else if (b == "nodes") {
                        settings.timecontrol.type = SearchSettings::Type::Nodes;
                    }
                } else if (a == "time") {
                    settings.timecontrol.p1time = b.get<int>();
                    settings.timecontrol.p2time = b.get<int>();
                    settings.timecontrol.movetime = b.get<int>();
                } else if (a == "increment") {
                    settings.timecontrol.p1inc = b.get<int>();
                    settings.timecontrol.p2inc = b.get<int>();
                } else if (a == "nodes") {
                    settings.timecontrol.nodes = b.get<int>();
                } else if (a == "ply") {
                    settings.timecontrol.ply = b.get<int>();
                }
            }
        } else if (key == "openings") {
            for (const auto &[a, b] : value.items()) {
                if (a == "path") {
                    settings.openings_path = b.get<std::string>();
                } else if (a == "repeat") {
                    settings.repeat = b.get<bool>();
                } else if (a == "shuffle") {
                    settings.shuffle_openings = b.get<bool>();
                }
            }
        } else if (key == "sprt") {
            for (const auto &[a, b] : value.items()) {
                if (a == "enabled") {
                    settings.sprt.enabled = b.get<bool>();
                } else if (a == "confidence") {
                    settings.sprt.alpha = 1.0f - b.get<float>();
                    settings.sprt.beta = 1.0f - b.get<float>();
                } else if (a == "alpha") {
                    settings.sprt.alpha = b.get<float>();
                } else if (a == "beta") {
                    settings.sprt.beta = b.get<float>();
                } else if (a == "elo0") {
                    settings.sprt.elo0 = b.get<float>();
                } else if (a == "elo1") {
                    settings.sprt.elo1 = b.get<float>();
                }
            }
        } else if (key == "options") {
            for (const auto &[a, b] : value.items()) {
                engine_options[a] = b;
            }
        } else if (key == "pgn") {
        }
    }

    if (settings.openings_path.empty()) {
        throw std::invalid_argument("Settings .json must include \"openings\" option");
    }

    auto iter = json.find("engines");

    if (iter == json.end()) {
        throw std::invalid_argument("Settings .json must include \"engines\" option");
    }

    for (const auto &[key, value] : (*iter).items()) {
        auto gg = EngineSettings();

        gg.id = settings.engine_settings.size();

        // Copy global engine options
        gg.options = engine_options;

        // asd
        for (const auto &[a, b] : value.items()) {
            if (a == "name") {
                gg.name = b;
            } else if (a == "path") {
                gg.path = b;
            } else if (a == "parameters") {
                gg.parameters = b;
            } else if (a == "options") {
                for (const auto &[f, g] : b.items()) {
                    gg.options[f] = g;
                }
            }
        }

        settings.engine_settings.emplace_back(gg);
    }

    if (settings.engine_settings.size() < 2) {
        throw std::invalid_argument("Settings .json must include at least two engines");
    }

    return settings;
}
