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
    QUEEN,
    KING
};

struct ColoredFigure {
    Color color;
    Figure figure;

    ColoredFigure(Color color, Figure figure)
        : color(color), figure(figure) {}
};

struct ColoredFigureHash {
    // При этом хеш-значения { WHITE, NOTHING } и { BLACK, NOTHING } равны.
    size_t operator() (ColoredFigure f) const noexcept {
        return static_cast<int>(f.figure) * (static_cast<int>(f.color) + 1);
    }
};