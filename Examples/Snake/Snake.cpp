#include "Snake.hpp"

namespace Snake
{
SnakeBody::SnakeBody(const Point& initialPosition)
{
    body.push_back({ initialPosition, headColor });
}

const std::vector<std::pair<Point, ColorPair>>& SnakeBody::GetBody() const
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

    return body[0].first;
}

void SnakeBody::SetInitialPosition(const Point& initialPosition)
{
    if (initialPositionSet)
    {
        return;
    }

    body.clear();
    body.push_back({ initialPosition, headColor });
    initialPositionSet = true;
}

void SnakeBody::UpdatePosition(const Point& delta)
{
    if (body.empty())
    {
        return;
    }

    const Point head{ body[0].first.X + delta.X, body[0].first.Y + delta.Y };
    if (snakeAte)
    {
        // grow by one segment: every segment moves one slot towards the tail (keeping its color)
        // and the segment right behind the head takes the color of what was eaten
        snakeAte = false;
        body.push_back(body.back());
        for (size_t i = body.size() - 2; i > 0; i--)
        {
            body[i] = body[i - 1];
        }
        body[1].second = lastColorAte;
    }
    else
    {
        // same length: positions follow the head, colors stay where they are
        for (size_t i = body.size() - 1; i > 0; i--)
        {
            body[i].first = body[i - 1].first;
        }
    }
    body[0].first = head;
}

void SnakeBody::Ate(const ColorPair& color)
{
    snakeAte     = true;
    lastColorAte = color;
}

const ColorPair& SnakeBody::GetHeadColor() const
{
    return headColor;
}

const ColorPair& SnakeBody::GetBodyColor(const Point& position) const
{
    for (const auto& entry : body)
    {
        if (entry.first == position)
        {
            return entry.second;
        }
    }

    return bodyColor;
}
} // namespace Snake
