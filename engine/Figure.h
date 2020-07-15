#pragma once


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

    ColoredFigure(Color color, Figure figure)
        : color(color), figure(figure) {}
};