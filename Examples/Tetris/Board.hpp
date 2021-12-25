#pragma once

#include "Piece.hpp"

#include <random>
#include <array>
#include <deque>

namespace Tetris
{
constexpr auto boardSize = 50U;

class Board
{
  private:
    std::array<std::array<bool, boardSize>, boardSize> matrix{ false };
    int matrixVSize   = 0;
    int matrixHSize   = 0;
    int matrixXLeft   = 0;
    int matrixXRight  = 0;
    int matrixYTop    = 0;
    int matrixYBottom = 0;

    Size size{ 1, 1 }; // block size on canvas
    ColorPair color{ Color::White, Color::Transparent };

    std::deque<Piece> pieces{};
    std::optional<Piece> currentPiece;
    unsigned long currentPieceUpdated = -1;

    std::random_device rd;
    std::default_random_engine dre{ rd() };
    std::uniform_int_distribution<int> uid{ 0, static_cast<int>(PieceType::End) - 1 };

    int scale = 1;

    unsigned int score = 0;

    enum class IsCollidingOn
    {
        Left,
        Right,
        Top,
        Bottom,
        Spawn,
        None
    };

  public:
    bool AdvanceOnYAxis();
    bool AdvanceOnXAxisLeft();
    bool AdvanceOnXAxisRight();
    bool Rotate();

    void Update(
          int scale,
          unsigned int maxPiecesInQueue,
          const Reference<Control> control,
          const Size& size,
          unsigned long delta,
          bool& gameOver);
    void SetMatrixBounds(const Size& canvasSize);

    bool Draw(Renderer& renderer, const Size& canvasSize);
    bool DrawPieceById(Renderer& renderer, unsigned int id, const Size& canvasSize, int scale);

    unsigned int GetScore() const;

  private:
    bool CanAdvanceOnYAxis();
    bool CanAdvanceOnXAxisLeft();
    bool CanAdvanceOnXAxisRight();

    bool SetNextCurrentPiece();
    bool ClearPieceFootprint();
    bool ClearPieceFootprint(const Piece& piece);
    bool AddPieceFootprint();
    bool AddPieceFootprint(const Piece& piece);

    IsCollidingOn IsColliding(const Point& futurePosition);
    IsCollidingOn IsColliding(const Piece& piece, const Point& futurePosition);

    bool HandleLineCompletion();
};
} // namespace Tetris
