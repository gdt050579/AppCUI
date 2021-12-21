#pragma once

#include "Piece.hpp"

#include <random>
#include <array>
#include <deque>

class Board
{
  private:
    std::array<std::array<bool, 50>, 50> matrix{ false };
    int maxtrixVSize  = 0;
    int maxtrixHSize  = 0;
    int matrixXLeft   = 0;
    int matrixXRight  = 0;
    int matrixYTop    = 0;
    int matrixYBottom = 0;

    std::deque<Piece> pieces{};
    std::vector<Piece> piecesProcessed;
    std::optional<Piece> currentPiece;
    unsigned long currentPieceUpdated = -1;

    std::random_device r;
    std::default_random_engine e1{ r() };
    std::uniform_int_distribution<int> uniform_dist{ 0, static_cast<int>(PieceType::End) - 1 };

    int scale = 1;

    unsigned int score = 0;

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
          unsigned long delta);
    void SetMatrixBounds(const Size& size);

    bool Draw(Renderer& renderer);
    bool DrawPieceById(Renderer& renderer, unsigned int id, const Size& size);

    unsigned int GetScore() const;

  private:
    bool CanAdvanceOnYAxis() const;
    bool CanAdvanceOnXAxisLeft() const;
    bool CanAdvanceOnXAxisRight() const;
};
