#pragma once

#include "Coords.h"
#include "Figure.h"

#include <string>
#include <array>
#include <vector>
#include <set>
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
    explicit Chessboard(const std::string& fen);

    bool MakeMove(Coords from, Coords to, Figure figure_to_place = Figure::NOTHING);
    std::string GetFOWFen();
    enum Result Result();
private:
    typedef std::array<std::array<ColoredFigure, 8>, 8> Table;

    bool IsCheck(Color to_player);
    bool NoCheckAfterMove(Coords from, Coords to, Color to_player);
    std::array<std::array<std::vector<Coords>, 8>, 8> AllPossibleMoves(Color for_player);
    std::array<std::array<std::vector<Coords>, 8>, 8> ProtectedFields(Color by_player);

    std::vector<Coords> GetMoves(Coords figure_pos, bool only_possible);
    std::vector<Coords> GetMovesPawn(Coords figure_pos, bool only_possible);
    std::vector<Coords> GetMovesKnight(Coords figure_pos, bool only_possible);
    std::vector<Coords> GetMovesBishop(Coords figure_pos, bool only_possible);
    std::vector<Coords> GetMovesRook(Coords figure_pos, bool only_possible);
    std::vector<Coords> GetMovesQueen(Coords figure_pos, bool only_possible);
    std::vector<Coords> GetCastlingMoves(Coords figure_pos);
    std::vector<Coords> GetMovesKing(Coords figure_pos, bool only_possible);

    // функции для проверки на конец партии
    bool IsMate();
    bool IsImpossibleToMate();
    bool IsStaleMate();
    bool IsTripleRepetition();
    bool IsFiftyMovesWithoutCapture();

    struct TableHash {
        size_t operator() (const Table& table) const noexcept;
    };

    Color _current_turn;
    bool _white_can_kingside_castling;
    bool _white_can_queenside_castling;
    bool _black_can_kingside_castling;
    bool _black_can_queenside_castling;
    bool _was_triple_repetition;
    std::optional<Coords> _en_passant_square;
    int _moves_without_capture_counter;
    int _moves_counter;
    std::unordered_map<Table, int, TableHash> _position_repetitions;
    Table _table;

public:
    // Debug
    void Print();
};