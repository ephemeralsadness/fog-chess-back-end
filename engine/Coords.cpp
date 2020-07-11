#include "Coords.h"

bool CheckRow(char row) noexcept {
    return 'A' <= row && row <= 'H';
};

bool CheckCol(char col) noexcept {
    return 1 <= col && col <= 8;
}

Coords::Coords(char row, char col) noexcept {
    if (CheckRow(row) && CheckCol(col)) {
        _row = row;
        _col = col;
    } else {
        // TODO придумать, что делать в случае неверно заданных координат
    }
}

char Coords::GetRow() const noexcept {
    return _row;
}

char Coords::GetCol() const noexcept {
    return _col;
}

int Coords::GetRowIndex() const noexcept {
    return _row - 'A';
}

int Coords::GetColIndex() const noexcept {
    return _col - 1;
}

std::string Coords::ToString() const noexcept {
    std::string s;
    s.push_back(_row); s.push_back(_col);
    return s;
}

std::ostream& operator << (std::ostream& in, const Coords& coords) {
    return in << coords.GetRow() << coords.GetCol();
}

