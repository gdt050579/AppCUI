#pragma once

#include "AppCUI.hpp"
#include <array>

namespace Tetris
{
using namespace AppCUI::Utils;
using namespace AppCUI::Graphics;
using namespace AppCUI::Controls;

enum class PieceType
{
    I = 0,
    O = 1,
    T = 2,
    S = 3,
    L = 4,
    Z = 5,
    J = 6,

    End
};

class Piece
{
  private:
    static const unsigned int rows    = 4;
    static const unsigned int columns = 4;
    Size size{ 1, 1 }; // on canvas
    ColorPair color{ Color::White, Color::Transparent };
    std::array<std::array<bool, rows>, columns> matrix{ { { false } } };
    Reference<Control> control = nullptr;
    const PieceType type;
    Point positionOnBoard{ 0, 0 };

  public:
    Piece(const PieceType type, const Reference<Control> control, const Point& position);

    bool Draw(Renderer& renderer, int scale, const Point& position);
    Size GetSize(int scale) const;
    int GetBlockWidth(int scale) const;
    int GetBlockHeight(int scale) const;
    int GetLeftXPosition() const;
    int GetRightXPosition(int scale) const;
    int GetTopYPosition() const;
    int GetBottomYPosition(int scale) const;
    bool Rotate();

    const std::array<std::array<bool, rows>, columns>& GetMatrix() const;
    const Point& GetPositionOnBoard() const;

    void SetPositionOnBoard(const Point& position);
    void UpdatePositionOnBoard(const Point& position);
};
} // namespace Tetris
