#pragma once

#include <AppCUI.hpp>

namespace Snake
{
using namespace AppCUI::Graphics;

class Fruit
{
  private:
    Point position{ 0, 0 };
    ColorPair color{ Color::White, Color::Transparent };
    int score = 0;
  public:
    Fruit(const Point& position, const Color& color);

    void SetPosition(const Point& position);
    const Point& GetPosition() const;

    void SetColor(const Color& color);
    const ColorPair& GetColor() const;

    const int GetScore() const;
};
} // namespace Snake
