#pragma once

#include "Coords.h"
#include "Figure.h"

#include <string>
#include <array>
#include <vector>
#include <unordered_map>

enum class Result {
    IN_PROGRESS,
    WHITE_WIN,
    DRAW,
    BLACK_WIN
};

class Chessboard {
public:
    Chessboard() noexcept : Chessboard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") { }
    Chessboard(const std::string& fen);

    bool MakeMove(Coords from, Coords to, Figure figure_to_place = Figure::NOTHING);

    std::string GetFOWFen();
    Result Result();
private:
    typedef std::array<std::array<ColoredFigure, 8>, 8> Table;

    bool IsCheck(Color to_player);
    std::vector<Coords> AllPossibleMoves(Coords figure_pos);
    std::array<std::array<int, 8>, 8> ProtectedFields(Color by_player);

    // Ходы разных фигур
    bool MakeMovePawn(Coords from, Coords to, Figure figure_to_place = Figure::NOTHING);
    bool MakeMoveKnight(Coords from, Coords to);
    bool MakeMoveBishop(Coords from, Coords to);
    bool MakeMoveRook(Coords from, Coords to);
    bool MakeMoveQueen(Coords from, Coords to);
    bool MakeMoveKing(Coords from, Coords to);

    // функции для проверки на конец партии
    bool IsMate();
    bool IsImpossibleToMate();
    bool IsStaleMate();
    bool IsTripleRepetition();
    bool IsFiftyMovesWithoutCapture();

    struct TableHash {
        size_t operator() (const Table& table) const noexcept;
    };

    bool _is_black_move;
    bool _white_can_kingside_castling;
    bool _white_can_queenside_castling;
    bool _black_can_kingside_castling;
    bool _black_can_queenside_castling;
    Coords _en_passant_square;
    int _moves_without_capture_counter;
    int _moves_counter;
    std::unordered_map<Table, int, TableHash> _position_repetitions;
    Table _table;
};