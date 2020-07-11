#pragma once

#include <cctype>
#include <string>
#include <iostream>

class Coords {
public:
    Coords(char row, char col) noexcept;
    char GetRow() const noexcept;
    char GetCol() const noexcept;
    int GetRowIndex() const noexcept;
    int GetColIndex() const noexcept;
    std::string ToString() const noexcept;
private:
    char _row;
    char _col;
};

std::ostream& operator << (std::ostream& in, const Coords& coords);