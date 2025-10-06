#pragma once
#include "Board.h"
#include <vector>

class Piece {
public:
    Piece(PieceType type, PieceColor color)
        : type_(type), color_(color) {}

    PieceType type() const { return type_; }
    PieceColor color() const { return color_; }
private:
    PieceType type_;
    PieceColor color_;
};

class ArrayBoard : public Board {
public:
    // For repetition and 50-move rule
    mutable std::vector<std::string> position_history_;
    int halfmove_clock_ = 0;
    // Castling rights: [white kingside, white queenside, black kingside, black queenside]
    bool castling_rights_[4] = {true, true, true, true};
    // Track en passant target square (-1, -1 if none)
    std::pair<int, int> en_passant_target_ = {-1, -1};
    ArrayBoard(PieceColor startTurn = PieceColor::White)
        : turn_(startTurn)
        , board_{}
    {}

    ~ArrayBoard() override {
    }

    std::string to_string() const override {
        std::string str;
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                Piece* piece = board_[row][col];
                char c = '.';
                if (piece) {
                    switch (piece->type()) {
                        case PieceType::Pawn:   c = 'P'; break;
                        case PieceType::Knight: c = 'K'; break;
                        case PieceType::Bishop: c = 'B'; break;
                        case PieceType::Rook:   c = 'R'; break;
                        case PieceType::Queen:  c = 'Q'; break;
                        case PieceType::King:   c = 'K'; break;
                        default: break;
                    }
                    if (piece->color() == PieceColor::Black)
                        c = tolower(c);
                }
                str += c;
                str += ' ';
            }
            str += '\n';
        }
        return str;
    }

    PieceType get_piece_type(int row, int col) const override {
        Piece* piece = board_[row][col];
        return piece ? piece->type() : PieceType::None;
    }

    PieceColor get_piece_color(int row, int col) const override {
        Piece* piece = board_[row][col];
        return piece ? piece->color() : PieceColor::None;
    }

    // doesnt check bounds
    bool make_move(const Move& move) override {
        Piece* fromPiece = board_[move.fromRow][move.fromCol];
        if (!fromPiece || fromPiece->color() != turn_) return false;

        // Generate valid moves for this piece
        auto validMoves = generate_valid_moves_for_piece(move.fromRow, move.fromCol);
        auto it = std::find_if(validMoves.begin(), validMoves.end(),
            [&](const Move& m) {
                return m.toRow == move.toRow && m.toCol == move.toCol && m.promotion == move.promotion;
        });

        if (it == validMoves.end()) return false; // Not a valid move

        Piece* toPiece = board_[move.toRow][move.toCol];
        // Castling move
        if (fromPiece->type() == PieceType::King && std::abs(move.toCol - move.fromCol) == 2) {
            int homeRow = (fromPiece->color() == PieceColor::White) ? 7 : 0;
            // Kingside
            if (move.toCol == 6) {
                // Move rook
                Piece* rook = board_[homeRow][7];
                board_[homeRow][5] = rook;
                board_[homeRow][7] = nullptr;
            } else if (move.toCol == 2) {
                // Queenside
                Piece* rook = board_[homeRow][0];
                board_[homeRow][3] = rook;
                board_[homeRow][0] = nullptr;
            }
        }
        // Pawn promotion
        if (fromPiece->type() == PieceType::Pawn &&
            ((fromPiece->color() == PieceColor::White && move.toRow == 0) ||
             (fromPiece->color() == PieceColor::Black && move.toRow == 7)) &&
            move.promotion != PieceType::None) {
            if (toPiece) delete toPiece;
            delete fromPiece;
            board_[move.toRow][move.toCol] = new Piece(move.promotion, turn_);
        } else {
            if (toPiece) delete toPiece;
            board_[move.toRow][move.toCol] = fromPiece;
        }
        board_[move.fromRow][move.fromCol] = nullptr;

        // Update halfmove clock for 50-move rule
        if (fromPiece->type() == PieceType::Pawn || toPiece) {
            halfmove_clock_ = 0;
        } else {
            ++halfmove_clock_;
        }

        // Update position history for repetition
        position_history_.push_back(to_string());

        // Update castling rights
        if (fromPiece->type() == PieceType::King) {
            if (fromPiece->color() == PieceColor::White) {
                castling_rights_[0] = false;
                castling_rights_[1] = false;
            } else {
                castling_rights_[2] = false;
                castling_rights_[3] = false;
            }
        }
        if (fromPiece->type() == PieceType::Rook) {
            if (fromPiece->color() == PieceColor::White) {
                if (move.fromRow == 7 && move.fromCol == 0) castling_rights_[1] = false; // Queenside
                if (move.fromRow == 7 && move.fromCol == 7) castling_rights_[0] = false; // Kingside
            } else {
                if (move.fromRow == 0 && move.fromCol == 0) castling_rights_[3] = false; // Queenside
                if (move.fromRow == 0 && move.fromCol == 7) castling_rights_[2] = false; // Kingside
            }
        }

        turn_ = (turn_ == PieceColor::White) ? PieceColor::Black : PieceColor::White;
        return true;
    }

    PieceColor current_turn() const override {
        return turn_;
    }
    bool is_game_over() const override {
        // Check if current player has any legal moves
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                Piece* piece = board_[row][col];
                if (piece && piece->color() == turn_) {
                    auto legalMoves = generate_legal_moves_for_piece(row, col);
                    if (!legalMoves.empty()) {
                        return false; // At least one legal move exists
                    }
                }
            }
        }
        // Checkmate or stalemate
        bool hasLegalMove = false;
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                Piece* piece = board_[row][col];
                if (piece && piece->color() == turn_) {
                    auto legalMoves = generate_legal_moves_for_piece(row, col);
                    if (!legalMoves.empty()) {
                        hasLegalMove = true;
                        break;
                    }
                }
            }
            if (hasLegalMove) break;
        }
        if (!hasLegalMove) return true; // Checkmate or stalemate

        // Insufficient material
        if (is_insufficient_material()) return true;

        // Threefold repetition
        if (is_threefold_repetition()) return true;

        // Fifty-move rule
        if (halfmove_clock_ >= 100) return true;

        return false;
    }

    std::string get_game_result() const {
        // Checkmate or stalemate
        bool hasLegalMove = false;
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                Piece* piece = board_[row][col];
                if (piece && piece->color() == turn_) {
                    auto legalMoves = generate_legal_moves_for_piece(row, col);
                    if (!legalMoves.empty()) {
                        hasLegalMove = true;
                        break;
                    }
                }
            }
            if (hasLegalMove) break;
        }
        if (!hasLegalMove) {
            if (is_king_in_check(turn_)) return "checkmate";
            else return "stalemate";
        }
        if (is_insufficient_material()) return "draw: insufficient material";
        if (is_threefold_repetition()) return "draw: threefold repetition";
        if (halfmove_clock_ >= 100) return "draw: fifty-move rule";
        return "ongoing";
    }
    // Helper: Insufficient material (only kings, or king + bishop/knight vs king)
    bool is_insufficient_material() const {
        int whitePieces = 0, blackPieces = 0;
        int whiteBishops = 0, blackBishops = 0;
        int whiteKnights = 0, blackKnights = 0;
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                Piece* piece = board_[row][col];
                if (!piece) continue;
                if (piece->type() == PieceType::King) continue;
                if (piece->color() == PieceColor::White) {
                    ++whitePieces;
                    if (piece->type() == PieceType::Bishop) ++whiteBishops;
                    if (piece->type() == PieceType::Knight) ++whiteKnights;
                } else if (piece->color() == PieceColor::Black) {
                    ++blackPieces;
                    if (piece->type() == PieceType::Bishop) ++blackBishops;
                    if (piece->type() == PieceType::Knight) ++blackKnights;
                }
            }
        }
        // Only kings
        if (whitePieces == 0 && blackPieces == 0) return true;
        // King + bishop or knight vs king
        if ((whitePieces == 1 && whiteBishops == 1 && blackPieces == 0) ||
            (whitePieces == 1 && whiteKnights == 1 && blackPieces == 0) ||
            (blackPieces == 1 && blackBishops == 1 && whitePieces == 0) ||
            (blackPieces == 1 && blackKnights == 1 && whitePieces == 0)) return true;
        // King + bishop vs king + bishop (same color squares) not implemented for simplicity
        return false;
    }

    // Helper: Threefold repetition
    bool is_threefold_repetition() const {
        int count = 0;
        std::string last = to_string();
        for (const auto& pos : position_history_) {
            if (pos == last) ++count;
        }
        return count >= 3;
    }
    
private:
    std::vector<Move> generate_valid_moves_for_piece(int row, int col) const {
        std::vector<Move> moves;
        Piece* piece = board_[row][col];
        if (!piece) return moves;

        switch (piece->type()) {
            case PieceType::Pawn:
                add_pawn_moves(row, col, moves);
                break;
            case PieceType::Knight:
                add_knight_moves(row, col, moves);
                break;
            case PieceType::Bishop:
                add_bishop_moves(row, col, moves);
                break;
            case PieceType::Rook:
                add_rook_moves(row, col, moves);
                break;
            case PieceType::Queen:
                add_queen_moves(row, col, moves);
                break;
            case PieceType::King:
                add_king_moves(row, col, moves);
                break;
            default:
                break;
        }
        return moves;
    }

    void add_pawn_moves(int row, int col, std::vector<Move>& moves) const {
        Piece* piece = board_[row][col];
        PieceColor color = piece->color();
        int dir = (color == PieceColor::White) ? -1 : 1;
        int nextRow = row + dir;
        // Single move forward
        if (nextRow >= 0 && nextRow < 8 && !board_[nextRow][col]) {
            // Promotion
            if ((color == PieceColor::White && nextRow == 0) || (color == PieceColor::Black && nextRow == 7)) {
                for (PieceType promo : {PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight}) {
                    moves.push_back({row, col, nextRow, col, promo});
                }
            } else {
                moves.push_back({row, col, nextRow, col});
            }
            // Double move forward from starting position
            int startRow = (color == PieceColor::White) ? 6 : 1;
            int doubleRow = row + 2 * dir;
            if (row == startRow && !board_[doubleRow][col]) {
                moves.push_back({row, col, doubleRow, col});
            }
        }
        // Captures
        for (int dc = -1; dc <= 1; dc += 2) {
            int nc = col + dc;
            if (nc >= 0 && nc < 8) {
                // Normal capture
                if (nextRow >= 0 && nextRow < 8 && board_[nextRow][nc] && board_[nextRow][nc]->color() != color) {
                    // Promotion
                    if ((color == PieceColor::White && nextRow == 0) || (color == PieceColor::Black && nextRow == 7)) {
                        for (PieceType promo : {PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight}) {
                            moves.push_back({row, col, nextRow, nc, promo});
                        }
                    } else {
                        moves.push_back({row, col, nextRow, nc});
                    }
                }
                // En passant capture
                if (en_passant_target_.first == nextRow && en_passant_target_.second == nc) {
                    moves.push_back({row, col, nextRow, nc});
                }
            }
        }
    }

    void add_knight_moves(int row, int col, std::vector<Move>& moves) const {
        static const int dr[8] = {-2, -1, 1, 2, 2, 1, -1, -2};
        static const int dc[8] = {1, 2, 2, 1, -1, -2, -2, -1};
        Piece* piece = board_[row][col];
        PieceColor color = piece->color();
        for (int i = 0; i < 8; ++i) {
            int nr = row + dr[i];
            int nc = col + dc[i];
            if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                Piece* target = board_[nr][nc];
                if (!target || target->color() != color) {
                    moves.push_back({row, col, nr, nc});
                }
            }
        }
    }

    void add_bishop_moves(int row, int col, std::vector<Move>& moves) const {
        Piece* piece = board_[row][col];
        PieceColor color = piece->color();
        static const int dr[4] = {-1, -1, 1, 1};
        static const int dc[4] = {-1, 1, 1, -1};
        for (int d = 0; d < 4; ++d) {
            int nr = row + dr[d];
            int nc = col + dc[d];
            while (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                Piece* target = board_[nr][nc];
                if (!target) {
                    moves.push_back({row, col, nr, nc});
                } else {
                    if (target->color() != color) {
                        moves.push_back({row, col, nr, nc});
                    }
                    break;
                }
                nr += dr[d];
                nc += dc[d];
            }
        }
    }

    void add_rook_moves(int row, int col, std::vector<Move>& moves) const {
        Piece* piece = board_[row][col];
        PieceColor color = piece->color();
        static const int dr[4] = {-1, 0, 1, 0};
        static const int dc[4] = {0, 1, 0, -1};
        for (int d = 0; d < 4; ++d) {
            int nr = row + dr[d];
            int nc = col + dc[d];
            while (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                Piece* target = board_[nr][nc];
                if (!target) {
                    moves.push_back({row, col, nr, nc});
                } else {
                    if (target->color() != color) {
                        moves.push_back({row, col, nr, nc});
                    }
                    break;
                }
                nr += dr[d];
                nc += dc[d];
            }
        }
    }

    void add_queen_moves(int row, int col, std::vector<Move>& moves) const {
        add_bishop_moves(row, col, moves);
        add_rook_moves(row, col, moves);
    }

    void add_king_moves(int row, int col, std::vector<Move>& moves) const {
        static const int dr[8] = {-1, -1, -1, 0, 1, 1, 1, 0};
        static const int dc[8] = {-1, 0, 1, 1, 1, 0, -1, -1};
        Piece* piece = board_[row][col];
        PieceColor color = piece->color();
        for (int i = 0; i < 8; ++i) {
            int nr = row + dr[i];
            int nc = col + dc[i];
            if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                Piece* target = board_[nr][nc];
                if (!target || target->color() != color) {
                    moves.push_back({row, col, nr, nc});
                }
            }
        }

        // Castling logic
        int homeRow = (color == PieceColor::White) ? 7 : 0;
        if (row == homeRow && col == 4) {
            // Kingside
            int rightsIdx = (color == PieceColor::White) ? 0 : 2;
            if (castling_rights_[rightsIdx]
                && board_[homeRow][5] == nullptr
                && board_[homeRow][6] == nullptr
                && board_[homeRow][7] && board_[homeRow][7]->type() == PieceType::Rook && board_[homeRow][7]->color() == color
                && !is_square_attacked(homeRow, 4, (color == PieceColor::White) ? PieceColor::Black : PieceColor::White)
                && !is_square_attacked(homeRow, 5, (color == PieceColor::White) ? PieceColor::Black : PieceColor::White)
                && !is_square_attacked(homeRow, 6, (color == PieceColor::White) ? PieceColor::Black : PieceColor::White)) {
                moves.push_back({homeRow, 4, homeRow, 6}); // King moves two squares right
            }
            // Queenside
            rightsIdx = (color == PieceColor::White) ? 1 : 3;
            if (castling_rights_[rightsIdx]
                && board_[homeRow][3] == nullptr
                && board_[homeRow][2] == nullptr
                && board_[homeRow][1] == nullptr
                && board_[homeRow][0] && board_[homeRow][0]->type() == PieceType::Rook && board_[homeRow][0]->color() == color
                && !is_square_attacked(homeRow, 4, (color == PieceColor::White) ? PieceColor::Black : PieceColor::White)
                && !is_square_attacked(homeRow, 3, (color == PieceColor::White) ? PieceColor::Black : PieceColor::White)
                && !is_square_attacked(homeRow, 2, (color == PieceColor::White) ? PieceColor::Black : PieceColor::White)) {
                moves.push_back({homeRow, 4, homeRow, 2}); // King moves two squares left
            }
        }
    }

        // Helper: Make a move without validation (used for testing legality)
        void make_move_no_validation(const Move& move) {
            Piece* fromPiece = board_[move.fromRow][move.fromCol];
            Piece* toPiece = board_[move.toRow][move.toCol];
            if (toPiece) delete toPiece;
            board_[move.toRow][move.toCol] = fromPiece;
            board_[move.fromRow][move.fromCol] = nullptr;
        }

        // Helper: Find king position for a color
        std::pair<int, int> find_king(PieceColor color) const {
            for (int row = 0; row < 8; ++row) {
                for (int col = 0; col < 8; ++col) {
                    Piece* piece = board_[row][col];
                    if (piece && piece->type() == PieceType::King && piece->color() == color) {
                        return {row, col};
                    }
                }
            }
            return {-1, -1}; // Not found
        }

        // Helper: Is a square attacked by the given color?
        bool is_square_attacked(int targetRow, int targetCol, PieceColor attacker) const {
            // Check all squares for attacking pieces
            for (int row = 0; row < 8; ++row) {
                for (int col = 0; col < 8; ++col) {
                    Piece* piece = board_[row][col];
                    if (!piece || piece->color() != attacker) continue;
                    switch (piece->type()) {
                        case PieceType::Pawn: {
                            int dir = (attacker == PieceColor::White) ? -1 : 1;
                            int pawnRow = row + dir;
                            if (pawnRow == targetRow && (col - 1 == targetCol || col + 1 == targetCol)) {
                                return true;
                            }
                            break;
                        }
                        case PieceType::Knight: {
                            static const int dr[8] = {-2, -1, 1, 2, 2, 1, -1, -2};
                            static const int dc[8] = {1, 2, 2, 1, -1, -2, -2, -1};
                            for (int i = 0; i < 8; ++i) {
                                int nr = row + dr[i];
                                int nc = col + dc[i];
                                if (nr == targetRow && nc == targetCol) return true;
                            }
                            break;
                        }
                        case PieceType::Bishop: {
                            static const int dr[4] = {-1, -1, 1, 1};
                            static const int dc[4] = {-1, 1, 1, -1};
                            for (int d = 0; d < 4; ++d) {
                                int nr = row + dr[d];
                                int nc = col + dc[d];
                                while (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                                    if (nr == targetRow && nc == targetCol) return true;
                                    if (board_[nr][nc]) break;
                                    nr += dr[d];
                                    nc += dc[d];
                                }
                            }
                            break;
                        }
                        case PieceType::Rook: {
                            static const int dr[4] = {-1, 0, 1, 0};
                            static const int dc[4] = {0, 1, 0, -1};
                            for (int d = 0; d < 4; ++d) {
                                int nr = row + dr[d];
                                int nc = col + dc[d];
                                while (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                                    if (nr == targetRow && nc == targetCol) return true;
                                    if (board_[nr][nc]) break;
                                    nr += dr[d];
                                    nc += dc[d];
                                }
                            }
                            break;
                        }
                        case PieceType::Queen: {
                            // Queen = bishop + rook
                            static const int dr[8] = {-1, -1, 1, 1, -1, 0, 1, 0};
                            static const int dc[8] = {-1, 1, 1, -1, 0, 1, 0, -1};
                            for (int d = 0; d < 8; ++d) {
                                int nr = row + dr[d];
                                int nc = col + dc[d];
                                while (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                                    if (nr == targetRow && nc == targetCol) return true;
                                    if (board_[nr][nc]) break;
                                    nr += dr[d];
                                    nc += dc[d];
                                }
                            }
                            break;
                        }
                        case PieceType::King: {
                            static const int dr[8] = {-1, -1, -1, 0, 1, 1, 1, 0};
                            static const int dc[8] = {-1, 0, 1, 1, 1, 0, -1, -1};
                            for (int i = 0; i < 8; ++i) {
                                int nr = row + dr[i];
                                int nc = col + dc[i];
                                if (nr == targetRow && nc == targetCol) return true;
                            }
                            break;
                        }
                        default: break;
                    }
                }
            }
            return false;
        }

        // Helper: Is king of color in check?
        bool is_king_in_check(PieceColor color) const {
            auto kingPos = find_king(color);
            if (kingPos.first == -1) return false; // King not found
            PieceColor opponent = (color == PieceColor::White) ? PieceColor::Black : PieceColor::White;
            return is_square_attacked(kingPos.first, kingPos.second, opponent);
        }

        // Generate legal moves for a piece (does not leave king in check)
        std::vector<Move> generate_legal_moves_for_piece(int row, int col) const {
            std::vector<Move> legalMoves;
            auto pseudoMoves = generate_valid_moves_for_piece(row, col);
            for (const Move& move : pseudoMoves) {
                ArrayBoard tempBoard = *this;
                tempBoard.make_move_no_validation(move);
                if (!tempBoard.is_king_in_check(board_[row][col]->color())) {
                    legalMoves.push_back(move);
                }
            }
            return legalMoves;
        }

    PieceColor turn_;
    Piece* board_[8][8];
};