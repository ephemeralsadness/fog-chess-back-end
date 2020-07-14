#include "Chessboard.h"

#include <sstream>
#include <set>
#include <vector>

std::set<char> figures = {'p', 'r', 'n', 'b', 'q', 'k', 'P', 'R', 'N', 'B', 'Q', 'K'};


/**
 * Парсинг доски из нотации Форсайта - Эдвардса (смотри вики)
 * При некорректных данных программа может завершиться аварийно!
 */
Chessboard::Chessboard(const std::string& fen) noexcept {
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
            if (figures.count(c)) {
                _table[row][col++] = c;
            } else if ('1' <= c && c <= '8') {
                for (int k = '0'; k < c; ++k) {
                    _table[row][col++] = 0;
                }
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

/**
 * Возвращает символ фигуры, размещенной по координатам coords
 */
char Chessboard::At(Coords coords) const noexcept {
    if (coords.GetRow() == 0 || coords.GetCol() == 0) {
        return 0;
    }
    return _table[coords.GetRowIndex()][coords.GetColIndex()];
}

/**
 * Делает ход, и, в случае, если ход корректный, возвращает true.
 * @param is_black_move true, если ход делается черными. Если белыми - false.
 * @param from позиция фигуры до совершения хода
 * @param to позиция фигуры после хода
 * @param figure фигура, в которую превращается пешка при достижении последнего ряда
 */
bool Chessboard::MakeMove(bool is_black_move, Coords from, Coords to, char figure) noexcept {
    bool move_correctness = false;

    char c = std::tolower(At(from));
    switch (c) {
        case 'p': move_correctness = TryMovePawn(is_black_move, from, to, figure); break;
        case 'n': move_correctness = TryMoveKnight(is_black_move, from, to); break;
        case 'b': move_correctness = TryMoveBishop(is_black_move, from, to); break;
        case 'r': move_correctness = TryMoveRook(is_black_move, from, to); break;
        case 'q': move_correctness = TryMoveQueen(is_black_move, from, to); break;
        case 'k': move_correctness = TryMoveKing(is_black_move, from, to); break;
    }

    return move_correctness;
}

/**
 * Возвращает true, если фигура враждебная для игрока (играющего черными, если for_black - true)
 */
bool IsEnemy(char figure, bool for_black) {
    if (for_black) {
       return 'A' <= figure && figure <= 'Z';
    }
    return 'a' <= figure && figure <= 'z';
}

bool Chessboard::IsCheck(const bool to_black_king) const noexcept {
    Coords king_pos;

    // find the king
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if ((to_black_king && _table[row][col] == 'k') ||
                !to_black_king && _table[row][col] == 'K') {
                king_pos.SetRow(row + '1'); king_pos.SetCol(col + 'A');
                break;
            }
        }
    }

    // check pawns
    {
        int player_side = to_black_king ? -1 : 1;
        char candidate_left = At(Coords(king_pos.GetRow() + player_side, king_pos.GetCol() - 1));
        char candidate_right = At(Coords(king_pos.GetRow() + player_side, king_pos.GetCol() + 1));
        if (std::tolower(candidate_left) == 'p' && IsEnemy(candidate_left, to_black_king) ||
            std::tolower(candidate_right) == 'p' && IsEnemy(candidate_right, to_black_king)) {
            return true;
        }
    }

    // check knights
    {
        std::vector<char> candidates = {
                At(Coords(king_pos.GetRow() - 2, king_pos.GetCol() - 1)),
                At(Coords(king_pos.GetRow() - 2, king_pos.GetCol() + 1)),
                At(Coords(king_pos.GetRow() - 1, king_pos.GetCol() - 2)),
                At(Coords(king_pos.GetRow() - 1, king_pos.GetCol() + 2)),
                At(Coords(king_pos.GetRow() + 1, king_pos.GetCol() - 2)),
                At(Coords(king_pos.GetRow() + 1, king_pos.GetCol() + 2)),
                At(Coords(king_pos.GetRow() + 2, king_pos.GetCol() - 1)),
                At(Coords(king_pos.GetRow() + 2, king_pos.GetCol() + 1)),
        };

        for (char figure : candidates) {
            if (std::tolower(figure) == 'n' && IsEnemy(figure, to_black_king)) {
                return true;
            }
        }

    }

    // check diagonals (bishops and queens)
    {
        int king_row = king_pos.GetRowIndex();
        int king_col = king_pos.GetColIndex();

        // check left top diagonal to the king
        {
            int i = king_row + 1;
            int j = king_row - 1;
            while (i < 8 && j > 0) {
                char figure = _table[i][j];
                if (figure != 0) {
                    if ((std::tolower(figure) == 'q' || std::tolower(figure) == 'b') &&
                        IsEnemy(figure, to_black_king)) {
                        return true;
                    }
                    break;
                }
                ++i;
                --j;
            }
        }


        // check right top diagonal to the king
        {
            int i = king_row + 1;
            int j = king_row + 1;
            while (i < 8 && j > 0) {
                char figure = _table[i][j];
                if (figure != 0) {
                    if ((std::tolower(figure) == 'q' || std::tolower(figure) == 'b') &&
                        IsEnemy(figure, to_black_king)) {
                        return true;
                    }
                    break;
                }
                ++i;
                ++j;
            }
        }

        // check left bottom diagonal to the king
        {
            int i = king_row - 1;
            int j = king_row - 1;
            while (i < 8 && j > 0) {
                char figure = _table[i][j];
                if (figure != 0) {
                    if ((std::tolower(figure) == 'q' || std::tolower(figure) == 'b') &&
                        IsEnemy(figure, to_black_king)) {
                        return true;
                    }
                    break;
                }
                --i;
                --j;
            }
        }

        // check right bottom diagonal to the king
        {
            int i = king_row - 1;
            int j = king_row + 1;
            while (i < 8 && j > 0) {
                char figure = _table[i][j];
                if (figure != 0) {
                    if ((std::tolower(figure) == 'q' || std::tolower(figure) == 'b') &&
                        IsEnemy(figure, to_black_king)) {
                        return true;
                    }
                    break;
                }
                --i;
                ++j;
            }
        }

    }


    // check lines (rooks and queens)
    {
        int king_row = king_pos.GetRowIndex();
        int king_col = king_pos.GetColIndex();

        {
            for (int i = king_row; i < 8; ++i) {
                char figure = _table[i][king_col];
                if (figure != 0) {
                    if ((std::tolower(figure) == 'q' || std::tolower(figure) == 'r') &&
                        IsEnemy(figure, to_black_king)) {
                        return true;
                    }
                    break;
                }
            }
        }

        {
            for (int i = king_row; i >= 0; --i) {
                char figure = _table[i][king_col];
                if (figure != 0) {
                    if ((std::tolower(figure) == 'q' || std::tolower(figure) == 'r') &&
                        IsEnemy(figure, to_black_king)) {
                        return true;
                    }
                    break;
                }
            }
        }

        {
            for (int j = king_col; j < 8; ++j) {
                char figure = _table[king_row][j];
                if (figure != 0) {
                    if ((std::tolower(figure) == 'q' || std::tolower(figure) == 'r') &&
                        IsEnemy(figure, to_black_king)) {
                        return true;
                    }
                    break;
                }
            }
        }

        {
            for (int j = king_col; j >= 0; --j) {
                char figure = _table[king_row][j];
                if (figure != 0) {
                    if ((std::tolower(figure) == 'q' || std::tolower(figure) == 'r') &&
                        IsEnemy(figure, to_black_king)) {
                        return true;
                    }
                    break;
                }
            }
        }

    }

    return false;
}

bool Chessboard::IsMate(bool to_black_king) const noexcept;
bool Chessboard::IsDraw(bool is_black_move) const noexcept;

std::size_t Chessboard::PositionRepetitionHash::operator()(char table[8][8]) {

}

bool Chessboard::TryMovePawn(bool is_black_move, Coords from, Coords to, char figure) noexcept {

}


bool Chessboard::TryMoveKnight(bool is_black_move, Coords from, Coords to) noexcept;
bool Chessboard::TryMoveBishop(bool is_black_move, Coords from, Coords to) noexcept;
bool Chessboard::TryMoveRook(bool is_black_move, Coords from, Coords to) noexcept;
bool Chessboard::TryMoveQueen(bool is_black_move, Coords from, Coords to) noexcept;
bool Chessboard::TryMoveKing(bool is_black_move, Coords from, Coords to) noexcept;

bool Chessboard::IsStalemate(bool to_black_king);
bool Chessboard::IsThreeRepetition();
bool Chessboard::IsImpossibleToMate();
