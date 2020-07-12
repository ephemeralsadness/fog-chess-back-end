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
    Coords(char row, char col) noexcept;

    /**
     * Стандартные геттеры (см. к-тор Coords(row, col))
     */
    char GetRow() const noexcept;
    char GetCol() const noexcept;

    /**
     * Возвращают порядковый номер (от 0 до 7) ряда или колонки.
     */
    int GetRowIndex() const noexcept;
    int GetColIndex() const noexcept;

    /**
     * Стандартные сеттеры (см. к-тор Coords(row, col))
     */
    void SetCol(char col) noexcept;
    void SetRow(char row) noexcept;

    /**
     * Строка вида "row,col" (без символа запятой)
     */
    std::string ToString() const noexcept;
private:
    char _row;
    char _col;
};

/**
 * Оператор вывода в поток класса Coords. Данные выводятся в формате строки, получаемой в методе ToString()
 * @param out поток вывода
 * @param coords объект координат
 */
std::ostream& operator << (std::ostream& out, const Coords& coords);