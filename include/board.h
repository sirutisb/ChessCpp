#pragma once
#include "Piece.h"
#include <vector>
#include <optional>
#include <vector>
#include <array>
#include <string>
#include <sstream>
#include <cctype>

struct Move {
    Position from;
    Position to;
    Promotion promotion = Promotion::None;
    bool isCapture : 1;
    bool isEnpassant : 1;
    bool isCastling : 1;
};

class Board {
public:
    // Construct an empty board or load from a FEN string when provided.
    // If `fen` is empty, startTurn is used to set side to move and the board is empty.
    Board(PieceColor startTurn = PieceColor::White, const std::string& fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")
    : turn_(startTurn)
    , board_{}
    , en_passant_target_{-1, -1}
    {
        if (!fen.empty()) parseFEN(fen);
    }

    ~Board() = default;

    bool inBounds(const Position& p) const {
        return p.row >= 0 && p.row < 8 && p.col >= 0 && p.col < 8;
    }

    Piece* getPiece(const Position& p) {
        if (!inBounds(p)) return nullptr;
        auto& opt = board_[p.row][p.col];
        return opt ? &*opt : nullptr;
    }

    const Piece* getPiece(const Position& p) const {
        if (!inBounds(p)) return nullptr;
        const auto& opt = board_[p.row][p.col];
        return opt ? &*opt : nullptr;
    }

    PieceColor getTurn() const { return turn_; }
    void switchTurn() { turn_ = turn_ == PieceColor::White ? PieceColor::Black : PieceColor::White; }

    Position enPassantTarget() const { return en_passant_target_; }

    const bool* getCastlingRights() const { return castling_rights_; }

    // Return an ASCII representation of the board: ranks 8->1, files a->h
    // Example:
    // 8 r n b q k b n r
    // 7 p p p p p p p p
    // 6 . . . . . . . .
    //   a b c d e f g h
    std::string toString() const {
        std::ostringstream os;
        for (int row = 0; row < 8; ++row) {
            int dispRank = 8 - row;
            os << dispRank << ' ';
            for (int col = 0; col < 8; ++col) {
                const auto& opt = board_[row][col];
                if (!opt) {
                    os << '.';
                } else {
                    char ch = '?';
                    switch (opt->type()) {
                        case PieceType::Pawn:   ch = 'p'; break;
                        case PieceType::Knight: ch = 'n'; break;
                        case PieceType::Bishop: ch = 'b'; break;
                        case PieceType::Rook:   ch = 'r'; break;
                        case PieceType::Queen:  ch = 'q'; break;
                        case PieceType::King:   ch = 'k'; break;
                        default: ch = '?'; break;
                    }
                    if (opt->color() == PieceColor::White) ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
                    os << ch;
                }
                if (col < 7) os << ' ';
            }
            os << '\n';
        }
        os << "  a b c d e f g h\n";
        return os.str();
    }

    bool tryMakeMove(const Move& move) {
        std::vector<Move> legal = legalMovesFrom(move.from);
        for (const auto& m : legal) {
            if (m.from == move.from && m.to == move.to) {
                // Apply the canonical legal move so we preserve flags (promotion, en-passant, castling)
                makeMove(m);
                return true;
            }
        }
        return false;
    }

    void makeMove(const Move& move) {
        // Ensure there is a piece to move (caller should have validated this)
        if (!inBounds(move.from) || !inBounds(move.to)) return;
        if (!board_[move.from.row][move.from.col]) return;

        // Copy the moving piece so we can inspect its type/color before we change the board
        Piece movingPiece = *board_[move.from.row][move.from.col];

        // Handle en-passant capture: the captured pawn sits on the same row as the mover's from.row
        // and in the destination column (i.e. Position(from.row, to.col)).
        if (move.isEnpassant) {
            Position capturedPos(move.from.row, move.to.col);
            if (inBounds(capturedPos)) board_[capturedPos.row][capturedPos.col].reset();
        }

        // castling handle
        if (move.isCastling && movingPiece.type() == PieceType::King) {
            int row = move.from.row;
            if (move.to.col == 6) { // kingside
                board_[row][5] = std::move(board_[row][7]);
                board_[row][7].reset();
            } else if (move.to.col == 2) { // queenside
                board_[row][3] = std::move(board_[row][0]);
                board_[row][0].reset();
            }
        }


        // Move the piece
        board_[move.to.row][move.to.col] = std::move(board_[move.from.row][move.from.col]);
        board_[move.from.row][move.from.col].reset();

        // Update en-passant target: if a pawn moved two squares, set the target to the square
        // it passed over. Otherwise clear the en-passant target.
        en_passant_target_ = Position(-1, -1);
        if (movingPiece.type() == PieceType::Pawn) {
            int delta = move.to.row - move.from.row;
            if (delta == 2 || delta == -2) {
                int dir = (movingPiece.color() == PieceColor::White) ? -1 : 1;
                en_passant_target_ = Position(move.from.row + dir, move.from.col);
            }
        }

        // Update castling rights
        if (movingPiece.type() == PieceType::King && movingPiece.color() == PieceColor::White) {
            castling_rights_[0] = false;
            castling_rights_[1] = false;
        }
        if (movingPiece.type() == PieceType::King && movingPiece.color() == PieceColor::Black) {
            castling_rights_[2] = false;
            castling_rights_[3] = false;
        }
        if (movingPiece.type() == PieceType::Rook && movingPiece.color() == PieceColor::White) {
            if (move.from.row == 7 && move.from.col == 0) castling_rights_[1] = false; // queenside
            if (move.from.row == 7 && move.from.col == 7) castling_rights_[0] = false; // kingside
        }
        if (movingPiece.type() == PieceType::Rook && movingPiece.color() == PieceColor::Black) {
            if (move.from.row == 0 && move.from.col == 0) castling_rights_[3] = false; // queenside
            if (move.from.row == 0 && move.from.col == 7) castling_rights_[2] = false; // kingside
        }
        if (move.isCapture) {
            if (move.to.row == 7 && move.to.col == 0) castling_rights_[1] = false;
            if (move.to.row == 7 && move.to.col == 7) castling_rights_[0] = false;
            if (move.to.row == 0 && move.to.col == 0) castling_rights_[3] = false;
            if (move.to.row == 0 && move.to.col == 7) castling_rights_[2] = false;
        }

        // promotion logic

        if (move.promotion != Promotion::None) {
            auto& old = board_[move.to.row][move.to.col];
            if (old) {
                Piece promoted(PieceType::Queen, old.value().color());
                old = promoted;
            }
        }

        // Switch side to move
        turn_ = (turn_ == PieceColor::White) ? PieceColor::Black : PieceColor::White;
    }

    std::vector<Move> legalMoves() const;
    std::vector<Move> legalMovesFrom(const Position& p) const;

    bool isKingInCheck(PieceColor color) const;

private:
    PieceColor turn_;
    std::array<std::array<std::optional<Piece>, 8>, 8> board_;
    Position en_passant_target_;

    // int halfmove_clock_ = 0;
    // Castling rights: [white kingside, white queenside, black kingside, black queenside]
    bool castling_rights_[4] = {true, true, true, true};
    // Track en passant target square (-1, -1 if none)
    // std::pair<int, int> en_passant_target_ = {-1, -1};

    void parseFEN(const std::string& fen) {
        // clear board
        for (int r = 0; r < 8; ++r)
            for (int c = 0; c < 8; ++c)
                board_[r][c].reset();

        std::istringstream iss(fen);
        std::string placement, side, castling, enpass;
        if (!(iss >> placement)) return;
        iss >> side;
        if (!(iss >> castling)) castling = "-";
        if (!(iss >> enpass)) enpass = "-";

        // parse placement: ranks separated by '/'
        std::vector<std::string> ranks;
        {
            std::istringstream ps(placement);
            std::string rank;
            while (std::getline(ps, rank, '/')) ranks.push_back(rank);
        }
        if (ranks.size() != 8) return;

        for (int rr = 0; rr < 8; ++rr) {
            const std::string& rank = ranks[rr];
            int row = rr; // ranks[0] is rank 8 -> row 0
            int col = 0;
            for (char ch : rank) {
                if (std::isdigit(static_cast<unsigned char>(ch))) {
                    col += ch - '0';
                } else {
                    PieceColor color = std::isupper(static_cast<unsigned char>(ch)) ? PieceColor::White : PieceColor::Black;
                    char lower = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
                    PieceType type = PieceType::None;
                    switch (lower) {
                        case 'p': type = PieceType::Pawn; break;
                        case 'n': type = PieceType::Knight; break;
                        case 'b': type = PieceType::Bishop; break;
                        case 'r': type = PieceType::Rook; break;
                        case 'q': type = PieceType::Queen; break;
                        case 'k': type = PieceType::King; break;
                        default: type = PieceType::None; break;
                    }
                    if (type != PieceType::None && col >= 0 && col < 8 && row >= 0 && row < 8) {
                        board_[row][col] = Piece(type, color);
                    }
                    ++col;
                }
            }
        }

        // side to move
        if (!side.empty() && side[0] == 'b') turn_ = PieceColor::Black; else turn_ = PieceColor::White;

        // en-passant target
        if (enpass == "-") {
            en_passant_target_ = Position(-1, -1);
        } else if (enpass.size() >= 2) {
            char file = enpass[0];
            char rankch = enpass[1];
            int col = file - 'a';
            int rank = rankch - '0';
            int row = 8 - rank; // rank 8 -> row 0
            if (row >= 0 && row < 8 && col >= 0 && col < 8) en_passant_target_ = Position(row, col);
            else en_passant_target_ = Position(-1, -1);
        } else {
            en_passant_target_ = Position(-1, -1);
        }
    }
};
