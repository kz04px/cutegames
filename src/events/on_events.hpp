#ifndef ON_EVENTS_HPP
#define ON_EVENTS_HPP

#include <libevents.hpp>
#include <memory>
#include <vector>
#include "../engine/engine.hpp"
#include "../match/settings.hpp"
#include "../match/statistics.hpp"

auto on_game_finished(const std::shared_ptr<libevents::Event> &,
                      const MatchSettings &,
                      MatchStatistics &,
                      std::vector<EngineStatistics> &,
                      libevents::Dispatcher &) noexcept -> void;

auto on_game_started(const std::shared_ptr<libevents::Event> &, const MatchSettings &) noexcept -> void;

auto on_engine_loaded(const std::shared_ptr<libevents::Event> &,
                      const MatchSettings &,
                      MatchStatistics &) noexcept -> void;

auto on_engine_unloaded(const std::shared_ptr<libevents::Event> &,
                        const MatchSettings &,
                        MatchStatistics &) noexcept -> void;

auto on_match_finished(const std::shared_ptr<libevents::Event> &, bool &) noexcept -> void;

#endif
