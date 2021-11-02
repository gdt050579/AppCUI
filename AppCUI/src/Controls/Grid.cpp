#include "ControlContext.hpp"

using namespace AppCUI::Graphics;

Grid::Grid(std::string_view layout, unsigned int columnsNo, unsigned int rowsNo, GridFlags flags)
    : Control(new GridControlContext(), "", layout, false)
{
    auto context              = reinterpret_cast<GridControlContext*>(Context);
    context->Layout.MinHeight = 10;
    context->Layout.MinWidth  = 10;
    context->Flags            = GATTR_ENABLE | GATTR_VISIBLE;

    context->columnsNo = columnsNo;
    context->rowsNo    = rowsNo;
    context->flags     = flags;
}

void Grid::Paint(Renderer& renderer)
{
    auto context = reinterpret_cast<GridControlContext*>(Context);

    renderer.Clear(' ', context->Cfg->Grid.Background);

    if ((context->flags & GridFlags::HideHorizontalLines) == GridFlags::None ||
        (context->flags & GridFlags::HideVerticalLines) == GridFlags::None)
    {
        DrawLines(renderer);
    }

    if ((context->flags & GridFlags::HideBoxes) == GridFlags::None)
    {
        DrawBoxes(renderer);
    }
}

void Grid::DrawBoxes(Graphics::Renderer& renderer)
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);

    // define cell dimensions
    const auto cWidth  = static_cast<unsigned int>(context->Layout.Width / context->columnsNo);
    const auto cHeight = static_cast<unsigned int>(context->Layout.Height / context->rowsNo);

    // center matrix
    const auto startingPointX = static_cast<unsigned int>((context->Layout.Width - cWidth * context->columnsNo) / 2);
    const auto startingPointY = static_cast<unsigned int>((context->Layout.Height - cHeight * context->rowsNo) / 2);

    for (auto i = 0U; i <= context->columnsNo; i++)
    {
        const auto x = startingPointX + i * cWidth;
        for (auto j = 0U; j <= context->rowsNo; j++)
        {
            const auto y = startingPointY + j * cHeight;

            auto sc = SpecialChars::BoxCrossSingleLine;
            if (i == 0)
            {
                if (j == 0)
                {
                    sc = SpecialChars::BoxTopLeftCornerSingleLine;
                }
                else if (j == context->rowsNo)
                {
                    sc = SpecialChars::BoxBottomLeftCornerSingleLine;
                }
                else
                {
                    sc = SpecialChars::BoxMidleLeft;
                }
            }
            else if (i == context->columnsNo)
            {
                if (j == 0)
                {
                    sc = SpecialChars::BoxTopRightCornerSingleLine;
                }
                else if (j == context->rowsNo)
                {
                    sc = SpecialChars::BoxBottomRightCornerSingleLine;
                }
                else
                {
                    sc = SpecialChars::BoxMidleRight;
                }
            }
            else
            {
                if (j == 0)
                {
                    sc = SpecialChars::BoxMidleTop;
                }
                else if (j == context->rowsNo)
                {
                    sc = SpecialChars::BoxMidleBottom;
                }
            }

            renderer.WriteSpecialCharacter(x, y, sc, context->Cfg->Grid.Lines.Box);
        }
    }
}

void Grid::DrawLines(Graphics::Renderer& renderer)
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);

    // define cell dimensions
    const auto cWidth  = static_cast<unsigned int>(context->Layout.Width / context->columnsNo);
    const auto cHeight = static_cast<unsigned int>(context->Layout.Height / context->rowsNo);

    // center matrix
    const auto startingPointX = static_cast<unsigned int>((context->Layout.Width - cWidth * context->columnsNo) / 2);
    const auto startingPointY = static_cast<unsigned int>((context->Layout.Height - cHeight * context->rowsNo) / 2);

    for (auto i = 0U; i <= context->columnsNo; i++)
    {
        const auto x = startingPointX + i * cWidth;
        for (auto j = 0U; j <= context->rowsNo; j++)
        {
            const auto y = startingPointY + j * cHeight;

            if ((context->flags & GridFlags::HideHorizontalLines) == GridFlags::None)
            {
                if (i < context->columnsNo)
                {
                    const auto endX = startingPointX + (i + 1) * cWidth;
                    renderer.DrawHorizontalLine(x + 1, y, endX - 1, context->Cfg->Grid.Lines.Horizontal, true);
                }
            }

            if ((context->flags & GridFlags::HideVerticalLines) == GridFlags::None)
            {
                if (j < context->rowsNo)
                {
                    const auto endY = startingPointY + (j + 1) * cHeight;
                    renderer.DrawVerticalLine(x, y + 1, endY - 1, context->Cfg->Grid.Lines.Vertical, true);
                }
            }
        }
    }
}
