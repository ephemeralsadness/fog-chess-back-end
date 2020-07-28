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

std::unordered_map<ColoredFigure, char, ColoredFigureHash> figure_to_char = {
        {{Color::WHITE, Figure::PAWN},   'P'},
        {{Color::WHITE, Figure::KNIGHT}, 'N'},
        {{Color::WHITE, Figure::BISHOP}, 'B'},
        {{Color::WHITE, Figure::ROOK},   'R'},
        {{Color::WHITE, Figure::QUEEN},  'Q'},
        {{Color::WHITE, Figure::KING},   'K'},
        {{Color::BLACK, Figure::PAWN},   'p'},
        {{Color::BLACK, Figure::KNIGHT}, 'n'},
        {{Color::BLACK, Figure::BISHOP}, 'b'},
        {{Color::BLACK, Figure::ROOK},   'r'},
        {{Color::BLACK, Figure::QUEEN},  'q'},
        {{Color::BLACK, Figure::KING},   'k'}
};


Chessboard::Chessboard(const std::string &fen) {
    // Пример нотации (стартовая позиция): rnbqkbnr/pppppppp/9/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
    std::istringstream notation_stream(fen);
    std::string buffer;

    // Считывание фигур на поле
    std::getline(notation_stream, buffer, ' ');
    int row = 7;

    std::istringstream figures_stream(buffer);
    while (row >= 0) {
        std::getline(figures_stream, buffer, '/');
        int col = 0;
        for (auto c : buffer) {
            auto it = char_to_figure.find(c);
            if (it != char_to_figure.end()) {
                _table[row][col++] = it->second;
            } else if ('1' <= c && c <= '8') {
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
                case 'K':
                    _white_can_kingside_castling = true;
                    break;
                case 'Q':
                    _white_can_queenside_castling = true;
                    break;
                case 'k':
                    _black_can_kingside_castling = true;
                    break;
                case 'q':
                    _black_can_queenside_castling = true;
                    break;
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

    result_cache = Result();
}


const Table& Chessboard::GetTable() const {
    return _table;
}


bool Chessboard::MakeMove(Coords from, Coords to, Figure figure_to_place) {
    ColoredFigure &figure_to_move = _table[from.GetRow()][from.GetCol()];

    if (figure_to_move.color != _current_turn) {
        return false;
    }

    ColoredFigure &figure_to_eat = _table[to.GetRow()][to.GetCol()];
    bool is_capture = figure_to_eat.figure != Figure::NOTHING || figure_to_move.figure == Figure::PAWN;

    // проверить ход фигуры
    auto possible_moves = GetMoves(from, true);
    if (std::find(possible_moves.begin(), possible_moves.end(), to) == possible_moves.end()) {
        return false;
    }

    if (figure_to_move.figure == Figure::PAWN && to == _en_passant_square) {
        if (figure_to_move.color == Color::WHITE) {
            _table[to.GetRow() - 1][to.GetCol()].figure = Figure::NOTHING;
        } else if (figure_to_move.color == Color::BLACK) {
            _table[to.GetRow() + 1][to.GetCol()].figure = Figure::NOTHING;
        }
    } else if (figure_to_move.figure == Figure::KING && abs(from.GetCol() - to.GetCol()) == 2) {
        if (to.GetCol() > from.GetCol()) {
            std::swap(_table[from.GetRow()][7], _table[from.GetRow()][5]);
        } else {
            std::swap(_table[from.GetRow()][0], _table[from.GetRow()][3]);
        }
    }
    figure_to_eat = figure_to_move;
    figure_to_move.figure = Figure::NOTHING;

    if (figure_to_eat.figure == Figure::KING) {
        if (_current_turn == Color::WHITE) {
            _white_can_kingside_castling = false;
            _white_can_queenside_castling = false;
        } else {
            _black_can_kingside_castling = false;
            _black_can_queenside_castling = false;
        }
    } else if (to.GetRow() == 0 && to.GetCol() == 0) {
        _white_can_queenside_castling = false;
    } else if (to.GetRow() == 0 && to.GetCol() == 7) {
        _white_can_kingside_castling = false;
    } else if (to.GetRow() == 7 && to.GetCol() == 0) {
        _black_can_queenside_castling = false;
    } else if (to.GetRow() == 7 && to.GetCol() == 7) {
        _black_can_kingside_castling = false;
    }

    // Обработка взятия на проходе
    if (figure_to_eat.figure == Figure::PAWN && abs(from.GetRow() - to.GetRow()) == 2) {
        _en_passant_square.emplace((from.GetRow() + to.GetRow()) / 2, from.GetCol());
    } else {
        _en_passant_square.reset();
    }

    // Обработка прохода пешки до последней горизонтали
    if (figure_to_eat.figure == Figure::PAWN && (to.GetRow() == 0 || to.GetRow() == 7)) {
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

    result_cache = Result();
    return true;
}


enum Color Chessboard::GetCurrentTurn() {
    return _current_turn;
}

bool Chessboard::IsCheck(Color to_player) {
    Color enemy_color = (to_player == Color::WHITE) ? Color::BLACK : Color::WHITE;
    auto protected_fields = ProtectedFields(enemy_color);

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (_table[i][j].figure == Figure::KING && _table[i][j].color == to_player) {
                return !protected_fields[i][j].empty();
            }
        }
    }

    return false;
}


bool Chessboard::NoCheckAfterMove(Coords from, Coords to, Color to_player) {
    ColoredFigure figure_on_from = _table[from.GetRow()][from.GetCol()];
    ColoredFigure figure_on_to = _table[to.GetRow()][to.GetCol()];

    _table[from.GetRow()][from.GetCol()].figure = Figure::NOTHING;
    _table[to.GetRow()][to.GetCol()] = figure_on_from;

    bool no_check = !IsCheck(to_player);

    _table[from.GetRow()][from.GetCol()] = figure_on_from;
    _table[to.GetRow()][to.GetCol()] = figure_on_to;

    return no_check;
}


std::array<std::array<std::vector<Coords>, 8>, 8> Chessboard::AllPossibleMoves(Color for_player) {
    std::array<std::array<std::vector<Coords>, 8>, 8> possible_moves;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (_table[i][j].color == for_player) {
                possible_moves[i][j] = GetMoves(Coords(i, j), true);
            }
        }
    }

    return possible_moves;
}


std::array<std::array<std::vector<Coords>, 8>, 8> Chessboard::ProtectedFields(Color by_player) {
    std::array<std::array<std::vector<Coords>, 8>, 8> protected_fields;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
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
std::string Chessboard::GetFOWFen(Color for_player) {
    // Пример нотации (стартовая позиция): rnbqkbnr/pppppppp/9/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
    //                                   :
    std::ostringstream stream;
    std::array<std::array<bool, 8>, 8> mask;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            mask[i][j] = false;
        }
    }

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (_table[i][j].figure != Figure::NOTHING && _table[i][j].color == for_player) {
                mask[i][j] = true;
                for (int i0 = std::max(0, i - 1); i0 <= std::min(7, i + 1); ++i0)
                    for (int j0 = std::max(0, j - 1); j0 <= std::min(7, j + 1); ++j0)
                        mask[i0][j0] = true;
            }
        }
    }


    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (_table[i][j].color == for_player) {
                auto v = GetMoves(Coords(i, j), false);
                for (auto cs : v) {
                    mask[cs.GetRow()][cs.GetCol()] = true;
                }
                if (_table[i][j].figure == Figure::PAWN && for_player == Color::WHITE && i == 1) {
                    mask[i + 2][j] = true;
                } else if (_table[i][j].figure == Figure::PAWN && for_player == Color::BLACK && i == 6) {
                    mask[i - 2][j] = true;
                }
            }
        }
    }


    auto protected_fields = ProtectedFields(for_player);
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (!protected_fields[i][j].empty()) {
                mask[i][j] = true;
            }
        }
    }

    Coords king_pos;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (_table[i][j].figure == Figure::KING && _table[i][j].color == for_player) {
                king_pos.SetRow(i); king_pos.SetCol(j);
            }
        }
    }

    Color enemy_color = (for_player == Color::WHITE ? Color::BLACK : Color::WHITE);
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if ((_table[i][j].figure == Figure::ROOK || _table[i][j].figure == Figure::QUEEN) && _table[i][j].color != for_player) {
                if (i == king_pos.GetRow() || j == king_pos.GetCol()) {
                    mask[i][j] = true;
                }
            }
        }
    }

    auto enemy_protected_fields = ProtectedFields(enemy_color);
    for (auto cds : enemy_protected_fields[king_pos.GetRow()][king_pos.GetCol()]) {
        mask[cds.GetRow()][cds.GetCol()] = true;
    }

    auto castling_moves = GetCastlingMoves(king_pos);
    for (auto cds : castling_moves) {
        for (auto enemy_figure_pos : enemy_protected_fields[cds.GetRow()][cds.GetCol()]) {
            mask[enemy_figure_pos.GetRow()][enemy_figure_pos.GetCol()] = true;
        }
    }

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if ((_table[i][j].figure == Figure::BISHOP || _table[i][j].figure == Figure::QUEEN) && _table[i][j].color != for_player) {
                if ((i - j == king_pos.GetRow() - king_pos.GetCol()) || (i + j == king_pos.GetRow() + king_pos.GetCol())) {
                    mask[i][j] = true;
                }
            }
        }
    }

    // + if visible, - if not visible
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (!mask[i][j]) {
                stream << '-';
            } else if (_table[i][j].figure == Figure::NOTHING) {
                stream << '+';
            } else {
                stream << figure_to_char.at(_table[i][j]);
            }
        }
    }

    stream << ' ';
    stream << (_current_turn == Color::WHITE ? 'w' : 'b');
    stream << ' ';
    if (_white_can_kingside_castling)
        stream << 'K';
    if (_white_can_queenside_castling)
        stream << 'Q';
    if (_black_can_kingside_castling)
        stream << 'k';
    if (_black_can_queenside_castling)
        stream << 'q';
    stream << ' ';
    if (_en_passant_square.has_value())
        stream << static_cast<char>('A' + _en_passant_square->GetCol())
               << static_cast<char>('1' + _en_passant_square->GetRow());
    else
        stream << '-';
    stream << ' ';
    stream << _moves_without_capture_counter;
    stream << ' ';
    stream << _moves_counter;

    return stream.str();
}


Result Chessboard::Result() {
    if (IsMate()) {
        if (_current_turn == Color::WHITE) {
            return Result::BLACK_WIN;
        } else {
            return Result::WHITE_WIN;
        }
    } else if (IsStaleMate() || IsFiftyMovesWithoutCapture() || IsTripleRepetition()) {
        return Result::DRAW;
    }

    return Result::IN_PROGRESS;
}


size_t Chessboard::TableHash::operator()(const Table &table) const noexcept {
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
        if (only_possible && _table[row + 1][col].figure == Figure::NOTHING) {
            Coords coords(row + 1, col);
            if (!only_possible || NoCheckAfterMove(figure_pos, coords, figure_color)) {
                moves.push_back(coords);
            }
        }

        if (col > 0 && (!only_possible || (_table[row + 1][col - 1].figure != Figure::NOTHING &&
                                           _table[row + 1][col - 1].color == Color::BLACK &&
                                           NoCheckAfterMove(figure_pos, Coords(row + 1, col - 1), figure_color)))) {
            moves.emplace_back(row + 1, col - 1);
        }

        if (col < 7 && (!only_possible || (_table[row + 1][col + 1].figure != Figure::NOTHING &&
                                           _table[row + 1][col + 1].color == Color::BLACK &&
                                           NoCheckAfterMove(figure_pos, Coords(row + 1, col + 1), figure_color)))) {
            moves.emplace_back(row + 1, col + 1);
        }

        if (only_possible && row == 1 && _table[row + 2][col].figure == Figure::NOTHING) {
            Coords coords(row + 2, col);
            if (!only_possible || NoCheckAfterMove(figure_pos, coords, figure_color)) {
                moves.push_back(coords);
            }
        }

        if (only_possible && _en_passant_square.has_value() &&
            _table[_en_passant_square->GetRow()][_en_passant_square->GetCol()].figure == Figure::NOTHING &&
            _en_passant_square->GetRow() == row + 1 && abs(_en_passant_square->GetCol() - col) == 1) {

            _table[_en_passant_square->GetRow() - 1][_en_passant_square->GetCol()].figure = Figure::NOTHING;
            if (NoCheckAfterMove(figure_pos, _en_passant_square.value(), figure_color)) {
                moves.push_back(_en_passant_square.value());
            }
            _table[_en_passant_square->GetRow() - 1][_en_passant_square->GetCol()] = ColoredFigure(Color::BLACK, Figure::PAWN);
        }

    } else {
        if (only_possible && _table[row - 1][col].figure == Figure::NOTHING) {
            Coords coords(row - 1, col);
            if (!only_possible || NoCheckAfterMove(figure_pos, coords, figure_color)) {
                moves.push_back(coords);
            }
        }

        if (col > 0 && (!only_possible || (_table[row - 1][col - 1].figure != Figure::NOTHING &&
                                           _table[row - 1][col - 1].color == Color::WHITE &&
                                           NoCheckAfterMove(figure_pos, Coords(row - 1, col - 1), figure_color)))) {
            moves.emplace_back(row - 1, col - 1);
        }

        if (col < 7 && (!only_possible || (_table[row - 1][col + 1].figure != Figure::NOTHING &&
                                           _table[row - 1][col + 1].color == Color::WHITE &&
                                           NoCheckAfterMove(figure_pos, Coords(row - 1, col + 1), figure_color)))) {
            moves.emplace_back(row - 1, col + 1);
        }

        if (only_possible && row == 6 && _table[row - 2][col].figure == Figure::NOTHING) {
            Coords coords(row - 2, col);
            if (!only_possible || NoCheckAfterMove(figure_pos, coords, figure_color)) {
                moves.push_back(coords);
            }
        }

        if (only_possible && _en_passant_square.has_value() &&
            _table[_en_passant_square->GetRow()][_en_passant_square->GetCol()].figure == Figure::NOTHING &&
            _en_passant_square->GetRow() == row - 1 && abs(_en_passant_square->GetCol() - col) == 1) {

            _table[_en_passant_square->GetRow() + 1][_en_passant_square->GetCol()].figure = Figure::NOTHING;
            if (NoCheckAfterMove(figure_pos, _en_passant_square.value(), figure_color)) {
                moves.push_back(_en_passant_square.value());
            }
            _table[_en_passant_square->GetRow() + 1][_en_passant_square->GetCol()] = ColoredFigure(Color::WHITE, Figure::PAWN);
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
                if (!only_possible || ((_table[i][j].figure == Figure::NOTHING ||
                                        _table[i][j].color != figure_color) &&
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
    for (int i = row - 1, j = col - 1; i >= 0 && j >= 0; --i, --j) {
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

    for (int i = row - 1, j = col + 1; i >= 0 && j < 8; --i, ++j) {
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

    for (int i = row + 1, j = col - 1; i < 8 && j >= 0; ++i, --j) {
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

    for (int i = row + 1, j = col + 1; i < 8 && j < 8; ++i, ++j) {
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


std::vector<Coords> Chessboard::GetCastlingMoves(Coords figure_pos) {
    std::vector<Coords> moves;

    Color figure_color = _table[figure_pos.GetRow()][figure_pos.GetCol()].color;
    Color enemy_color = figure_color == Color::WHITE ? Color::BLACK : Color::WHITE;

    auto attacked_fields = ProtectedFields(enemy_color);

    bool first_alpha = true;
    bool second_alpha = true;

    if ((figure_color == Color::WHITE && _white_can_kingside_castling) ||
        (figure_color == Color::BLACK && _black_can_kingside_castling)) {

        int row = figure_pos.GetRow();
        int col = figure_pos.GetCol();

        for (int i = col + 1; i < 7; ++i) {
            if (_table[row][i].figure != Figure::NOTHING) {
                first_alpha = false;
            }
        }

        if (first_alpha && attacked_fields[row][col].empty() && attacked_fields[row][col + 1].empty() &&
            attacked_fields[row][col + 2].empty()) {
            moves.emplace_back(row, col + 2);
        }

    }

    if ((figure_color == Color::WHITE && _white_can_queenside_castling) ||
        (figure_color == Color::BLACK && _black_can_queenside_castling)) {

        int row = figure_pos.GetRow();
        int col = figure_pos.GetCol();

        for (int i = col - 1; i > 0; --i) {
            if (_table[row][i].figure != Figure::NOTHING) {
                second_alpha = false;
            }
        }

        if (second_alpha && attacked_fields[row][col].empty() && attacked_fields[row][col - 1].empty() &&
            attacked_fields[row][col - 2].empty()) {
            moves.emplace_back(row, col - 2);
        }

    }

    return moves;
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

    if (only_possible) {
        auto castling_moves = GetCastlingMoves(figure_pos);
        moves.insert(moves.end(), castling_moves.begin(), castling_moves.end());
    }

    return moves;
}


bool Chessboard::IsMate() {
    return IsCheck(_current_turn) && IsStaleMate();
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

// Debug
void Chessboard::Print() {
    std::cout << "Now " << (_current_turn == Color::WHITE ? "white" : "black") << " moves" << std::endl;

    for (int i = 7; i >= 0; --i) {
        std::cout << "\033[34m" << std::to_string(i + 1) << "\033[0m ";
        for (int j = 0; j < 8; ++j) {
            if (_table[i][j].figure == Figure::NOTHING) {
                std::cout << '-';
            } else {
                std::cout << figure_to_char.at(_table[i][j]);
            }
            std::cout << ' ';
        }
        std::cout << std::endl;
    }

    std::cout << "  ";
    for (char i = 'A'; i <= 'H'; ++i) {
        std::cout << "\033[34m" << i << "\033[0m ";
    }
    std::cout << std::endl;
}
