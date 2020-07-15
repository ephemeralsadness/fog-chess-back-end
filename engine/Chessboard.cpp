#include "Chessboard.h"

#include <map>
#include <sstream>
#include <algorithm>

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
    if (buffer == "w") {
        _current_turn = Color::WHITE;
    } else if (buffer == "b") {
        _current_turn = Color::BLACK;
    }

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

    _was_triple_repetition = false;
    _position_repetitions.emplace(_table, 1);
}


bool Chessboard::MakeMove(Coords from, Coords to, Figure figure_to_place) {
    ColoredFigure& figure_to_move = _table[from.GetRowIndex()][from.GetColIndex()];
    ColoredFigure& figure_to_eat = _table[to.GetRowIndex()][to.GetColIndex()];
    bool is_capture = figure_to_eat.figure != Figure::NOTHING || figure_to_move.figure == Figure::PAWN;

    // проверить ход фигуры
    auto possible_moves = GetMoves(from, true);
    if (std::find(possible_moves.begin(), possible_moves.end(), to) == possible_moves.end()) {
        return false;
    }

    figure_to_eat = figure_to_move;
    figure_to_move.figure = Figure::NOTHING;

    // проверить не возникает ли шаха после хода
    if (IsCheck(_current_turn)) {
        return false;
    }

    // Обработка взятия на проходе
    if (figure_to_eat.figure == Figure::PAWN && abs(from.GetRowIndex() - to.GetRowIndex()) == 2) {
        _en_passant_square.SetRow((static_cast<int>(from.GetRow()) + to.GetRow()) / 2);
        _en_passant_square.SetCol(from.GetCol());
    } else {
        _en_passant_square = {0, 0};
    }

    // Обработка прохода пешки до последней горизонтали
    if (to.GetRow() == '1' || to.GetRow() == '8') {
        figure_to_eat.figure = figure_to_place;
    }

    // увеличить счетчики ходов и передать ход другому игроку
    _current_turn = (_current_turn == Color::WHITE) ? Color::BLACK : Color::WHITE;
    if (is_capture) {
        _moves_without_capture_counter = 0;
        _position_repetitions.clear();
        _position_repetitions.emplace(_table, 1);
    } else {
        ++_moves_without_capture_counter;
        auto it = _position_repetitions.find(_table);
        if (it != _position_repetitions.end()) {
            if (it->second == 2) {
                _was_triple_repetition = true;
            } else {
                ++it->second;
            }
        } else {
            _position_repetitions.emplace(_table, 1);
        }
    }
    ++_moves_counter;

    return true;
}


bool Chessboard::IsCheck(Color to_player) {
    Color enemy_color = (to_player == Color::WHITE) ? Color::BLACK : Color::WHITE;
    auto protected_fields = ProtectedFields(enemy_color);

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (_table[i][j].figure == Figure::KING && _table[i][j].color == to_player) {
                return protected_fields[i][j] != 0;
            }
        }
    }

    return false;
}


std::vector<Coords, std::set<Coords>> Chessboard::AllPossibleMoves();


std::array<std::array<int, 8>, 8> Chessboard::ProtectedFields(Color by_player);


bool Chessboard::NoCheckAfterMove(Coords from, Coords to) {
    ColoredFigure figure_on_from = _table[from.GetRowIndex()][from.GetColIndex()];
    ColoredFigure figure_on_to = _table[from.GetRowIndex()][from.GetColIndex()];


}


std::string Chessboard::GetFOWFen();


Result Chessboard::Result();


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


std::vector<Coords> Chessboard::GetMovesPawn(Coords figure_pos, bool only_possible) {
    int row = figure_pos.GetRowIndex();
    int col = figure_pos.GetColIndex();
    Color figure_color = _table[row][col].color;
    std::vector<Coords> moves;
    if (figure_color == Color::WHITE) {
        if (_table[row][col].figure == Figure::NOTHING) {

        }

        if (col > 0 && _table[row + 1][col - 1].figure != Figure::NOTHING &&
            _table[row + 1][col - 1].color == Color::BLACK) {

        }

        if (col < 7 && _table[row + 1][col + 1].figure != Figure::NOTHING &&
            _table[row + 1][col + 1].color == Color::BLACK) {

        }

        if (row == 2 && _table[row + 2][col].figure == Figure::NOTHING) {

        }

        if (_en_passant_square.GetRow() == row + 1 && abs(_en_passant_square.GetCol() - col) == 1) {
            // push _en_passant_square too
        }

    } else {

    }


}


std::vector<Coords> Chessboard::GetMovesKnight(Coords figure_pos, bool only_possible);


std::vector<Coords> Chessboard::GetMovesBishop(Coords figure_pos, bool only_possible);


std::vector<Coords> Chessboard::GetMovesRook(Coords figure_pos, bool only_possible);


std::vector<Coords> Chessboard::GetMovesQueen(Coords figure_pos, bool only_possible);


std::vector<Coords> Chessboard::GetMovesKing(Coords figure_pos, bool only_possible);


bool Chessboard::IsMate();


bool Chessboard::IsImpossibleToMate();


bool Chessboard::IsStaleMate();


bool Chessboard::IsTripleRepetition();


bool Chessboard::IsFiftyMovesWithoutCapture();