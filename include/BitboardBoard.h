#pragma once
#include "Board.h"

class BitBoardBoard : public Board {
private:
    std::uint64_t white_pawns;
    std::uint64_t white_knights;
    std::uint64_t white_bishops;
    std::uint64_t white_rooks;
    std::uint64_t white_queens;
    std::uint64_t white_kings;

    std::uint64_t black_pawns;
    std::uint64_t black_knights;
    std::uint64_t black_bishops;
    std::uint64_t black_rooks;
    std::uint64_t black_queens;
    std::uint64_t black_kings;
};