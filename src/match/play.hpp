#ifndef MATCH_PLAY_HPP
#define MATCH_PLAY_HPP

#include <libevents.hpp>
#include <memory>
#include <string>
#include "engine/engine.hpp"
#include "games/game.hpp"
#include "settings.hpp"
#include "store.hpp"

void play_game(std::shared_ptr<Game> pos,
               const std::size_t game_id,
               const std::string fen,
               const std::size_t engine1_id,
               const std::size_t engine2_id,
               const MatchSettings &settings,
               libevents::Dispatcher &dispatcher,
               Store<Engine> &engine_store);

#endif
