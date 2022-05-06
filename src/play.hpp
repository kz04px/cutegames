#ifndef PLAY_HPP
#define PLAY_HPP

#include <string>
#include <vector>
#include "engine/engine_ugi.hpp"
#include "events/events.hpp"
#include "match.hpp"
#include "settings.hpp"
#include "store.hpp"

void play_game(const std::size_t game_id,
               const std::string fen,
               const std::size_t engine1_id,
               const std::size_t engine2_id,
               const MatchSettings &settings,
               libevents::Dispatcher &dispatcher,
               Store<UGIEngine> &engine_store);

#endif
