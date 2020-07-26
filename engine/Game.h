#pragma once

#include "Chessboard.h"

#include <memory>

enum GameStatus {
    NOT_STARTED,
    ONGOING,
    FINISHED
};


class Game {
public:
    Game(unsigned int player_whites, unsigned int player_blacks)
        : chessboard(new Chessboard()),
          player_whites(player_whites),
          player_blacks(player_blacks),
          status(GameStatus::NOT_STARTED) {}

    Chessboard& GetChessboard();
    bool CheckPlayerWhites(unsigned int id);
    bool CheckPlayerBlacks(unsigned int id);
    GameStatus GetStatus();
private:
    std::unique_ptr<Chessboard> chessboard;
    unsigned int player_whites;
    unsigned int player_blacks;
    GameStatus status;
};