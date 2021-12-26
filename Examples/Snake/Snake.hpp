#pragma once

#include <AppCUI.hpp>

#include <vector>

namespace Snake
{
using namespace AppCUI::Utils;
using namespace AppCUI::Graphics;

class SnakeBody
{
  private:
    std::vector<Point> body;
    bool initialPositionSet = false;

    ColorPair headColor{ Color::White, Color::Transparent };
    ColorPair bodyColor{ Color::White, Color::Transparent };

  public:
    SnakeBody(const Point& initialPosition);

    void SetInitialPosition(const Point& initialPosition);
    void UpdatePosition(const Point& delta);

    const std::vector<Point>& GetBody() const;
    const Point& GetPositionOnBoard() const;
    const ColorPair& GetHeadColor() const;
    const ColorPair& GetBodyColor() const;
};
} // namespace Snake
