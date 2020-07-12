#pragma once

#include "Coords.h"

#include <string>
#include <unordered_map>

class Chessboard {
public:
    Chessboard() noexcept : Chessboard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") { }
    Chessboard(const std::string& fen) noexcept;

    char At(Coords coords) const noexcept;

    bool MakeMove(bool is_black_move, Coords from, Coords to, char figure = 0) noexcept;

    bool IsCheck(bool to_black_king) const noexcept;
    bool IsMate(bool to_black_king) const noexcept;
    bool IsDraw(bool is_black_move) const noexcept;

private:

    struct PositionRepetitionHash {
        std::size_t operator()(char table[8][8]);
    };

    bool TryMovePawn(bool is_black_move, Coords from, Coords to, char figure = 0) noexcept;
    bool TryMoveKnight(bool is_black_move, Coords from, Coords to) noexcept;
    bool TryMoveBishop(bool is_black_move, Coords from, Coords to) noexcept;
    bool TryMoveRook(bool is_black_move, Coords from, Coords to) noexcept;
    bool TryMoveQueen(bool is_black_move, Coords from, Coords to) noexcept;
    bool TryMoveKing(bool is_black_move, Coords from, Coords to) noexcept;

    bool IsStalemate(bool to_black_king);
    bool IsTripleRepetition();
    bool IsImpossibleToMate();

    bool _is_black_move;
    bool _white_can_kingside_castling;
    bool _white_can_queenside_castling;
    bool _black_can_kingside_castling;
    bool _black_can_queenside_castling;
    Coords _en_passant_square;
    int _moves_without_capture_counter;
    int _moves_counter;
    std::unordered_map<char[8][8], int, PositionRepetitionHash> _position_repetitions;
    char _table[8][8];
};