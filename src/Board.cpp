#include "Board.h"
#include "MoveGenerator.h"
#include <vector>

std::vector<Move> Board::legalMoves() const {
    std::vector<Move> pseudo;
    MoveGenerator::generateAll(*this, pseudo);
    std::vector<Move> legal;
    legal.reserve(pseudo.size());
    for (auto& m : pseudo) {
        Board copy = *this; // or use makeMove/undoMove for performance
        copy.makeMove(m);
        if (!copy.isKingInCheck(turn_)) legal.push_back(m);
    }
    return legal;
}

std::vector<Move> Board::legalMovesFrom(const Position& p) const {
    std::vector<Move> pseudo;
    MoveGenerator::generateFrom(*this, p, pseudo);
    std::vector<Move> legal;
    legal.reserve(pseudo.size());
    for (auto& m : pseudo) {
        Board copy = *this; // or use makeMove/undoMove for performance
        copy.makeMove(m);
        if (!copy.isKingInCheck(turn_)) legal.push_back(m);
    }
    return legal;
}