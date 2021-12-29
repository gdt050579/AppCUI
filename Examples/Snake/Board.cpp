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
    const auto& body = snake.GetBody();
    for (const auto& [point, color] : body)
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
    const auto& body = snake.GetBody();
    for (const auto& [point, color] : body)
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
              [snakeHead](const auto& e) { return e.first == snakeHead; }))
    {
        return IsCollidingOn::Self;
    }

    if (fruit.has_value())
    {
        const auto& fruitPosition = fruit->GetPosition();
        if (fruitPosition == snakeHead)
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

void Board::Update(int scale, const Reference<Control> control, const Size& size, unsigned long delta, bool& gameOver)
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
        slice              = 1000.0 / (level * 2.0);
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
                snake.Ate(fruit->GetColor());
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
                    renderer.FillRectSize(position.X, position.Y, w, h, ' ', snake.GetHeadColor());

                    Point leftEye{ position.X + 1, position.Y + 1 };
                    Point rightEye{ position.X + w - 2, position.Y + 1 };
                    SpecialChars eye{ SpecialChars::TriangleUp };

                    switch (direction)
                    {
                    case Snake::Board::HeadingTo::Left:
                        eye      = SpecialChars::TriangleLeft;
                        leftEye  = { position.X + 2, position.Y };
                        rightEye = { position.X + 2, position.Y + h - 1 };
                        break;
                    case Snake::Board::HeadingTo::Right:
                        leftEye  = { position.X + w - 2, position.Y };
                        rightEye = { position.X + w - 2, position.Y + h - 1 };
                        eye      = SpecialChars::TriangleRight;
                        break;
                    case Snake::Board::HeadingTo::Up:
                        eye      = SpecialChars::TriangleUp;
                        leftEye  = { position.X + 1, position.Y + 1 };
                        rightEye = { position.X + w - 2, position.Y + 1 };
                        break;
                    case Snake::Board::HeadingTo::Down:
                        eye      = SpecialChars::TriangleDown;
                        leftEye  = { position.X + 1, position.Y + h - 2 };
                        rightEye = { position.X + w - 2, position.Y + h - 2 };
                        break;
                    case Snake::Board::HeadingTo::None:
                        break;
                    default:
                        break;
                    }

                    renderer.WriteSpecialCharacter(leftEye.X, leftEye.Y, eye, { Color::White, Color::Transparent });
                    renderer.WriteSpecialCharacter(rightEye.X, rightEye.Y, eye, { Color::White, Color::Transparent });
                }
                else if (std::any_of(
                               snakeBody.begin(),
                               snakeBody.end(),
                               [x, y](const auto& e) { return e.first.X == x && e.first.Y == y; }))
                {
                    const auto& color = snake.GetBodyColor({ x, y });
                    renderer.FillRectSize(position.X, position.Y, w, h, ' ', color);
                    renderer.DrawRectSize(position.X, position.Y, w, h, color, LineType::Single);
                }
                else if (y == fruitPosition.Y && x == fruitPosition.X)
                {
                    const auto wQuad = w / 4;
                    const auto hQuad = h / 4;

                    renderer.FillRectSize(
                          position.X + wQuad, position.Y + hQuad, wQuad * 2, hQuad * 2, ' ', fruit->GetColor());
                    renderer.DrawRectSize(
                          position.X, position.Y, w, h, { Color::White, Color::Transparent }, LineType::Single);
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
