#pragma once
#include "Board.h"
#include <vector>

class MoveGenerator {
public:
    static void generateAll(const Board& board, std::vector<Move>& out) {
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                Position pos(r, c);
                generateFrom(board, pos, out);
            }
        }
    }

    static void generateAllSide(const Board& board, PieceColor side, std::vector<Move>& out) {
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                Position pos(r, c);
                const Piece* p = board.getPiece(pos);
                if (p && p->color() != side) continue;
                generateFrom(board, pos, out);
            }
        }
    }

    static void generateFrom(const Board& board, const Position& from, std::vector<Move>& out) {
        const Piece* p = board.getPiece(from);
        if (!p) return;
        switch (p->type()) {
        case PieceType::Pawn:   add_pawnMoves(board, from, out); break;
        case PieceType::Knight: add_knightMoves(board, from, out); break;
        case PieceType::Bishop: add_bishopMoves(board, from, out); break;
        case PieceType::Rook:   add_rookMoves(board, from, out); break;
        case PieceType::Queen:  add_queenMoves(board, from, out); break;
        case PieceType::King:   add_kingMoves(board, from, out); break;
        default: break;
        }
    }

private:
    static void add_pawnMoves(const Board& board, const Position& from, std::vector<Move>& out) {
        const Piece* me = board.getPiece(from);
        if (!me || me->type() != PieceType::Pawn) return;

        const int dir = (me->color() == PieceColor::White) ? -1 : 1;
        const int startRow = (me->color() == PieceColor::White) ? 6 : 1;
        const int promoteRow = (me->color() == PieceColor::White) ? 0 : 7;
        Position one { from.row + dir, from.col };
        if (board.inBounds(one) && board.getPiece(one) == nullptr) {
            if (one.row == promoteRow) {
                for (Promotion pr : {Promotion::Queen, Promotion::Rook, Promotion::Knight, Promotion::Bishop }) {
                    Move m{from, one};
                    m.Promotion = pr;
                    m.isCapture = false;
                    m.isEnpassant = false;
                    m.isCastling = false;
                    out.push_back(m);
                }
            } else {
                Move m{from, one};
                m.Promotion = Promotion::None;
                m.isCapture = false;
                m.isEnpassant = false;
                m.isCastling = false;
                out.push_back(m);

                Position two{ from.row + 2*dir, from.col };
                if (from.row == startRow && board.inBounds(two) && board.getPiece(two) == nullptr) {
                    Move m2{from, two};
                    m2.Promotion = Promotion::None;
                    m2.isCapture = false;
                    m2.isEnpassant = false; // this is set when the move is applied
                    m2.isCastling = false;
                    out.push_back(m2);
                }
            }
        }
        
        Position enPass = board.enPassantTarget();
        for (int dc : {-1, 1}) {
            Position cap{ from.row + dir, from.col + dc};
            if (!board.inBounds(cap)) continue;
            const Piece* target = board.getPiece(cap);
            if (target && target->color() != me->color()) {
                if (cap.row == promoteRow) {
                    for (Promotion pr : { Promotion::Queen, Promotion::Rook, Promotion::Bishop, Promotion::Knight }) {
                        Move m{from, cap};
                        m.Promotion = pr;
                        m.isCapture = true;
                        m.isEnpassant = false;
                        m.isCastling = false;
                        out.push_back(m);
                    }
                } else {
                    Move m{from, cap};
                    m.Promotion = Promotion::None;
                    m.isCapture = true;
                    m.isEnpassant = false;
                    m.isCastling = false;
                    out.push_back(m);
                }
            }

            // en-passant capture
            if (enPass.row == cap.row && enPass.col == cap.col) {
                // ensure the en-passant target exists and the captured pawn is on the correct square
                // board.enPassantTarget() should have been set to the square behind the moved pawn
                Move m{from, cap};
                m.Promotion = Promotion::None;
                m.isCapture = true;
                m.isEnpassant = true;
                m.isCastling = false;
                out.push_back(m);
            }
        }
    }

    static void add_knightMoves(const Board& board, const Position& from, std::vector<Move>& out) {
        static int dr[8] {-1, -1,  1,  1, -2, -2,  2,  2};
        static int dc[8] {-2,  2, -2,  2, -1,  1, -1,  1};
        const Piece* me = board.getPiece(from);
        for (int i = 0; i < 8; i++) {
            Position cap(from.row + dr[i], from.col + dc[i]);
            if (!board.inBounds(cap)) continue;
            const Piece* target = board.getPiece(cap);
            if (target && target->color() == me->color()) continue;

            Move m;
            m.from = from;
            m.to = cap;
            m.isCapture = target ? true : false;
            m.isEnpassant = false;
            m.isCastling = false;
            m.Promotion = Promotion::None;

            out.push_back(m);
        }
    }

    static void add_bishopMoves(const Board& board, const Position& from, std::vector<Move>& out) {
        const Piece* me = board.getPiece(from);

        auto helper = [&](const Position& cap) -> bool {
            const Piece* target = board.getPiece(cap);
            
            if (target) {
                if (target->color() != me->color()) {
                    Move m;
                    m.from = from;
                    m.to = cap;
                    m.isCapture = true;
                    m.isEnpassant = false;
                    m.isCastling = false;
                    m.Promotion = Promotion::None;
                    out.push_back(m);
                }
                return false;
            }

            Move m;
            m.from = from;
            m.to = cap;
            m.isCapture = false;
            m.isEnpassant = false;
            m.isCastling = false;
            m.Promotion = Promotion::None;
            out.push_back(m);
            return true;
        };

        for (Position p(from.row + 1, from.col + 1); board.inBounds(p); p.row++, p.col++) {
            if (!helper(p)) break;
        }

        for (Position p(from.row - 1, from.col - 1); board.inBounds(p); p.row--, p.col--) {
            if (!helper(p)) break;
        }

        for (Position p(from.row - 1, from.col + 1); board.inBounds(p); p.row--, p.col++) {
            if (!helper(p)) break;
        }

        for (Position p(from.row + 1, from.col - 1); board.inBounds(p); p.row++, p.col--) {
            if (!helper(p)) break;
        }
    }

    static void add_rookMoves(const Board& board, const Position& from, std::vector<Move>& out) {
        const Piece* me = board.getPiece(from);

        auto helper = [&](const Position& cap) -> bool {
            const Piece* target = board.getPiece(cap);
            
            if (target) {
                if (target->color() != me->color()) {
                    Move m;
                    m.from = from;
                    m.to = cap;
                    m.isCapture = true;
                    m.isEnpassant = false;
                    m.isCastling = false;
                    m.Promotion = Promotion::None;
                    out.push_back(m);
                }
                return false;
            }

            Move m;
            m.from = from;
            m.to = cap;
            m.isCapture = false;
            m.isEnpassant = false;
            m.isCastling = false;
            m.Promotion = Promotion::None;
            out.push_back(m);
            return true;
        };

        for (int r = from.row + 1; r < 8; r++) {
            Position cap({r, from.col});
            if (!helper(cap)) break;
        }

        for (int r = from.row - 1; r >= 0; r--) {
            Position cap({r, from.col});
            if (!helper(cap)) break;
        }

        for (int c = from.col + 1; c < 8; c++) {
            Position cap({from.row, c});
            if (!helper(cap)) break;
        }

        for (int c = from.col - 1; c >= 0; c--) {
            Position cap({from.row, c});
            if (!helper(cap)) break;
        }
    }

    static void add_queenMoves(const Board& board, const Position& from, std::vector<Move>& out) {
        add_rookMoves(board, from, out);
        add_bishopMoves(board, from, out);
    }
    
    static void add_kingMoves(const Board& board, const Position& from, std::vector<Move>& out) {
        const Piece* me = board.getPiece(from);
        for (int dr = -1; dr <= 1; dr++) {
            for (int dc = -1; dc <= 1; dc++) {
                if (dr == 0 && dc == 0) continue;
                Position pos(from.row + dr, from.col + dc);
                if (!board.inBounds(pos)) continue;
                const Piece* target = board.getPiece(pos);
                if (target && target->color() == me->color()) continue;
                Move m;
                m.from = from;
                m.to = pos;
                m.isCapture = target ? true : false;
                m.isCastling = false;
                m.isEnpassant = false;
                m.Promotion = Promotion::None;
                out.push_back(m);
            }
        }

        // Castling logic
        // [0]=white kingside, [1]=white queenside, [2]=black kingside, [3]=black queenside
        PieceColor color = me->color();
        PieceColor opp = (color == PieceColor::White) ? PieceColor::Black : PieceColor::White;
        int row = (color == PieceColor::White) ? 7 : 0;
        int king_col = 4;
        const bool* castling_rights = board.getCastlingRights();
        // Kingside
        int kingside_idx = (color == PieceColor::White) ? 0 : 2;
        if (from.row == row && from.col == king_col && castling_rights[kingside_idx]) {
            // Squares between king and rook must be empty
            if (!board.getPiece({row, 5}) && !board.getPiece({row, 6})) {
                // King not in check, and does not pass through or land in check
                if (!isSquareAttacked(board, {row, 4}, opp) &&
                    !isSquareAttacked(board, {row, 5}, opp) &&
                    !isSquareAttacked(board, {row, 6}, opp)) {
                    Move m;
                    m.from = from;
                    m.to = {row, 6};
                    m.isCapture = false;
                    m.isCastling = true;
                    m.isEnpassant = false;
                    m.Promotion = Promotion::None;
                    out.push_back(m);
                }
            }
        }
        // Queenside
        int queenside_idx = (color == PieceColor::White) ? 1 : 3;
        if (from.row == row && from.col == king_col && castling_rights[queenside_idx]) {
            if (!board.getPiece({row, 1}) && !board.getPiece({row, 2}) && !board.getPiece({row, 3})) {
                if (!isSquareAttacked(board, {row, 4}, opp) &&
                    !isSquareAttacked(board, {row, 3}, opp) &&
                    !isSquareAttacked(board, {row, 2}, opp)) {
                    Move m;
                    m.from = from;
                    m.to = {row, 2};
                    m.isCapture = false;
                    m.isCastling = true;
                    m.isEnpassant = false;
                    m.Promotion = Promotion::None;
                    out.push_back(m);
                }
            }
        }
    }

    // Returns true if the given square is attacked by the given color
    static bool isSquareAttacked(const Board& board, const Position& sq, PieceColor byColor) {
        std::vector<Move> theirMoves;
    MoveGenerator::generateAllSide(board, byColor, theirMoves);
        for (const auto& m : theirMoves) {
            if (m.to == sq) return true;
        }
        return false;
    }
};