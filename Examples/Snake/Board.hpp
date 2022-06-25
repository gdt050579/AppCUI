#pragma once

#include <AppCUI.hpp>

#include "Snake.hpp"
#include "Fruit.hpp"

#include <random>
#include <array>
#include <deque>

namespace Snake
{
using namespace AppCUI::Utils;
using namespace AppCUI::Graphics;
using namespace AppCUI::Controls;

constexpr auto boardSize = 50;

class Board
{
  public:
    enum class HeadingTo
    {
        Left,
        Right,
        Up,
        Down,
        None
    };

  private:
    std::array<std::array<bool, boardSize>, boardSize> matrix{ { false } };
    int matrixVSize = 0;
    int matrixHSize = 0;

    Size size{ 1, 1 }; // block size on canvas

    SnakeBody snake{ { 0, 0 } };
    double slice          = 0;
    double snakeUpdated   = -1;
    bool directionUpdated = false;

    std::optional<Fruit> fruit;

    std::random_device rd;
    std::default_random_engine dre{ rd() };
    std::uniform_int_distribution<int> uidFruitColor{ 0, static_cast<int>(Color::Transparent) - 1 };

    double scale = 0.5;

    unsigned int score                    = 0;
    unsigned int level                    = 1;
    const unsigned int deltaLevelIncrease = 30 * CLOCKS_PER_SEC;

    enum class IsCollidingOn
    {
        Left,
        Right,
        Top,
        Bottom,
        Self,
        Fruit,
        None
    };

    HeadingTo direction = HeadingTo::Left;

  public:
    void Update(int scale, const Reference<Control> control, const Size& size, unsigned long delta, bool& gameOver);
    void SetMatrixBounds(const Size& canvasSize);

    bool Draw(Renderer& renderer, const Size& canvasSize);

    unsigned int GetScore() const;
    unsigned int GetLevel() const;

    bool SetDirection(HeadingTo direction);

  private:
    bool ClearSnakeFootprint();
    bool AddSnakeFootprint();

    IsCollidingOn IsColliding();

    void GenerateFruit();
};
} // namespace Snake
