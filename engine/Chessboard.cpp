#include "Chessboard.h"

#include <map>
#include <sstream>

std::map<char, ColoredFigure> char_to_figure = {
        {'P', {Color::WHITE, Figure::PAWN}},
        {'N', {Color::WHITE, Figure::KNIGHT}},
        {'B', {Color::WHITE, Figure::BISHOP}},
        {'R', {Color::WHITE, Figure::ROOK}},
        {'Q', {Color::WHITE, Figure::QUEEN}},
        {'K', {Color::WHITE, Figure::KING}},
        {'p', {Color::BLACK, Figure::PAWN}},
        {'n', {Color::BLACK, Figure::KNIGHT}},
        {'b', {Color::BLACK, Figure::BISHOP}},
        {'r', {Color::BLACK, Figure::ROOK}},
        {'q', {Color::BLACK, Figure::QUEEN}},
        {'k', {Color::BLACK, Figure::KING}},
};


Chessboard::Chessboard(const std::string& fen) {
    // Пример нотации (стартовая позиция): rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
    std::istringstream notation_stream(fen);
    std::string buffer;

    // Считывание фигур на поле
    std::getline(notation_stream, buffer, ' ');
    int row = 7;

    std::istringstream figures_stream(buffer);
    while (row > 0) {
        std::getline(figures_stream, buffer, '/');
        int col = 0;
        for (auto c : buffer) {
            auto it = char_to_figure.find(c);
            if (it != char_to_figure.end()) {
                _table[row][col++] = it->second;
            } else if ('1' <= c && c <= '8') {
                col += c - '0';
            }
        }
        --row;
    }

    // Считывание игрока, обладающего первым ходом
    std::getline(notation_stream, buffer, ' ');
    _is_black_move = (buffer == "b");

    // Считывание права на рокировки обоих игроков
    std::getline(notation_stream, buffer, ' ');
    _white_can_kingside_castling = false;
    _white_can_queenside_castling = false;
    _black_can_kingside_castling = false;
    _black_can_queenside_castling = false;
    if (buffer != "-") {
        for (auto c : buffer) {
            switch (c) {
                case 'K': _white_can_kingside_castling = true; break;
                case 'Q': _white_can_queenside_castling = true; break;
                case 'k': _black_can_kingside_castling = true; break;
                case 'q': _black_can_queenside_castling = true; break;
            }
        }
    }

    // Считывание поля, по которому можно произвести взятие на проходе
    std::getline(notation_stream, buffer, ' ');
    if (buffer != "-") {
        _en_passant_square.SetRow(buffer[0]);
        _en_passant_square.SetCol(buffer[1]);
    }

    // Считывание количества ходов без взятий
    notation_stream >> _moves_without_capture_counter;

    // Считывание номера хода
    notation_stream >> _moves_counter;

}


bool MakeMove(Coords from, Coords to, Figure figure_to_place = Figure::NOTHING) {
    if ()
    // проверить что from и to не фигуры одного цвета
    // проверить ход фигуры
    // проверить не возникает ли шаха после хода
}

bool IsCheck(Color to_player);
std::string GetFOWFen();
Result Result();

size_t Chessboard::TableHash::operator() (const Table& table) const noexcept {
    ColoredFigureHash cf_hash;
    const size_t R = 2;
    size_t hash = 0;
    for (const auto &row : table) {
        for (const auto cf : row) {
            hash = (R * hash + cf_hash(cf));
        }
    }

    return hash;
}

bool MakeMovePawn(Coords from, Coords to, Figure figure_to_place = Figure::NOTHING);
bool MakeMoveKnight(Coords from, Coords to);
bool MakeMoveBishop(Coords from, Coords to);
bool MakeMoveRook(Coords from, Coords to);
bool MakeMoveQueen(Coords from, Coords to);
bool MakeMoveKing(Coords from, Coords to);

bool IsMate();
bool IsImpossibleToMate();
bool IsStaleMate();
bool IsTripleRepetition();
bool IsFiftyMovesWithoutCapture();