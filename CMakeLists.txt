cmake_minimum_required(VERSION 3.14)
project(fog_chess_back_end)

set(CMAKE_CXX_STANDARD 17)

add_executable(fog_chess_back_end test.cpp engine/Game.cpp engine/Game.h engine/Chessboard.cpp engine/Chessboard.h engine/Coords.cpp engine/Coords.h engine/Figure.cpp engine/Figure.h)