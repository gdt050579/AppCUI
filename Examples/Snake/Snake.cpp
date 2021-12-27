#include "Snake.hpp"

namespace Snake
{
SnakeBody::SnakeBody(const Point& initialPosition)
{
    body.emplace_back(initialPosition);
}

const std::vector<Point>& SnakeBody::GetBody() const
{
    return body;
}

const Point& SnakeBody::GetPositionOnBoard() const
{
    if (body.size() == 0)
    {
        const static Point p{ 0, 0 };
        return p;
    }

    return body[0];
}

void SnakeBody::SetInitialPosition(const Point& initialPosition)
{
    if (initialPositionSet)
    {
        return;
    }

    body.clear();
    body.emplace_back(initialPosition);
    initialPositionSet = true;
}
void SnakeBody::UpdatePosition(const Point& delta)
{
    const Point p{ body[0].X + delta.X, body[0].Y + delta.Y };
    body.insert(body.begin(), p);

    if (snakeAte == false)
    {
        body.pop_back();
    }
    else
    {
        snakeAte = false;
    }
}
void SnakeBody::Ate()
{
    snakeAte = true;
}
const ColorPair& SnakeBody::GetHeadColor() const
{
    return headColor;
}
const ColorPair& SnakeBody::GetBodyColor() const
{
    return bodyColor;
}
} // namespace Snake
