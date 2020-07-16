#include "Coords.h"

Coords::Coords(int8_t row, int8_t col) noexcept {
    SetRow(row);
    SetCol(col);
}

char Coords::GetRow() const noexcept {
    return _row;
}

char Coords::GetCol() const noexcept {
    return _col;
}

void Coords::SetRow(int8_t row) noexcept {
    _row = row;
}

void Coords::SetCol(int8_t col) noexcept {
    _col = col;
}

std::string Coords::ToString() const noexcept {
    std::string s;
    s.push_back('1' + _row); s.push_back('A' + _col);
    return s;
}

std::ostream& operator << (std::ostream& out, const Coords& coords) {
    return out << coords.ToString();
}

bool operator == (const Coords& lhs, const Coords& rhs) {
    return lhs.GetRow() == rhs.GetRow() && lhs.GetCol() == rhs.GetCol();
}
