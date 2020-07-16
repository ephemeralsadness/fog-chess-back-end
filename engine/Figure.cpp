#include "Figure.h"

size_t ColoredFigureHash::operator() (const ColoredFigure& f) const noexcept {
    return static_cast<int>(f.figure) * (static_cast<int>(f.color) + 1);
}

bool operator == (const ColoredFigure& lhs, const ColoredFigure& rhs) {
    if (lhs.figure == Figure::NOTHING) {
        return rhs.figure == Figure::NOTHING;
    }
    return lhs.figure == rhs.figure && lhs.color == rhs.color;
}
