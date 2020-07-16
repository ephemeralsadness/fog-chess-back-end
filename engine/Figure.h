#pragma once

#include <cstddef>

enum class Color {
    WHITE,
    BLACK
};

enum class Figure {
    NOTHING,
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
};

struct ColoredFigure {
    Color color;
    Figure figure;

    ColoredFigure()
        : color(Color::WHITE), figure(Figure::NOTHING) {}

    ColoredFigure(Color color, Figure figure)
        : color(color), figure(figure) {}
};

struct ColoredFigureHash {
    // При этом хеш-значения { WHITE, NOTHING } и { BLACK, NOTHING } равны.
    size_t operator() (const ColoredFigure& f) const noexcept;
};

bool operator == (const ColoredFigure& lhs, const ColoredFigure& rhs);