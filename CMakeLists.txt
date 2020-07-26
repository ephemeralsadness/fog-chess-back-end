cmake_minimum_required(VERSION 3.14)
project(fog_chess_back_end)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost 1.71.0 REQUIRED)

add_executable(server engine/Game.cpp engine/Game.h engine/Chessboard.cpp engine/Chessboard.h engine/Coords.cpp engine/Coords.h engine/Figure.cpp engine/Figure.h main.cpp Server.cpp Server.h)
target_compile_definitions (server PRIVATE BOOST_ERROR_CODE_HEADER_ONLY)
target_link_libraries(server pthread)
