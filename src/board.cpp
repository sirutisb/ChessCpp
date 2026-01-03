#include "board.h"
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

bool Board::isKingInCheck(PieceColor color) const {
        PieceColor opponent = color == PieceColor::White ? PieceColor::Black : PieceColor::White;
        std::vector<Move> legal;
        MoveGenerator::generateAllSide(*this, opponent, legal);

        // find the king
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                Position pos(r, c);
                const Piece* piece = getPiece(pos);
                if (!piece || piece->color() != color || piece->type() != PieceType::King) continue;

                // king found
                for (const auto& m : legal) {
                    if (m.to == pos) return true;
                }
                return false;
            }
        }

        // throw std::runtime_error("Why is there no king");
        return false; // no king found?
    }
