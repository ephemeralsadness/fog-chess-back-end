#include "Game.h"

Chessboard& Game::GetChessboard() {
    return *chessboard;
}

bool Game::CheckPlayerWhites(unsigned int id) {
    return id == player_whites;
}

bool Game::CheckPlayerBlacks(unsigned int id) {
    return id == player_blacks;
}

GameStatus Game::GetStatus() {
    return status;
}