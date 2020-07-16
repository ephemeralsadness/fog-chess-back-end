#pragma once

#include <cctype>
#include <string>
#include <iostream>

class Coords {
public:
    /**
     * Конструктор по умолчанию
     */
    Coords() noexcept : _row(0), _col(0) { }

    /**
     * Конструктор от двух символов
     * @param row символ от '1' до '8' - ряд шахматной доски
     * @param col символ от 'A' до 'H' - колонка шахматной доски
     */
    Coords(int8_t row, int8_t col) noexcept;

    /**
     * Стандартные геттеры (см. к-тор Coords(row, col))
     */
    char GetRow() const noexcept;
    char GetCol() const noexcept;

    /**
     * Стандартные сеттеры (см. к-тор Coords(row, col))
     */
    void SetCol(int8_t col) noexcept;
    void SetRow(int8_t row) noexcept;

    /**
     * Строка вида "C2", "A5" и т.д.
     */
    std::string ToString() const noexcept;
private:
    int8_t _row;
    int8_t _col;
};

/**
 * Оператор вывода в поток класса Coords. Данные выводятся в формате строки, получаемой в методе ToString()
 * @param out поток вывода
 * @param coords объект координат
 */
std::ostream& operator << (std::ostream& out, const Coords& coords);

bool operator == (const Coords& lhs, const Coords& rhs);