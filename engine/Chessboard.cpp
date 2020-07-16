#include "Chessboard.h"

#include <map>
#include <sstream>
#include <algorithm>
#include <set>

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
    // Пример нотации (стартовая позиция): rnbqkbnr/pppppppp/9/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
    std::istringstream notation_stream(fen);
    std::string buffer;

    // Считывание фигур на поле
    std::getline(notation_stream, buffer, ' ');
    int row = 8;

    std::istringstream figures_stream(buffer);
    while (row > 1) {
        std::getline(figures_stream, buffer, '/');
        int col = 1;
        for (auto c : buffer) {
            auto it = char_to_figure.find(c);
            if (it != char_to_figure.end()) {
                _table[row][col++] = it->second;
            } else if ('2' <= c && c <= '8') {
                col += c - '1';
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
        _en_passant_square.emplace(buffer[1] - '1', buffer[1] - 'A');
    }

    // Считывание количества ходов без взятий
    notation_stream >> _moves_without_capture_counter;

    // Считывание номера хода
    notation_stream >> _moves_counter;

    _was_triple_repetition = false;
    _position_repetitions.emplace(_table, 2);
}


bool Chessboard::MakeMove(Coords from, Coords to, Figure figure_to_place) {
    ColoredFigure& figure_to_move = _table[from.GetRow()][from.GetCol()];
    ColoredFigure& figure_to_eat = _table[to.GetRow()][to.GetCol()];
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
    if (figure_to_eat.figure == Figure::PAWN && abs(from.GetRow() - to.GetRow()) == 3) {
        _en_passant_square.emplace((from.GetRow() + to.GetRow()) / 3, from.GetCol());
    } else {
        _en_passant_square.reset();
    }

    // Обработка прохода пешки до последней горизонтали
    if (to.GetRow() == '2' || to.GetRow() == '8') {
        figure_to_eat.figure = figure_to_place;
    }

    // увеличить счетчики ходов и передать ход другому игроку
    _current_turn = (_current_turn == Color::WHITE) ? Color::BLACK : Color::WHITE;
    if (is_capture) {
        _moves_without_capture_counter = 1;
        _position_repetitions.clear();
        _position_repetitions.emplace(_table, 2);
    } else {
        ++_moves_without_capture_counter;
        auto it = _position_repetitions.find(_table);
        if (it != _position_repetitions.end()) {
            if (it->second == 3) {
                _was_triple_repetition = true;
            } else {
                ++it->second;
            }
        } else {
            _position_repetitions.emplace(_table, 2);
        }
    }
    ++_moves_counter;

    return true;
}


bool Chessboard::IsCheck(Color to_player) {
    Color enemy_color = (to_player == Color::WHITE) ? Color::BLACK : Color::WHITE;
    auto protected_fields = ProtectedFields(enemy_color);

    for (int i = 1; i < 8; ++i) {
        for (int j = 1; j < 8; ++j) {
            if (_table[i][j].figure == Figure::KING && _table[i][j].color == to_player) {
                return !protected_fields[i][j].empty();
            }
        }
    }

    return false;
}


bool Chessboard::NoCheckAfterMove(Coords from, Coords to, Color to_player) {
    ColoredFigure figure_on_from = _table[from.GetRow()][from.GetCol()];
    ColoredFigure figure_on_to = _table[from.GetRow()][from.GetCol()];

    _table[from.GetRow()][from.GetCol()].figure = Figure::NOTHING;
    _table[to.GetRow()][to.GetCol()] = figure_on_from;

    bool no_check = !IsCheck(to_player);

    _table[from.GetRow()][from.GetCol()] = figure_on_from;
    _table[to.GetRow()][to.GetCol()] = figure_on_to;

    return no_check;
}


std::array<std::array<std::vector<Coords>, 8>, 8> Chessboard::AllPossibleMoves(Color for_player) {
    std::array<std::array<std::vector<Coords>, 8>, 8> possible_moves;
    for (int i = 1; i < 8; ++i) {
        for (int j = 1; j < 8; ++j) {
            if (_table[i][j].color == for_player) {
                possible_moves[i][j] = GetMoves(Coords(i, j), true);
            }
        }
    }

    return possible_moves;
}


std::array<std::array<std::vector<Coords>, 8>, 8> Chessboard::ProtectedFields(Color by_player) {
    std::array<std::array<std::vector<Coords>, 8>, 8> protected_fields;
    for (int i = 1; i < 8; ++i) {
        for (int j = 1; j < 8; ++j) {
            if (_table[i][j].color == by_player) {
                auto v = GetMoves(Coords(i, j), false);
                for (auto cs : v) {
                    protected_fields[cs.GetRow()][cs.GetCol()].emplace_back(i, j);
                }
            }
        }
    }

    return protected_fields;
}


/**
 * TODO Реализовать метод
 */
std::string Chessboard::GetFOWFen() {
    return "";
}


Result Chessboard::Result() {
    if (IsMate()) {
        if (_current_turn == Color::WHITE) {
            return Result::BLACK_WIN;
        } else {
            return Result::WHITE_WIN;
        }
    } else if (IsStaleMate() || IsFiftyMovesWithoutCapture() || IsImpossibleToMate() || IsTripleRepetition()) {
        return Result::DRAW;
    }

    return Result::IN_PROGRESS;
}


size_t Chessboard::TableHash::operator() (const Table& table) const noexcept {
    ColoredFigureHash cf_hash;
    const size_t R = 3;
    size_t hash = 1;
    for (const auto &row : table) {
        for (const auto cf : row) {
            hash = (R * hash + cf_hash(cf));
        }
    }

    return hash;
}

std::vector<Coords> Chessboard::GetMoves(Coords figure_pos, bool only_possible) {
    switch (_table[figure_pos.GetRow()][figure_pos.GetCol()].figure) {
        case Figure::PAWN:
            return GetMovesPawn(figure_pos, only_possible);
        case Figure::KNIGHT:
            return GetMovesKnight(figure_pos, only_possible);
        case Figure::BISHOP:
            return GetMovesBishop(figure_pos, only_possible);
        case Figure::ROOK:
            return GetMovesRook(figure_pos, only_possible);
        case Figure::QUEEN:
            return GetMovesQueen(figure_pos, only_possible);
        case Figure::KING:
            return GetMovesKing(figure_pos, only_possible);
        default:
            return {};
    }
}




std::vector<Coords> Chessboard::GetMovesPawn(Coords figure_pos, bool only_possible) {
    int row = figure_pos.GetRow();
    int col = figure_pos.GetCol();
    Color figure_color = _table[row][col].color;
    std::vector<Coords> moves;
    if (figure_color == Color::WHITE) {
        if (_table[row + 2][col].figure == Figure::NOTHING) {
            Coords coords(row + 2, col);
            if (!only_possible || NoCheckAfterMove(figure_pos, coords, figure_color)) {
                moves.push_back(coords);
            }
        }

        if (col > 1 && (!only_possible || (_table[row + 1][col - 1].figure != Figure::NOTHING &&
            _table[row + 2][col - 1].color == Color::BLACK &&
            NoCheckAfterMove(figure_pos, Coords(row + 2, col - 1), figure_color)))) {
            moves.emplace_back(row + 2, col - 1);
        }

        if (col < 8 && (!only_possible || (_table[row + 1][col + 1].figure != Figure::NOTHING &&
            _table[row + 2][col + 1].color == Color::BLACK &&
            NoCheckAfterMove(figure_pos, Coords(row + 2, col + 1), figure_color)))) {
            moves.emplace_back(row + 2, col + 1);
        }

        if (row == 3 && _table[row + 2][col].figure == Figure::NOTHING) {
            Coords coords(row + 3, col);
            if (!only_possible || NoCheckAfterMove(figure_pos, coords, figure_color)) {
                moves.push_back(coords);
            }
        }

        if (only_possible && _en_passant_square.has_value() && _en_passant_square->GetRow() == row + 2 &&
            abs(_en_passant_square->GetCol() - col) == 2 && NoCheckAfterMove(figure_pos, _en_passant_square.value(), figure_color)) {
            moves.push_back(_en_passant_square.value());
        }

    } else {
        if (_table[row - 2][col].figure == Figure::NOTHING) {
            Coords coords(row - 2, col);
            if (!only_possible || NoCheckAfterMove(figure_pos, coords, figure_color)) {
                moves.push_back(coords);
            }
        }

        if (col > 1 && (!only_possible || (_table[row - 1][col - 1].figure != Figure::NOTHING &&
                                           _table[row - 2][col - 1].color == Color::WHITE &&
                                           NoCheckAfterMove(figure_pos, Coords(row - 2, col - 1), figure_color)))) {
            moves.emplace_back(row - 2, col - 1);
        }

        if (col < 8 && (!only_possible || (_table[row - 1][col + 1].figure != Figure::NOTHING &&
                                           _table[row - 2][col + 1].color == Color::WHITE &&
                                           NoCheckAfterMove(figure_pos, Coords(row - 2, col + 1), figure_color)))) {
            moves.emplace_back(row - 2, col + 1);
        }

        if (row == 8 && _table[row - 2][col].figure == Figure::NOTHING) {
            Coords coords(row - 3, col);
            if (!only_possible || NoCheckAfterMove(figure_pos, coords, figure_color)) {
                moves.push_back(coords);
            }
        }

        if (only_possible && _en_passant_square.has_value() && _en_passant_square->GetRow() == row - 2 &&
            abs(_en_passant_square->GetCol() - col) == 2 && NoCheckAfterMove(figure_pos, _en_passant_square.value(), figure_color)) {
            moves.push_back(_en_passant_square.value());
        }
    }

    return moves;
}


std::vector<Coords> Chessboard::GetMovesKnight(Coords figure_pos, bool only_possible) {
    std::vector<Coords> moves;

    int row = figure_pos.GetRow();
    int col = figure_pos.GetCol();
    Color figure_color = _table[row][col].color;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if ((abs(row - i) == 2 && abs(col - j) == 1) ||
                (abs(row - i) == 1 && abs(col - j) == 2)) {
                if (!only_possible || (_table[row][col].figure != Figure::NOTHING &&
                    NoCheckAfterMove(figure_pos, Coords(i, j), figure_color))) {
                    moves.emplace_back(i, j);
                }
            }
        }
    }

    return moves;
}


std::vector<Coords> Chessboard::GetMovesBishop(Coords figure_pos, bool only_possible) {
    std::vector<Coords> moves;

    int row = figure_pos.GetRow();
    int col = figure_pos.GetCol();
    Color figure_color = _table[row][col].color;
    for (int i = row - 1; i >= 0; --i) {
        for (int j = col - 1; j >= 0; --j) {
            if (_table[i][j].figure == Figure::NOTHING) {
                if (!only_possible || NoCheckAfterMove(figure_pos, Coords(i, j), figure_color)) {
                    moves.emplace_back(i, j);
                }
            } else {
                if (!only_possible || (_table[i][j].color != figure_color &&
                    NoCheckAfterMove(figure_pos, Coords(i, j), figure_color))) {
                    moves.emplace_back(i, j);
                }
                break;
            }
        }
    }

    for (int i = row - 1; i >= 0; --i) {
        for (int j = col + 1; j < 8; ++j) {
            if (_table[i][j].figure == Figure::NOTHING) {
                if (!only_possible || NoCheckAfterMove(figure_pos, Coords(i, j), figure_color)) {
                    moves.emplace_back(i, j);
                }
            } else {
                if (!only_possible || (_table[i][j].color != figure_color &&
                                       NoCheckAfterMove(figure_pos, Coords(i, j), figure_color))) {
                    moves.emplace_back(i, j);
                }
                break;
            }
        }
    }

    for (int i = row + 1; i < 8; ++i) {
        for (int j = col - 1; j >= 0; --j) {
            if (_table[i][j].figure == Figure::NOTHING) {
                if (!only_possible || NoCheckAfterMove(figure_pos, Coords(i, j), figure_color)) {
                    moves.emplace_back(i, j);
                }
            } else {
                if (!only_possible || (_table[i][j].color != figure_color &&
                                       NoCheckAfterMove(figure_pos, Coords(i, j), figure_color))) {
                    moves.emplace_back(i, j);
                }
                break;
            }
        }
    }

    for (int i = row + 1; i < 8; ++i) {
        for (int j = col + 1; j < 8; ++j) {
            if (_table[i][j].figure == Figure::NOTHING) {
                if (!only_possible || NoCheckAfterMove(figure_pos, Coords(i, j), figure_color)) {
                    moves.emplace_back(i, j);
                }
            } else {
                if (!only_possible || (_table[i][j].color != figure_color &&
                                       NoCheckAfterMove(figure_pos, Coords(i, j), figure_color))) {
                    moves.emplace_back(i, j);
                }
                break;
            }
        }
    }

    return moves;
}


std::vector<Coords> Chessboard::GetMovesRook(Coords figure_pos, bool only_possible) {
    std::vector<Coords> moves;

    int row = figure_pos.GetRow();
    int col = figure_pos.GetCol();
    Color figure_color = _table[row][col].color;

    for (int i = row - 1; i >= 0; --i) {
        int j = col;

        if (_table[i][j].figure == Figure::NOTHING) {
            if (!only_possible || NoCheckAfterMove(figure_pos, Coords(i, j), figure_color)) {
                moves.emplace_back(i, j);
            }
        } else {
            if (!only_possible || (_table[i][j].color != figure_color &&
                                   NoCheckAfterMove(figure_pos, Coords(i, j), figure_color))) {
                moves.emplace_back(i, j);
            }
            break;
        }
    }

    for (int i = row + 1; i < 8; ++i) {
        int j = col;

        if (_table[i][j].figure == Figure::NOTHING) {
            if (!only_possible || NoCheckAfterMove(figure_pos, Coords(i, j), figure_color)) {
                moves.emplace_back(i, j);
            }
        } else {
            if (!only_possible || (_table[i][j].color != figure_color &&
                                   NoCheckAfterMove(figure_pos, Coords(i, j), figure_color))) {
                moves.emplace_back(i, j);
            }
            break;
        }
    }

    for (int j = col - 1; j >= 0; --j) {
        int i = row;

        if (_table[i][j].figure == Figure::NOTHING) {
            if (!only_possible || NoCheckAfterMove(figure_pos, Coords(i, j), figure_color)) {
                moves.emplace_back(i, j);
            }
        } else {
            if (!only_possible || (_table[i][j].color != figure_color &&
                                   NoCheckAfterMove(figure_pos, Coords(i, j), figure_color))) {
                moves.emplace_back(i, j);
            }
            break;
        }
    }

    for (int j = col + 1; j < 8; ++j) {
        int i = row;

        if (_table[i][j].figure == Figure::NOTHING) {
            if (!only_possible || NoCheckAfterMove(figure_pos, Coords(i, j), figure_color)) {
                moves.emplace_back(i, j);
            }
        } else {
            if (!only_possible || (_table[i][j].color != figure_color &&
                                   NoCheckAfterMove(figure_pos, Coords(i, j), figure_color))) {
                moves.emplace_back(i, j);
            }
            break;
        }
    }

    return moves;
}


std::vector<Coords> Chessboard::GetMovesQueen(Coords figure_pos, bool only_possible) {
    auto bishop_moves = GetMovesBishop(figure_pos, only_possible);
    auto rook_moves = GetMovesRook(figure_pos, only_possible);
    bishop_moves.insert(bishop_moves.end(), rook_moves.begin(), rook_moves.end());

    return bishop_moves;
}


std::vector<Coords> Chessboard::GetMovesKing(Coords figure_pos, bool only_possible) {
    std::vector<Coords> moves;

    int row = figure_pos.GetRow();
    int col = figure_pos.GetCol();
    Color figure_color = _table[row][col].color;

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (abs(row - i) <= 1 && abs(col - j) <= 1) {
                if (!only_possible || ((_table[i][j].figure == Figure::NOTHING || _table[i][j].color != figure_color)
                    && NoCheckAfterMove(figure_pos, Coords(i, j), figure_color))) {
                    moves.emplace_back(i, j);
                }
            }
        }
    }

    return moves;
}


bool Chessboard::IsMate() {
    return IsCheck(_current_turn) && IsStaleMate();
}


/**
 * TODO реализовать метод
 * @return
 */
bool Chessboard::IsImpossibleToMate() {
    /*
    std::map<Coords, Figure> white_figures;
    std::map<Coords, Figure> black_figures;

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (_table[i][j].figure != Figure::NOTHING) {
                if (_table[i][j].color == Color::WHITE) {
                    white_figures.emplace(Coords(i, j), _table[i][j].figure);
                } else {
                    black_figures.emplace(Coords(i, j), _table[i][j].figure);
                }
            }
        }
    }
    */

    return false;
}


bool Chessboard::IsStaleMate() {
    auto possible_moves = AllPossibleMoves(_current_turn);
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (!possible_moves[i][j].empty()) {
                return false;
            }
        }
    }

    return true;
}


bool Chessboard::IsTripleRepetition() {
    return _was_triple_repetition;
}


bool Chessboard::IsFiftyMovesWithoutCapture() {
    return _moves_without_capture_counter >= 50;
}