#include "Board.hpp"

namespace Snake
{
bool Board::SetDirection(HeadingTo direction)
{
    CHECK(directionUpdated, false, "");

    // you can't change direction 180 degrees
    if (this->direction == HeadingTo::Down && direction == HeadingTo::Up ||
        direction == HeadingTo::Down && this->direction == HeadingTo::Up ||
        direction == HeadingTo::Left && this->direction == HeadingTo::Right ||
        this->direction == HeadingTo::Left && direction == HeadingTo::Right)
    {
        return false;
    }

    this->direction  = direction;
    directionUpdated = false;

    return true;
}

bool Board::ClearSnakeFootprint()
{
    const auto& position = snake.GetPositionOnBoard();
    const auto& body     = snake.GetBody();
    for (const auto& point : body)
    {
        if (point.X < 0 || point.Y < 0)
        {
            continue;
        }
        matrix[point.Y][point.X] = false;
    }

    return true;
}

bool Board::AddSnakeFootprint()
{
    const auto& position = snake.GetPositionOnBoard();
    const auto& body     = snake.GetBody();
    for (const auto& point : body)
    {
        if (point.X < 0 || point.Y < 0)
        {
            continue;
        }
        matrix[point.Y][point.X] = true;
    }

    return true;
}

Board::IsCollidingOn Board::IsColliding()
{
    const auto& snakeHead = snake.GetPositionOnBoard();

    CHECK(snakeHead.X > -1, IsCollidingOn::Left, "");
    CHECK(snakeHead.X < matrixHSize, IsCollidingOn::Right, "");
    CHECK(snakeHead.Y > -1, IsCollidingOn::Top, "");
    CHECK(snakeHead.Y < matrixVSize, IsCollidingOn::Bottom, "");

    const auto& snakeBody = snake.GetBody();

    if (std::any_of(
              snakeBody.begin() + 1, // skip the head
              snakeBody.end(),
              [snakeHead](const Point& p) { return p.X == snakeHead.X && p.Y == snakeHead.Y; }))
    {
        return IsCollidingOn::Self;
    }

    if (fruit.has_value())
    {
        const auto& fruitPosition = fruit->GetPosition();
        if (fruitPosition.X == snakeHead.X && fruitPosition.Y == snakeHead.Y)
        {
            return IsCollidingOn::Fruit;
        }
    }

    return IsCollidingOn::None;
}

void Board::GenerateFruit()
{
    std::uniform_int_distribution<int> uidMapH{ 0, std::min<>(matrixHSize - 1, boardSize - 1) };
    std::uniform_int_distribution<int> uidMapV{ 0, std::min<>(matrixVSize - 1, boardSize - 1) };

    Point p{ -1, -1 };
    do
    {
        p.X = uidMapH(dre);
        p.Y = uidMapV(dre);
    } while (matrix[p.Y][p.X]);

    if (fruit.has_value())
    {
        const auto& pOld       = fruit->GetPosition();
        matrix[pOld.Y][pOld.X] = false;
    }
    fruit.emplace(Fruit{ p, static_cast<Color>(uidFruitColor(dre)) });
    matrix[p.Y][p.X] = true;
}

void Board::Update(
      int scale,
      unsigned int maxPiecesInQueue,
      const Reference<Control> control,
      const Size& size,
      unsigned long delta,
      bool& gameOver)
{
    gameOver = false;
    level    = static_cast<unsigned int>(delta / deltaLevelIncrease) + 1;

    this->scale = scale;
    SetMatrixBounds(size);

    if (matrixHSize != 0 && matrixVSize != 0)
    {
        snake.SetInitialPosition({ matrixHSize / 2, matrixVSize / 2 });

        if (fruit.has_value() == false)
        {
            GenerateFruit();
        }
    }

    IsCollidingOn collided = IsCollidingOn::None;
    if (delta != snakeUpdated || snakeUpdated == -1)
    {
        // TODO: I might need to trigger a redraw here in order for slice to actually work..
        slice              = 1.0 / (level * 2.0);
        double deltaUpdate = delta - snakeUpdated;
        if (deltaUpdate > slice)
        {
            switch (direction)
            {
            case Snake::Board::HeadingTo::Left:
                snake.UpdatePosition({ -1, 0 });
                break;
            case Snake::Board::HeadingTo::Right:
                snake.UpdatePosition({ 1, 0 });
                break;
            case Snake::Board::HeadingTo::Up:
                snake.UpdatePosition({ 0, -1 });
                break;
            case Snake::Board::HeadingTo::Down:
                snake.UpdatePosition({ 0, 1 });
                break;
            case Snake::Board::HeadingTo::None:
                break;
            default:
                break;
            }

            collided = IsColliding();

            if (collided == IsCollidingOn::Fruit)
            {
                snake.Ate();
                score += fruit->GetScore();
                GenerateFruit();
            }

            ClearSnakeFootprint();
            AddSnakeFootprint();

            snakeUpdated = delta;

            directionUpdated = true;
        }
    }

    if (collided != IsCollidingOn::None && collided != IsCollidingOn::Fruit)
    {
        gameOver = true;
    }
}

void Board::SetMatrixBounds(const Size& canvasSize)
{
    // compute # of squares on horizontal
    const auto w          = static_cast<int>(size.Width * scale * 2);
    const auto panelWidth = canvasSize.Width;
    matrixHSize           = std::min<>(static_cast<int>(panelWidth / w), boardSize);

    // compute # of squares on vertical
    const auto h           = static_cast<int>(size.Height * scale);
    const auto panelHeight = canvasSize.Height;
    matrixVSize            = std::min<>(static_cast<int>(panelHeight / h), boardSize);
}

bool Board::Draw(Renderer& renderer, const Size& canvasSize)
{
    CHECK(matrixVSize > 0 && matrixHSize > 0, false, "");

    const auto w = static_cast<int>(size.Width * scale * 2);
    const auto h = static_cast<int>(size.Height * scale);

    const int xStart = 1 + ((canvasSize.Width - 2) % w) / 2;
    const int yStart = 1 + ((canvasSize.Height - 2) % h) / 2;

    const auto& snakeHead = snake.GetPositionOnBoard();
    const auto& snakeBody = snake.GetBody();
    auto fruitPosition    = Point{ -1, -1 };
    if (fruit.has_value())
    {
        fruitPosition = fruit->GetPosition();
    }
    Point position = { xStart, yStart };

    for (auto y = 0; y < matrixVSize; y++)
    {
        for (auto x = 0; x < matrixHSize; x++)
        {
            if (matrix[y][x] == true)
            {
                if (y == snakeHead.Y && x == snakeHead.X)
                {
                    const auto wQuad = w / 4;
                    const auto hQuad = h / 4;
                    renderer.FillRectSize(
                          position.X + wQuad,
                          position.Y + hQuad,
                          wQuad * 2,
                          hQuad * 2,
                          ' ',
                          { Color::White, Color::White });
                    renderer.DrawRectSize(position.X, position.Y, w, h, snake.GetHeadColor(), true);
                }
                else if (std::any_of(
                               snakeBody.begin(),
                               snakeBody.end(),
                               [x, y](const Point& p) { return p.X == x && p.Y == y; }))
                {
                    renderer.DrawRectSize(position.X, position.Y, w, h, snake.GetBodyColor(), false);
                }
                else if (y == fruitPosition.Y && x == fruitPosition.X)
                {
                    const auto wQuad = w / 4;
                    const auto hQuad = h / 4;

                    renderer.FillRectSize(
                          position.X + wQuad, position.Y + hQuad, wQuad * 2, hQuad * 2, ' ', fruit->GetColor());
                    renderer.DrawRectSize(position.X, position.Y, w, h, { Color::White, Color::Transparent }, false);
                }
            }

            position.X += w;
        }

        position.X = xStart;
        position.Y += h;
    }

    return true;
}

unsigned int Board::GetScore() const
{
    return score;
}

unsigned int Board::GetLevel() const
{
    return level;
}
} // namespace Snake
