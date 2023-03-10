#ifndef TOURNAMENT_GENERATOR_HPP
#define TOURNAMENT_GENERATOR_HPP

#include <cstdint>

struct [[nodiscard]] GameInfo {
    std::size_t id = 0;
    std::size_t idx_opening = 0;
    std::size_t idx_player1 = 0;
    std::size_t idx_player2 = 0;

    [[nodiscard]] constexpr auto operator==(const GameInfo &rhs) const noexcept -> bool {
        return id == rhs.id && idx_opening == rhs.idx_opening && idx_player1 == rhs.idx_player1 &&
               idx_player2 == rhs.idx_player2;
    }
};

class TournamentGenerator {
   public:
    virtual ~TournamentGenerator() {
    }

    [[nodiscard]] virtual auto is_finished() -> bool = 0;

    [[nodiscard]] virtual auto expected() -> std::size_t = 0;

    [[nodiscard]] virtual auto next() -> GameInfo = 0;

   private:
    virtual auto increment() -> void = 0;
};

#endif
