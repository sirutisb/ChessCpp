#pragma once
#include <cstdint>
#include <string>

enum class PieceType { None, Pawn, Knight, Bishop, Rook, Queen, King};
enum class PieceColor { None, White, Black };

struct Move {
    int fromRow, fromCol, toRow, toCol;
    PieceType promotion = PieceType::None;
};

class Board {
public:
    virtual ~Board() = default;

    virtual std::string to_string() const = 0;
    virtual PieceType get_piece_type(int row, int col) const = 0;
    virtual PieceColor get_piece_color(int row, int col) const = 0;
    virtual bool make_move(const Move& move) = 0;
    virtual PieceColor current_turn() const = 0;
    virtual bool is_game_over() const = 0;
private:
    PieceColor turn_;
};