#include "ControlContext.hpp"

using namespace AppCUI::Input;
using namespace AppCUI::Graphics;

constexpr unsigned int InvalidCellIndex = 0xFFFFFFFF;

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

    // renderer.Clear(' ', context->Cfg->Grid.Background.Grid);

    if ((context->flags & GridFlags::DoNotDrawCellBackground) == GridFlags::None)
    {
        DrawCellsBackground(renderer);
    }

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

void Grid::OnMousePressed(int x, int y, MouseButton button)
{
    auto context = reinterpret_cast<GridControlContext*>(Context);

    switch (button)
    {
    case MouseButton::None:
        break;
    case MouseButton::Left:
    {
        context->hoveredCellIndex = InvalidCellIndex;
        context->selectedCellsIndexes.clear();
        const auto index = ComputeCellNumber(x, y);
        if (index != InvalidCellIndex)
        {
            context->selectedCellsIndexes.insert(index);
        }
    }
    break;
    case MouseButton::Center:
        break;
    case MouseButton::Right:
        break;
    case MouseButton::DoubleClicked:
        break;
    default:
        break;
    }
}

void Grid::OnMouseReleased(int x, int y, MouseButton button)
{
}

bool Grid::OnMouseDrag(int x, int y, MouseButton button)
{
    auto context = reinterpret_cast<GridControlContext*>(Context);

    switch (button)
    {
    case AppCUI::Input::MouseButton::None:
        break;
    case AppCUI::Input::MouseButton::Left:
    {
        context->hoveredCellIndex = InvalidCellIndex;
        const auto currentIndex   = ComputeCellNumber(x, y);
        if (currentIndex != InvalidCellIndex)
        {
            break;
        }

        if (context->selectedCellsIndexes.size() > 0)
        {
            auto initialIndex = *context->selectedCellsIndexes.begin();
            auto lastIndex    = *context->selectedCellsIndexes.rbegin();

            if (initialIndex != currentIndex)
            {
                const auto minColumnIndex = initialIndex % context->columnsNo;
                const auto minRowIndex    = initialIndex / context->columnsNo;

                const auto currentColumnIndex = currentIndex % context->columnsNo;
                const auto currentRowIndex    = currentIndex / context->columnsNo;

                const auto maxColumnIndex = lastIndex % context->columnsNo;
                const auto maxRowIndex    = lastIndex / context->columnsNo;

                const auto startColumnIndex = std::min<>(minColumnIndex, currentColumnIndex);
                const auto startRowIndex    = std::min<>(minRowIndex, currentRowIndex);

                const auto endColumnIndex = std::max<>(currentColumnIndex, maxColumnIndex);
                const auto endRowIndex    = std::max<>(currentRowIndex, maxRowIndex);

                for (auto i = startColumnIndex; i <= endColumnIndex; i++)
                {
                    for (auto j = startRowIndex; j <= endRowIndex; j++)
                    {
                        const auto current = context->columnsNo * j + i;
                        context->selectedCellsIndexes.insert(current);
                    }
                }
            }
        }
        else
        {
            context->selectedCellsIndexes.insert(currentIndex);
        }

        return true;
    }
    break;
    case AppCUI::Input::MouseButton::Center:
        break;
    case AppCUI::Input::MouseButton::Right:
        break;
    case AppCUI::Input::MouseButton::DoubleClicked:
        break;
    default:
        break;
    }

    return false;
}

bool Grid::OnMouseOver(int x, int y)
{
    auto context                        = reinterpret_cast<GridControlContext*>(Context);
    const auto previousHoveredCellIndex = context->hoveredCellIndex;
    context->hoveredCellIndex           = ComputeCellNumber(x, y);

    return context->hoveredCellIndex != previousHoveredCellIndex;
}

bool Grid::OnMouseLeave()
{
    auto context                        = reinterpret_cast<GridControlContext*>(Context);
    const auto previousHoveredCellIndex = context->hoveredCellIndex;
    context->hoveredCellIndex           = InvalidCellIndex;

    return context->hoveredCellIndex != previousHoveredCellIndex;
}

void Grid::OnLoseFocus()
{
    auto context = reinterpret_cast<GridControlContext*>(Context);
    context->selectedCellsIndexes.clear();
}

void Grid::DrawBoxes(Renderer& renderer)
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);

    // define cell dimensions
    const auto cWidth  = static_cast<unsigned int>(context->Layout.Width / context->columnsNo);
    const auto cHeight = static_cast<unsigned int>(context->Layout.Height / context->rowsNo);

    // center matrix
    const auto offsetX = static_cast<unsigned int>((context->Layout.Width - cWidth * context->columnsNo) / 2);
    const auto offsetY = static_cast<unsigned int>((context->Layout.Height - cHeight * context->rowsNo) / 2);

    for (auto i = 0U; i <= context->columnsNo; i++)
    {
        const auto x = offsetX + i * cWidth;
        for (auto j = 0U; j <= context->rowsNo; j++)
        {
            const auto y  = offsetY + j * cHeight;
            const auto sc = ComputeBoxType(i, j);
            renderer.WriteSpecialCharacter(x, y, sc, context->Cfg->Grid.Lines.Box.Normal);
        }
    }

    if (context->hoveredCellIndex != InvalidCellIndex &&
        ((context->flags & GridFlags::HideHoveredCell) == GridFlags::None))
    {
        const auto columnIndex = context->hoveredCellIndex % context->columnsNo;
        const auto rowIndex    = context->hoveredCellIndex / context->columnsNo;

        const auto xLeft  = offsetX + columnIndex * cWidth;
        const auto xRight = offsetX + (columnIndex + 1) * cWidth;

        const auto yTop    = offsetY + rowIndex * cHeight;
        const auto yBottom = offsetY + (rowIndex + 1) * cHeight;

        renderer.WriteSpecialCharacter(
              xLeft, yTop, SpecialChars::BoxTopLeftCornerSingleLine, context->Cfg->Grid.Lines.Box.Hovered);

        renderer.WriteSpecialCharacter(
              xRight, yTop, SpecialChars::BoxTopRightCornerSingleLine, context->Cfg->Grid.Lines.Box.Hovered);

        renderer.WriteSpecialCharacter(
              xLeft, yBottom, SpecialChars::BoxBottomLeftCornerSingleLine, context->Cfg->Grid.Lines.Box.Hovered);

        renderer.WriteSpecialCharacter(
              xRight, yBottom, SpecialChars::BoxBottomRightCornerSingleLine, context->Cfg->Grid.Lines.Box.Hovered);
    }

    if (context->selectedCellsIndexes.size() > 0 && ((context->flags & GridFlags::HideSelectedCell) == GridFlags::None))
    {
        for (const auto& cellIndex : context->selectedCellsIndexes)
        {
            const auto columnIndex = cellIndex % context->columnsNo;
            const auto rowIndex    = cellIndex / context->columnsNo;

            const auto xLeft  = offsetX + columnIndex * cWidth;
            const auto xRight = offsetX + (columnIndex + 1) * cWidth;

            const auto yTop    = offsetY + rowIndex * cHeight;
            const auto yBottom = offsetY + (rowIndex + 1) * cHeight;

            renderer.WriteSpecialCharacter(
                  xLeft, yTop, SpecialChars::BoxTopLeftCornerSingleLine, context->Cfg->Grid.Lines.Box.Selected);

            renderer.WriteSpecialCharacter(
                  xRight, yTop, SpecialChars::BoxTopRightCornerSingleLine, context->Cfg->Grid.Lines.Box.Selected);

            renderer.WriteSpecialCharacter(
                  xLeft, yBottom, SpecialChars::BoxBottomLeftCornerSingleLine, context->Cfg->Grid.Lines.Box.Selected);

            renderer.WriteSpecialCharacter(
                  xRight, yBottom, SpecialChars::BoxBottomRightCornerSingleLine, context->Cfg->Grid.Lines.Box.Selected);
        }
    }
}

void Grid::DrawLines(Renderer& renderer)
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);

    // define cell dimensions
    const auto cWidth  = static_cast<unsigned int>(context->Layout.Width / context->columnsNo);
    const auto cHeight = static_cast<unsigned int>(context->Layout.Height / context->rowsNo);

    // center matrix
    const auto offsetX = static_cast<unsigned int>((context->Layout.Width - cWidth * context->columnsNo) / 2);
    const auto offsetY = static_cast<unsigned int>((context->Layout.Height - cHeight * context->rowsNo) / 2);

    for (auto i = 0U; i <= context->columnsNo; i++)
    {
        const auto x = offsetX + i * cWidth;
        for (auto j = 0U; j <= context->rowsNo; j++)
        {
            const auto y = offsetY + j * cHeight;

            if ((context->flags & GridFlags::HideHorizontalLines) == GridFlags::None)
            {
                if (i < context->columnsNo)
                {
                    const auto endX = offsetX + (i + 1) * cWidth;
                    renderer.DrawHorizontalLine(x + 1, y, endX - 1, context->Cfg->Grid.Lines.Horizontal.Normal, true);
                }
            }

            if ((context->flags & GridFlags::HideVerticalLines) == GridFlags::None)
            {
                if (j < context->rowsNo)
                {
                    const auto endY = offsetY + (j + 1) * cHeight;
                    renderer.DrawVerticalLine(x, y + 1, endY - 1, context->Cfg->Grid.Lines.Vertical.Normal, true);
                }
            }
        }
    }

    if (context->hoveredCellIndex != InvalidCellIndex &&
        ((context->flags & GridFlags::HideHoveredCell) == GridFlags::None))
    {
        const auto columnIndex = context->hoveredCellIndex % context->columnsNo;
        const auto rowIndex    = context->hoveredCellIndex / context->columnsNo;

        const auto xLeft  = offsetX + columnIndex * cWidth;
        const auto xRight = offsetX + (columnIndex + 1) * cWidth;

        const auto yTop    = offsetY + rowIndex * cHeight;
        const auto yBottom = offsetY + (rowIndex + 1) * cHeight;

        renderer.DrawVerticalLine(xLeft, yTop + 1, yBottom - 1, context->Cfg->Grid.Lines.Vertical.Hovered, true);
        renderer.DrawVerticalLine(xRight, yTop + 1, yBottom - 1, context->Cfg->Grid.Lines.Vertical.Hovered, true);

        renderer.DrawHorizontalLine(xLeft + 1, yTop, xRight - 1, context->Cfg->Grid.Lines.Horizontal.Hovered, true);
        renderer.DrawHorizontalLine(xLeft + 1, yBottom, xRight - 1, context->Cfg->Grid.Lines.Horizontal.Hovered, true);
    }

    if (context->selectedCellsIndexes.size() > 0 && ((context->flags & GridFlags::HideSelectedCell) == GridFlags::None))
    {
        for (const auto& cellIndex : context->selectedCellsIndexes)
        {
            const auto columnIndex = cellIndex % context->columnsNo;
            const auto rowIndex    = cellIndex / context->columnsNo;

            const auto xLeft  = offsetX + columnIndex * cWidth;
            const auto xRight = offsetX + (columnIndex + 1) * cWidth;

            const auto yTop    = offsetY + rowIndex * cHeight;
            const auto yBottom = offsetY + (rowIndex + 1) * cHeight;

            renderer.DrawVerticalLine(xLeft, yTop + 1, yBottom - 1, context->Cfg->Grid.Lines.Vertical.Selected, true);
            renderer.DrawVerticalLine(xRight, yTop + 1, yBottom - 1, context->Cfg->Grid.Lines.Vertical.Selected, true);

            renderer.DrawHorizontalLine(
                  xLeft + 1, yTop, xRight - 1, context->Cfg->Grid.Lines.Horizontal.Selected, true);
            renderer.DrawHorizontalLine(
                  xLeft + 1, yBottom, xRight - 1, context->Cfg->Grid.Lines.Horizontal.Selected, true);
        }
    }
}

unsigned int Grid::ComputeCellNumber(int x, int y)
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);

    // define cell dimensions
    const auto cWidth  = static_cast<unsigned int>(context->Layout.Width / context->columnsNo);
    const auto cHeight = static_cast<unsigned int>(context->Layout.Height / context->rowsNo);

    // center matrix
    const auto offsetX = static_cast<unsigned int>((context->Layout.Width - cWidth * context->columnsNo) / 2);
    const auto offsetY = static_cast<unsigned int>((context->Layout.Height - cHeight * context->rowsNo) / 2);

    const auto endX = static_cast<unsigned int>(context->Layout.Width - offsetX);
    const auto endY = static_cast<unsigned int>(context->Layout.Height - offsetY);

    if (static_cast<unsigned int>(x) <= offsetX || static_cast<unsigned int>(x) >= endX)
    {
        return InvalidCellIndex;
    }

    if (static_cast<unsigned int>(y) <= offsetY || static_cast<unsigned int>(y) >= endY)
    {
        return InvalidCellIndex;
    }

    const auto columnIndex = (static_cast<unsigned int>(x) - offsetX) / cWidth;
    const auto rowIndex    = (static_cast<unsigned int>(y) - offsetY) / cHeight;
    const auto cellIndex   = context->columnsNo * rowIndex + columnIndex;

    return cellIndex;
}

SpecialChars Grid::ComputeBoxType(unsigned int i, unsigned int j)
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);

    if (i == 0)
    {
        if (j == 0)
        {
            return SpecialChars::BoxTopLeftCornerSingleLine;
        }
        else if (j == context->rowsNo)
        {
            return SpecialChars::BoxBottomLeftCornerSingleLine;
        }
        else
        {
            return SpecialChars::BoxMidleLeft;
        }
    }
    else if (i == context->columnsNo)
    {
        if (j == 0)
        {
            return SpecialChars::BoxTopRightCornerSingleLine;
        }
        else if (j == context->rowsNo)
        {
            return SpecialChars::BoxBottomRightCornerSingleLine;
        }
        else
        {
            return SpecialChars::BoxMidleRight;
        }
    }
    else
    {
        if (j == 0)
        {
            return SpecialChars::BoxMidleTop;
        }
        else if (j == context->rowsNo)
        {
            return SpecialChars::BoxMidleBottom;
        }
    }

    return SpecialChars::BoxCrossSingleLine;
}

void Grid::DrawCellsBackground(Graphics::Renderer& renderer)
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);

    // define cell dimensions
    const auto cWidth  = static_cast<unsigned int>(context->Layout.Width / context->columnsNo);
    const auto cHeight = static_cast<unsigned int>(context->Layout.Height / context->rowsNo);

    // center matrix
    const auto offsetX = static_cast<unsigned int>((context->Layout.Width - cWidth * context->columnsNo) / 2);
    const auto offsetY = static_cast<unsigned int>((context->Layout.Height - cHeight * context->rowsNo) / 2);

    for (auto i = 0U; i < context->columnsNo; i++)
    {
        for (auto j = 0U; j < context->rowsNo; j++)
        {
            const auto xLeft  = offsetX + i * cWidth;
            const auto xRight = offsetX + (i + 1) * cWidth;

            const auto yTop    = offsetY + j * cHeight;
            const auto yBottom = offsetY + (j + 1) * cHeight;

            renderer.FillRect(
                  xLeft + 1, yTop + 1, xRight - 1, yBottom - 1, ' ', context->Cfg->Grid.Background.Cell.Normal);
        }
    }

    if (context->hoveredCellIndex != InvalidCellIndex &&
        ((context->flags & GridFlags::HideHoveredCell) == GridFlags::None))
    {
        const auto columnIndex = context->hoveredCellIndex % context->columnsNo;
        const auto rowIndex    = context->hoveredCellIndex / context->columnsNo;

        const auto xLeft  = offsetX + columnIndex * cWidth;
        const auto xRight = offsetX + (columnIndex + 1) * cWidth;

        const auto yTop    = offsetY + rowIndex * cHeight;
        const auto yBottom = offsetY + (rowIndex + 1) * cHeight;

        renderer.FillRect(
              xLeft + 1, yTop + 1, xRight - 1, yBottom - 1, ' ', context->Cfg->Grid.Background.Cell.Hovered);
    }

    if (context->selectedCellsIndexes.size() > 0 && ((context->flags & GridFlags::HideSelectedCell) == GridFlags::None))
    {
        for (const auto& cellIndex : context->selectedCellsIndexes)
        {
            const auto columnIndex = cellIndex % context->columnsNo;
            const auto rowIndex    = cellIndex / context->columnsNo;

            const auto xLeft  = offsetX + columnIndex * cWidth;
            const auto xRight = offsetX + (columnIndex + 1) * cWidth;

            const auto yTop    = offsetY + rowIndex * cHeight;
            const auto yBottom = offsetY + (rowIndex + 1) * cHeight;

            renderer.FillRect(
                  xLeft + 1, yTop + 1, xRight - 1, yBottom - 1, ' ', context->Cfg->Grid.Background.Cell.Selected);
        }
    }
}
