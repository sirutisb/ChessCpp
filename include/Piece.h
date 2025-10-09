#pragma once

enum class PieceColor { None, White, Black };
enum class PieceType { None, Pawn, Knight, Bishop, Rook, Queen, King };

struct Position {
    int row, col;
    Position(int row = 0, int col = 0) : row(row), col(col) {}

    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }
};

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

enum class Promotion { None, Knight, Bishop, Rook, Queen };
