#include "Piece.hpp"

Piece::Piece(const PieceType type, const AppCUI::Utils::Reference<AppCUI::Controls::Control> control, int x, int y)
    : control(control), type(type), x(x), y(x)
{
    switch (type)
    {
    case PieceType::I:
        matrix[0][0] = 1;
        matrix[1][0] = 1;
        matrix[2][0] = 1;
        matrix[3][0] = 1;
        break;
    case PieceType::O:
        matrix[0][0] = 1;
        matrix[0][1] = 1;
        matrix[1][0] = 1;
        matrix[1][1] = 1;
        break;
    case PieceType::T:
        matrix[0][0] = 1;
        matrix[1][0] = 1;
        matrix[2][0] = 1;
        matrix[1][1] = 1;
        break;
    case PieceType::S:
        matrix[0][1] = 1;
        matrix[0][2] = 1;
        matrix[1][0] = 1;
        matrix[1][1] = 1;
        break;
    case PieceType::L:
        matrix[0][0] = 1;
        matrix[1][0] = 1;
        matrix[1][1] = 1;
        matrix[1][2] = 1;
        break;
    case PieceType::Z:
        matrix[0][0] = 1;
        matrix[0][1] = 1;
        matrix[1][1] = 1;
        matrix[1][2] = 1;
        break;
    case PieceType::J:
        matrix[0][2] = 1;
        matrix[1][0] = 1;
        matrix[1][1] = 1;
        matrix[1][2] = 1;
        break;
    default:
        break;
    }
}

bool Piece::Draw(AppCUI::Graphics::Renderer& renderer, int scale, bool center, int w, int h)
{
    if (center)
    {
        if (w < 2 || h < 2)
        {
            return false;
        }

        const auto size = GetSize(scale);
        const int x     = std::max<>(1, static_cast<int>((w - size.Width) / 2));
        const int y     = std::max<>(1, static_cast<int>(static_cast<int>((h - 2 - size.Height)) / 2));
        SetPosition(x, y);

        return Draw(renderer, scale);
    }

    SetPosition(1, 1);
    return Draw(renderer, scale);
}

bool Piece::Draw(AppCUI::Graphics::Renderer& renderer, int scale)
{
    int xx = x;
    int yy = y;

    for (auto i = 0U; i < cells; i++)
    {
        for (auto j = 0U; j < cells; j++)
        {
            if (matrix[i][j] == 1)
            {
                renderer.FillRectSize(xx, yy, GetBlockWidth(scale), GetBlockHeight(scale), '-', color);
            }
            else
            {
                renderer.DrawRectSize(xx, yy, GetBlockWidth(scale), GetBlockHeight(scale), color, false);
            }

            xx += width * scale * 2;
        }

        xx = x;
        yy += height * scale;
    }

    return true;
}

AppCUI::Graphics::Size Piece::GetSize(int scale) const
{
    const auto w = width * scale * 2;
    const auto h = height * scale;

    switch (type)
    {
    case PieceType::I:
        return { w, h * 3 };
    case PieceType::O:
        return { w * 2, h };
    case PieceType::T:
        return { w * 2, h * 2 };
    case PieceType::S:
        return { w * 3, h };
    case PieceType::L:
        return { w * 3, h };
    case PieceType::Z:
        return { w * 3, h };
    case PieceType::J:
        return { w * 3, h };
    default:
        break;
    }

    return { 0, 0 };
}

void Piece::UpdatePosition(int x, int y)
{
    this->x += x;
    this->y += y;
}

void Piece::SetPosition(int x, int y)
{
    this->x = x;
    this->y = y;
}

int Piece::GetBlockWidth(int scale) const
{
    return width * scale * 2;
}

int Piece::GetBlockHeight(int scale) const
{
    return height * scale;
}

int Piece::GetLeftXPosition() const
{
    return x;
}

int Piece::GetRightXPosition(int scale) const
{
    return x + GetSize(scale).Width;
}

int Piece::GetTopYPosition() const
{
    return y;
}

int Piece::GetBottomYPosition(int scale) const
{
    return y + GetSize(scale).Height;
}

void Piece::Rotate()
{
    std::array<std::array<bool, cells>, cells> tMatrix{ 0 };

    for (int i = 0; i < matrix.size(); i++)
    {
        for (int j = 0; j < matrix[i].size(); j++)
        {
            tMatrix[j][matrix.size() - i - 1] = matrix[i][j];
        }
    }

    matrix = tMatrix;
}
