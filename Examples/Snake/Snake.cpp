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
    const Point p{ body[0].first.X + delta.X, body[0].first.Y + delta.Y };
    const ColorPair c{ body[0].second };
    body.insert(body.begin(), { p, c });

    if (snakeAte)
    {
        snakeAte       = false;
        body[1].second = lastColorAte;
    }
    else // pass the color before popping
    {        
        for (auto i = 0; i < body.size() - 1; i++)
        {
            auto& entry  = body[i];
            auto& nEntry = body[i + 1ULL];
            entry.second = nEntry.second;
        }

        body.pop_back();
    }
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
