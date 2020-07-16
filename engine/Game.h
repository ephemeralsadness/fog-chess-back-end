#pragma once

#include "Chessboard.h"

#include <memory>

class Game {
public:

private:
    std::unique_ptr<Chessboard> chessboard;
};