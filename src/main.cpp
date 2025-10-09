#include <iostream>
#include "Board.h"

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>

const int TILE_SIZE = 80;
const int BOARD_SIZE = 8;

// Helper to get asset filename for a piece
std::string getPieceAsset(PieceType type, PieceColor color) {
    if (type == PieceType::Pawn) return color == PieceColor::White ? "assets/wp.png" : "assets/bp.png";
    if (type == PieceType::Knight) return color == PieceColor::White ? "assets/wn.png" : "assets/bn.png";
    if (type == PieceType::Bishop) return color == PieceColor::White ? "assets/wb.png" : "assets/bb.png";
    if (type == PieceType::Rook) return color == PieceColor::White ? "assets/wr.png" : "assets/br.png";
    if (type == PieceType::Queen) return color == PieceColor::White ? "assets/wq.png" : "assets/bq.png";
    if (type == PieceType::King) return color == PieceColor::White ? "assets/wk.png" : "assets/bk.png";
    return "undefined piece";
}

int main()
{
    Position draggingPieceFrom(-1, -1);
    std::vector<Move> draggingMoves;
    sf::Vector2i mousePosition;

    sf::RenderWindow window(sf::VideoMode({TILE_SIZE * BOARD_SIZE, TILE_SIZE * BOARD_SIZE}), "Chess Game");
    window.setFramerateLimit(15);
    Board board;

    // Load piece textures
    std::unordered_map<std::string, sf::Texture> textures;
    for (auto color : {PieceColor::White, PieceColor::Black}) {
        for (auto type : {PieceType::Pawn, PieceType::Knight, PieceType::Bishop, PieceType::Rook, PieceType::Queen, PieceType::King}) {
            std::string path = getPieceAsset(type, color);
            if (!path.empty()) {
                textures[path] = sf::Texture(path);
                if (!textures[path].loadFromFile(path)) throw std::runtime_error("Could not find file!");
            }
        }
    }

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mousePressed->button == sf::Mouse::Button::Left) {
                    mousePosition = mousePressed->position;
                    sf::Vector2f worldPos = window.mapPixelToCoords(mousePosition);
                    Position boardPos(worldPos.y / TILE_SIZE, worldPos.x / TILE_SIZE);
                    if (board.inBounds(boardPos)) {
                        const Piece* piece = board.getPiece(boardPos);
                        if (piece && piece->color() == board.getTurn()) {
                            draggingPieceFrom = boardPos;
                            draggingMoves = board.legalMovesFrom(draggingPieceFrom);
                        }
                    }
                }
            }

            if (auto* mouseReleased = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (mouseReleased->button == sf::Mouse::Button::Left) {
                    mousePosition = mouseReleased->position;
                    sf::Vector2f worldPos = window.mapPixelToCoords(mousePosition);
                    Position boardPos(worldPos.y / TILE_SIZE, worldPos.x / TILE_SIZE);
                    if (board.inBounds(boardPos)) {
                        // should handle this a better way
                        Move m;
                        m.from = draggingPieceFrom;
                        m.to = boardPos;
                        board.tryMakeMove(m);
                        draggingPieceFrom = {-1, -1};
                    }
                }
            }

            if (auto* mouseMoved = event->getIf<sf::Event::MouseMoved>()) {
                mousePosition = mouseMoved->position;
            }
        }

        window.clear();
        // Draw chessboard

        for (int r = 0; r < BOARD_SIZE; r++) {
            for (int c = 0; c < BOARD_SIZE; c++) {
                sf::RectangleShape square(sf::Vector2f(TILE_SIZE, TILE_SIZE));
                square.setPosition({
                    static_cast<float>(c * TILE_SIZE),
                    static_cast<float>(r * TILE_SIZE)
                });
                bool light = (r + c) % 2 == 0;
                square.setFillColor(light ? sf::Color(240, 217, 181) : sf::Color(181, 136, 99));
                window.draw(square);

                // Draw piece if present
                Position pos(r, c);
                const Piece* piece = board.getPiece(pos);
                if (piece) {
                    std::string asset = getPieceAsset(piece->type(), piece->color());
                    if (!asset.empty() && textures.count(asset)) {
                        sf::Sprite sprite(textures[asset]);

                        if (r == draggingPieceFrom.row && c == draggingPieceFrom.col) {
                            continue; // draw it at the end
                        }

                        sprite.setPosition({
                            static_cast<float>(c * TILE_SIZE),
                            static_cast<float>(r * TILE_SIZE)
                        });

                        sprite.setScale({
                            TILE_SIZE / static_cast<float>(textures[asset].getSize().x),
                            TILE_SIZE / static_cast<float>(textures[asset].getSize().y)
                        });

                        window.draw(sprite);
                    }
                }
            }
        }

        if (draggingPieceFrom.row != -1 && draggingPieceFrom.col != -1) {

            sf::CircleShape indicator;
            static sf::Color indicatorColor(45, 45, 45, 45);
            indicator.setFillColor(indicatorColor);
            indicator.setRadius(15);
            indicator.setOrigin({15, 15});
            for (const auto& m : draggingMoves) {
                indicator.setPosition({
                    static_cast<float>(m.to.col * TILE_SIZE + TILE_SIZE / 2),
                    static_cast<float>(m.to.row * TILE_SIZE + TILE_SIZE / 2)
                });
                window.draw(indicator);
            }

            const Piece* piece = board.getPiece(draggingPieceFrom);
            if (piece) {
                std::string asset = getPieceAsset(piece->type(), piece->color());
                if (!asset.empty() && textures.count(asset)) {
                    sf::Sprite sprite(textures[asset]);

                    sf::Vector2f worldPos = window.mapPixelToCoords(mousePosition);
                    sprite.setPosition(worldPos);

                    sprite.setScale({
                        TILE_SIZE / static_cast<float>(textures[asset].getSize().x),
                        TILE_SIZE / static_cast<float>(textures[asset].getSize().y)
                    });

                    sprite.setOrigin({TILE_SIZE, TILE_SIZE});

                    window.draw(sprite);
                }
            }
        }
        window.display();
    }
}