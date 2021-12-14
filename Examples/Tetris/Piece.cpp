#include "Piece.hpp"

Piece::Piece(const PieceType type, const AppCUI::Utils::Reference<AppCUI::Controls::Control> control)
    : control(control), type(type)
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

        const auto size = GetSize(type, scale);
        const int x     = std::max<>(1, static_cast<int>((w - size.Width) / 2));
        const int y     = std::max<>(1, static_cast<int>(static_cast<int>((h - 2 - size.Height)) / 2));

        return Draw(renderer, scale, x, y);
    }
    return Draw(renderer, scale, 1, 1);
}

bool Piece::Draw(AppCUI::Graphics::Renderer& renderer, int scale, const int x, const int y)
{
    int xx = x;
    int yy = y;

    for (auto i = 0U; i < cells; i++)
    {
        for (auto j = 0U; j < cells; j++)
        {
            if (matrix[i][j] == 1)
            {
                renderer.DrawRectSize(xx, yy, width * scale * 2, height * scale, color, false);
            }

            xx += width * scale * 2;
        }

        xx = x;
        yy += height * scale;
    }

    return true;
}

AppCUI::Graphics::Size Piece::GetSize(PieceType type, int scale) const
{
    const auto w = width * scale * 2;
    const auto h = height * scale;

    switch (type)
    {
    case PieceType::I:
        return { w, h * 4 };
    case PieceType::O:
        return { w * 2, h * 2 };
    case PieceType::T:
        return { w * 2, h * 3 };
    case PieceType::S:
        return { w * 3, h * 2 };
    case PieceType::L:
        return { w * 3, h * 2 };
    case PieceType::Z:
        return { w * 3, h * 2 };
    case PieceType::J:
        return { w * 3, h * 2 };
    default:
        break;
    }

    return { 0, 0 };
}
