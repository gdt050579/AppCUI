#include "Fruit.hpp"

namespace Snake
{
Fruit::Fruit(const Point& position, const Color& color)
{
    this->position         = position;
    this->color.Background = color;
    this->score            = static_cast<int>(color);
}

void Fruit::SetPosition(const Point& position)
{
    this->position = position;
}

const Point& Fruit::GetPosition() const
{
    return position;
}

void Fruit::SetColor(const Color& color)
{
    this->color.Background = color;
}

const ColorPair& Fruit::GetColor() const
{
    return color;
}
const int Fruit::GetScore() const
{
    return score;
}
} // namespace Snake
