#include "Board.hpp"

namespace Snake
{
bool Board::SetDirection(HeadingTo direction)
{
    // you can't change direction 180 degrees
    if (this->direction == HeadingTo::Down && direction == HeadingTo::Up ||
        direction == HeadingTo::Down && this->direction == HeadingTo::Up ||
        direction == HeadingTo::Left && this->direction == HeadingTo::Right ||
        this->direction == HeadingTo::Left && direction == HeadingTo::Right)
    {
        return false;
    }

    this->direction = direction;

    return true;
}

bool Board::SetNextCurrentPiece()
{
    // TODO: fruit

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

    // TODO:

    return IsCollidingOn::None;
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
    level    = static_cast<unsigned int>(delta / deltaSecondsToIncreaseLevel);
    if (level == 0)
    {
        level++;
    }

    this->scale = scale;
    SetMatrixBounds(size);

    if (matrixHSize != 0 && matrixVSize != 0)
    {
        snake.SetInitialPosition({ matrixHSize / 2, matrixVSize / 2 });
    }

    IsCollidingOn collided = IsCollidingOn::None;
    if (delta != snakeUpdated)
    {
        slice              = 1.0 / (level * 2.0);
        double deltaUpdate = delta - snakeUpdated;
        if (deltaUpdate > slice)
        {
            for (auto i = 0.0; i < deltaUpdate / slice - 1; i++)
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
                if (collided != IsCollidingOn::None && collided != IsCollidingOn::Fruit)
                {
                    break;
                }
            }

            ClearSnakeFootprint();
            AddSnakeFootprint();

            snakeUpdated = delta;
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
    matrixHSize           = panelWidth / w;
    matrixXLeft           = 1 + (panelWidth % w) / 2;
    matrixXRight          = panelWidth - matrixXLeft;

    // compute # of squares on vertical
    const auto h           = static_cast<int>(size.Height * scale);
    const auto panelHeight = canvasSize.Height;
    matrixVSize            = panelHeight / h;
    matrixYTop             = 1 + (panelHeight % h) / 2;
    matrixYBottom          = panelHeight - matrixYTop;
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
    Point position        = { xStart, yStart };

    for (auto y = 0; y < matrixVSize; y++)
    {
        for (auto x = 0; x < matrixHSize; x++)
        {
            if (matrix[y][x] == true)
            {
                if (y == snakeHead.Y && x == snakeHead.X)
                {
                    renderer.DrawRectSize(position.X, position.Y, w, h, snake.GetHeadColor(), true);
                }
                else if (std::any_of(
                               snakeBody.begin(),
                               snakeBody.end(),
                               [x, y](const Point& p) { return p.X == x && p.Y == y; }))
                {
                    renderer.DrawRectSize(position.X, position.Y, w, h, snake.GetBodyColor(), false);
                }
                // TODO:
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
