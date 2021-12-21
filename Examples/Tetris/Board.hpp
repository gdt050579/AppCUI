#pragma once

#include "Piece.hpp"

#include <array>

class Board
{
  public:
    std::array<std::array<bool, 50>, 50> matrix{ false };
    int maxtrixVSize  = 0;
    int maxtrixHSize  = 0;
    int matrixXLeft   = 0;
    int matrixXRight  = 0;
    int matrixYTop    = 0;
    int matrixYBottom = 0;

    bool CanAdvanceOnYAxis(const Piece& piece, int scale) const;
    bool CanAdvanceOnXAxisLeft(const Piece& piece, int scale) const;
    bool CanAdvanceOnXAxisRight(const Piece& piece, int scale) const;
    void AdvanceOnYAxis(Piece& piece, int scale);
    void AdvanceOnXAxisLeft(Piece& piece, int scale);
    void AdvanceOnXAxisRight(Piece& piece, int scale);
};
