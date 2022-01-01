#include "Board.hpp"

namespace Tetris
{
bool Board::CanAdvanceOnYAxis()
{
    CHECK(currentPiece.has_value(), false, "");

    const auto& positionOnBoard       = currentPiece->GetPositionOnBoard();
    const Point futurePositionOnBoard = { positionOnBoard.X, positionOnBoard.Y + 1 };

    return IsColliding(*currentPiece, futurePositionOnBoard) != IsCollidingOn::Bottom;
}

bool Board::CanAdvanceOnXAxisLeft()
{
    CHECK(currentPiece.has_value(), false, "");

    const auto& positionOnBoard       = currentPiece->GetPositionOnBoard();
    const Point futurePositionOnBoard = { positionOnBoard.X - 1, positionOnBoard.Y };

    return IsColliding(*currentPiece, futurePositionOnBoard) != IsCollidingOn::Left;
}

bool Board::CanAdvanceOnXAxisRight()
{
    CHECK(currentPiece.has_value(), false, "");

    const auto& positionOnBoard       = currentPiece->GetPositionOnBoard();
    const Point futurePositionOnBoard = { positionOnBoard.X + 1, positionOnBoard.Y };

    return IsColliding(*currentPiece, futurePositionOnBoard) != IsCollidingOn::Right;
}

bool Board::SetNextCurrentPiece()
{
    CHECK(pieces.size() > 0, true, "");
    CHECK(matrixHSize > 0 && matrixVSize > 0, false, "");

    currentPiece.emplace(pieces.front());
    pieces.pop_front();

    const Point position{ matrixHSize / 2 - 1, 0 };
    currentPiece->SetPositionOnBoard(position);
    CHECK(IsColliding(position) != IsCollidingOn::Spawn, false, "");

    AddPieceFootprint();

    return true;
}

bool Board::ClearPieceFootprint()
{
    CHECK(currentPiece.has_value(), false, "");
    return ClearPieceFootprint(*currentPiece);
}

bool Board::ClearPieceFootprint(const Piece& piece)
{
    const auto& positionOnBoard = piece.GetPositionOnBoard();
    const auto& pieceMatrix     = piece.GetMatrix();
    for (auto y = 0ULL; y < pieceMatrix.size(); y++)
    {
        for (auto x = 0ULL; x < pieceMatrix[y].size(); x++)
        {
            if (pieceMatrix[y][x] == true)
            {
                matrix[positionOnBoard.Y + y][positionOnBoard.X + x].first  = false;
                matrix[positionOnBoard.Y + y][positionOnBoard.X + x].second = Color::White;
            }
        }
    }

    return true;
}

bool Board::AddPieceFootprint()
{
    CHECK(currentPiece.has_value(), false, "");
    return AddPieceFootprint(*currentPiece);
}

bool Board::AddPieceFootprint(const Piece& piece)
{
    const auto& positionOnBoard = piece.GetPositionOnBoard();
    const auto& pieceMatrix     = piece.GetMatrix();
    for (auto y = 0ULL; y < pieceMatrix.size(); y++)
    {
        for (auto x = 0ULL; x < pieceMatrix[y].size(); x++)
        {
            if (pieceMatrix[y][x] == true)
            {
                matrix[positionOnBoard.Y + y][positionOnBoard.X + x].first  = true;
                matrix[positionOnBoard.Y + y][positionOnBoard.X + x].second = piece.GetBodyColor().Foreground;
            }
        }
    }

    return true;
}

Board::IsCollidingOn Board::IsColliding(const Point& futurePosition)
{
    CHECK(currentPiece.has_value(), IsCollidingOn::None, "");
    return IsColliding(*currentPiece, futurePosition);
}

Board::IsCollidingOn Board::IsColliding(const Piece& piece, const Point& futurePosition)
{
    // if future position is current position means that the piece is generated on the board
    // should not clear current board data in order to check if it collides with something existent
    // -> game over
    const auto& currentPosition = piece.GetPositionOnBoard();
    if (futurePosition.X != currentPosition.X || futurePosition.Y != currentPosition.Y)
    {
        ClearPieceFootprint(piece);
    }

    const auto& pieceMatrix = piece.GetMatrix();
    for (auto y = 0LL; y < static_cast<int64_t>(pieceMatrix.size()); y++)
    {
        for (auto x = 0LL; x < static_cast<int64_t>(pieceMatrix[y].size()); x++)
        {
            if (pieceMatrix[y][x] == true)
            {
                if (futurePosition.X + x >= matrixHSize)
                {
                    AddPieceFootprint(piece);
                    return IsCollidingOn::Right;
                }

                if (futurePosition.Y + y >= matrixVSize)
                {
                    AddPieceFootprint(piece);
                    return IsCollidingOn::Bottom;
                }

                if (futurePosition.X + x < 0)
                {
                    AddPieceFootprint(piece);
                    return IsCollidingOn::Left;
                }

                if (futurePosition.Y + y < 0)
                {
                    AddPieceFootprint(piece);
                    return IsCollidingOn::Top;
                }

                if (matrix[futurePosition.Y + y][futurePosition.X + x].first == true)
                {
                    const auto xDelta = futurePosition.X - currentPosition.X;
                    const auto yDelta = futurePosition.Y - currentPosition.Y;

                    if (xDelta == 0 && yDelta == 0)
                    {
                        AddPieceFootprint(piece);
                        return IsCollidingOn::Spawn;
                    }

                    if (xDelta > 0)
                    {
                        AddPieceFootprint(piece);
                        return IsCollidingOn::Right;
                    }
                    else if (xDelta < 0)
                    {
                        AddPieceFootprint(piece);
                        return IsCollidingOn::Left;
                    }

                    if (yDelta > 0)
                    {
                        AddPieceFootprint(piece);
                        return IsCollidingOn::Bottom;
                    }
                    else if (yDelta < 0)
                    {
                        AddPieceFootprint(piece);
                        return IsCollidingOn::Top;
                    }
                }
            }
        }
    }

    if (futurePosition.X != currentPosition.X || futurePosition.Y != currentPosition.Y)
    {
        AddPieceFootprint(piece);
    }

    return IsCollidingOn::None;
}

bool Board::HandleLineCompletion()
{
    bool linesCompletedFound = false;
    std::vector<unsigned int> rowsCompleted{};
    rowsCompleted.reserve(boardSize);

    auto y = 0;
    for (const auto& row : matrix)
    {
        if (std::all_of(
                  row.begin(), row.begin() + matrixHSize, [](std::pair<bool, Color> value) { return value.first; }))
        {
            rowsCompleted.push_back(y);
        }
        y++;
    }

    score += static_cast<unsigned int>(rowsCompleted.size() * rowsCompleted.size());

    for (const auto row : rowsCompleted)
    {
        if (row == 0)
        {
            matrix[0] = { std::pair<bool, Color>{ false, Color::White } };
        }

        for (auto y = row; y > 0; y--)
        {
            matrix[y] = matrix[y - 1ULL];
        }
    }

    return linesCompletedFound;
}

bool Board::AdvanceOnYAxis()
{
    CHECK(CanAdvanceOnYAxis(), false, "");

    ClearPieceFootprint();
    currentPiece->UpdatePositionOnBoard({ 0, 1 });
    AddPieceFootprint();

    return true;
}

bool Board::AdvanceOnXAxisLeft()
{
    CHECK(CanAdvanceOnXAxisLeft(), false, "");

    ClearPieceFootprint();
    currentPiece->UpdatePositionOnBoard({ -1, 0 });
    AddPieceFootprint();

    return true;
}

bool Board::AdvanceOnXAxisRight()
{
    CHECK(CanAdvanceOnXAxisRight(), false, "");

    ClearPieceFootprint();
    currentPiece->UpdatePositionOnBoard({ 1, 0 });
    AddPieceFootprint();

    return true;
}

bool Board::Rotate()
{
    CHECK(currentPiece.has_value(), false, "");

    ClearPieceFootprint();
    currentPiece->Rotate({ static_cast<unsigned int>(matrixHSize), static_cast<unsigned int>(matrixVSize) });
    AddPieceFootprint();

    return true;
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

    this->scale = scale;
    SetMatrixBounds(size);

    while (pieces.size() < maxPiecesInQueue)
    {
        const auto pieceType  = static_cast<PieceType>(uid(dre));
        const auto pieceColor = colorMap.at(uidPieceColor(dre));
        pieces.emplace_back(Piece{ pieceType, control, { pieceColor, pieceColor } });
    }

    if (currentPiece.has_value())
    {
        if (CanAdvanceOnYAxis() == false)
        {
            currentPiece.reset();

            // check for Y lines
            HandleLineCompletion();
        }
        else if (currentPieceUpdated != delta)
        {
            currentPieceUpdated = delta;
            if (trainingMode == false)
            {
                AdvanceOnYAxis();
            }
        }
    }
    else
    {
        gameOver = (SetNextCurrentPiece() == false);
    }
}

void Board::SetMatrixBounds(const Size& canvasSize)
{
    // compute # of squares on horizontal
    const auto w          = size.Width * scale * 2;
    const auto panelWidth = canvasSize.Width;
    matrixHSize           = panelWidth / w;
    matrixXLeft           = 1 + (panelWidth % w) / 2;
    matrixXRight          = panelWidth - matrixXLeft;

    // compute # of squares on vertical
    const auto h           = size.Height * scale;
    const auto panelHeight = canvasSize.Height;
    matrixVSize            = panelHeight / h;
    matrixYTop             = 1 + (panelHeight % h) / 2;
    matrixYBottom          = panelHeight - matrixYTop;
}

bool Board::Draw(Renderer& renderer, const Size& canvasSize)
{
    CHECK(matrixVSize > 0 && matrixHSize > 0, false, "");

    const auto w = size.Width * scale * 2;
    const auto h = size.Height * scale;

    const int xStart = 1 + ((canvasSize.Width - 2) % w) / 2;
    const int yStart = 1 + ((canvasSize.Height - 2) % h) / 2;

    Point position = { xStart, yStart };

    for (auto y = 0; y < matrixVSize; y++)
    {
        for (auto x = 0; x < matrixHSize; x++)
        {
            // renderer.DrawRectSize(position.X, position.Y, w, h, colorBorder, LineType::Single);

            if (matrix[y][x].first == true)
            {
                renderer.FillRectSize(
                      position.X + 1, position.Y + 1, w - 1, h - 1, ' ', { Color::Transparent, matrix[y][x].second });
            }

            position.X += w;
        }

        position.X = xStart;
        position.Y += h;
    }

    return true;
}

bool Board::DrawPieceById(Renderer& renderer, unsigned int id, const Size& canvasSize, int scale)
{
    CHECK(pieces.size() > id, false, "Invalid id: [%u]!", id);

    auto& piece     = pieces[id];
    const auto size = piece.GetSize(scale);
    const Point position{ std::max<>(1, static_cast<int>((canvasSize.Width - size.Width) / 2)),
                          std::max<>(1, static_cast<int>((canvasSize.Height - 2 - size.Height) / 2)) };
    piece.Draw(renderer, scale, position);

    return true;
}

unsigned int Board::GetScore() const
{
    return score;
}

void Board::ToggleTrainingMode()
{
    trainingMode = !trainingMode;
}

bool Board::GetTrainingMode() const
{
    return trainingMode;
}
} // namespace Tetris
