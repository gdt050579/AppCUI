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
    std::vector<std::pair<Point, ColorPair>> body;
    bool initialPositionSet = false;

    ColorPair headColor{ Color::Gray, Color::DarkRed };
    ColorPair bodyColor{ Color::White, Color::White };

    bool snakeAte = false;
    ColorPair lastColorAte{};

  public:
    SnakeBody(const Point& initialPosition);

    void SetInitialPosition(const Point& initialPosition);
    void UpdatePosition(const Point& delta);
    void Ate(const ColorPair& color);

    const std::vector<std::pair<Point, ColorPair>>& GetBody() const;
    const Point& GetPositionOnBoard() const;
    const ColorPair& GetHeadColor() const;
    const ColorPair& GetBodyColor(const Point& position) const;
};
} // namespace Snake
