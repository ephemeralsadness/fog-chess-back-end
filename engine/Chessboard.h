#pragma once

#include "Coords.h"

#include <string>

class Chessboard {
public:
    Chessboard() noexcept;
    Chessboard(const std::string& fen) noexcept;

    char At(Coords coords);

    bool MakeMove(bool is_black_move, Coords from, Coords to, char figure = '0') noexcept;

    bool IsCheck(bool to_black_king) const noexcept;
    bool IsMate(bool to_black_king) const noexcept;

private:
    bool CheckMovePawn(bool is_black_move);
    bool CheckMoveKnight(bool is_black_move);
    bool CheckMoveBishop(bool is_black_move);
    bool CheckMoveRook(bool is_black_move);
    bool CheckMoveQueen(bool is_black_move);
    bool CheckMoveKing(bool is_black_move);

    char table[8][8];
};