#include "ControlContext.hpp"

using namespace AppCUI::Input;
using namespace AppCUI::Graphics;

constexpr unsigned int InvalidCellIndex = 0xFFFFFFFF;

constexpr unsigned int MenuCommandMergeCells = 0x01;

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

    UpdateCellData();

    context->rightClickMenu.AddCommandItem("&Merge Cells", MenuCommandMergeCells, Key::F2);
}

void Grid::Paint(Renderer& renderer)
{
    UpdateCellData();

    auto context = reinterpret_cast<GridControlContext*>(Context);

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
            context->anchorCellIndex = index;
            context->selectedCellsIndexes.emplace_back(index);
        }
    }
    break;
    case MouseButton::Center:
        break;
    case MouseButton::Right:
        if (context->selectedCellsIndexes.size() > 1)
        {
            context->rightClickMenu.Show(x, y);
        }
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
        if (currentIndex == InvalidCellIndex)
        {
            break;
        }

        context->selectedCellsIndexes.clear();
        context->selectedCellsIndexes.emplace_back(currentIndex);

        if (context->anchorCellIndex != currentIndex)
        {
            const auto anchorColumnIndex = context->anchorCellIndex % context->columnsNo;
            const auto anchorRowIndex    = context->anchorCellIndex / context->columnsNo;

            const auto currentColumnIndex = currentIndex % context->columnsNo;
            const auto currentRowIndex    = currentIndex / context->columnsNo;

            const auto startColumnIndex = std::min<>(anchorColumnIndex, currentColumnIndex);
            const auto startRowIndex    = std::min<>(anchorRowIndex, currentRowIndex);

            const auto endColumnIndex = std::max<>(anchorColumnIndex, currentColumnIndex);
            const auto endRowIndex    = std::max<>(anchorRowIndex, currentRowIndex);

            for (auto i = startColumnIndex; i <= endColumnIndex; i++)
            {
                for (auto j = startRowIndex; j <= endRowIndex; j++)
                {
                    const auto current = context->columnsNo * j + i;
                    context->selectedCellsIndexes.emplace_back(current);
                }
            }
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

bool AppCUI::Controls::Grid::OnEvent(Controls::Reference<Control>, Event eventType, int controlID)
{
    if (eventType == Event::Command)
    {
        switch (controlID)
        {
        case MenuCommandMergeCells:
            // TODO:
            break;
        default:
            break;
        }
    }

    return false;
}

void Grid::DrawBoxes(Renderer& renderer)
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);

    for (auto i = 0U; i <= context->columnsNo; i++)
    {
        const auto x = context->offsetX + i * context->cWidth;
        for (auto j = 0U; j <= context->rowsNo; j++)
        {
            const auto y  = context->offsetY + j * context->cHeight;
            const auto sc = ComputeBoxType(i, j, 0U, 0U, context->columnsNo, context->rowsNo);
            renderer.WriteSpecialCharacter(x, y, sc, context->Cfg->Grid.Lines.Box.Normal);
        }
    }

    if (context->hoveredCellIndex != InvalidCellIndex &&
        ((context->flags & GridFlags::HideHoveredCell) == GridFlags::None))
    {
        const auto columnIndex = context->hoveredCellIndex % context->columnsNo;
        const auto rowIndex    = context->hoveredCellIndex / context->columnsNo;

        const auto xLeft  = context->offsetX + columnIndex * context->cWidth;
        const auto xRight = context->offsetX + (columnIndex + 1) * context->cWidth;

        const auto yTop    = context->offsetY + rowIndex * context->cHeight;
        const auto yBottom = context->offsetY + (rowIndex + 1) * context->cHeight;

        const auto& color = context->Cfg->Grid.Lines.Box.Hovered;

        renderer.WriteSpecialCharacter(xLeft, yTop, SpecialChars::BoxTopLeftCornerSingleLine, color);
        renderer.WriteSpecialCharacter(xRight, yTop, SpecialChars::BoxTopRightCornerSingleLine, color);
        renderer.WriteSpecialCharacter(xLeft, yBottom, SpecialChars::BoxBottomLeftCornerSingleLine, color);
        renderer.WriteSpecialCharacter(xRight, yBottom, SpecialChars::BoxBottomRightCornerSingleLine, color);
    }

    if (context->selectedCellsIndexes.size() > 0 && ((context->flags & GridFlags::HideSelectedCell) == GridFlags::None))
    {
        // we assume these are already sorted
        const auto startCellIndex = context->selectedCellsIndexes[0];
        const auto endCellIndex   = context->selectedCellsIndexes[context->selectedCellsIndexes.size() - 1];

        const auto sci = startCellIndex % context->columnsNo;
        const auto sri = startCellIndex / context->columnsNo;

        const auto eci = endCellIndex % context->columnsNo + 1U;
        const auto eri = endCellIndex / context->columnsNo + 1U;

        for (auto i = sci; i <= eci; i++)
        {
            const auto x = context->offsetX + i * context->cWidth;
            for (auto j = sri; j <= eri; j++)
            {
                const auto y  = context->offsetY + j * context->cHeight;
                const auto sc = ComputeBoxType(i, j, sci, sri, eci, eri);
                renderer.WriteSpecialCharacter(x, y, sc, context->Cfg->Grid.Lines.Box.Selected);
            }
        }
    }
}

void Grid::DrawLines(Renderer& renderer)
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);

    for (auto i = 0U; i <= context->columnsNo; i++)
    {
        const auto x = context->offsetX + i * context->cWidth;
        for (auto j = 0U; j <= context->rowsNo; j++)
        {
            const auto y = context->offsetY + j * context->cHeight;

            if ((context->flags & GridFlags::HideHorizontalLines) == GridFlags::None)
            {
                if (i < context->columnsNo)
                {
                    const auto endX = context->offsetX + (i + 1) * context->cWidth;
                    renderer.DrawHorizontalLine(x + 1, y, endX - 1, context->Cfg->Grid.Lines.Horizontal.Normal, true);
                }
            }

            if ((context->flags & GridFlags::HideVerticalLines) == GridFlags::None)
            {
                if (j < context->rowsNo)
                {
                    const auto endY = context->offsetY + (j + 1) * context->cHeight;
                    renderer.DrawVerticalLine(x, y + 1, endY - 1, context->Cfg->Grid.Lines.Vertical.Normal, true);
                }
            }
        }
    }

    const auto drawLines = [&](unsigned int cellIndex, CellType cellType)
    {
        ColorPair vertical   = context->Cfg->Grid.Lines.Vertical.Normal;
        ColorPair horizontal = context->Cfg->Grid.Lines.Horizontal.Normal;

        switch (cellType)
        {
        case AppCUI::Controls::Grid::CellType::Normal:
            vertical   = context->Cfg->Grid.Lines.Vertical.Normal;
            horizontal = context->Cfg->Grid.Lines.Horizontal.Normal;
            break;
        case AppCUI::Controls::Grid::CellType::Selected:
            vertical   = context->Cfg->Grid.Lines.Vertical.Selected;
            horizontal = context->Cfg->Grid.Lines.Horizontal.Selected;
            break;
        case AppCUI::Controls::Grid::CellType::Hovered:
            vertical   = context->Cfg->Grid.Lines.Vertical.Hovered;
            horizontal = context->Cfg->Grid.Lines.Horizontal.Hovered;
            break;
        default:
            vertical   = context->Cfg->Grid.Lines.Vertical.Normal;
            horizontal = context->Cfg->Grid.Lines.Horizontal.Normal;
            break;
        }

        const auto columnIndex = cellIndex % context->columnsNo;
        const auto rowIndex    = cellIndex / context->columnsNo;

        const auto xLeft  = context->offsetX + columnIndex * context->cWidth;
        const auto xRight = context->offsetX + (columnIndex + 1) * context->cWidth;

        const auto yTop    = context->offsetY + rowIndex * context->cHeight;
        const auto yBottom = context->offsetY + (rowIndex + 1) * context->cHeight;

        renderer.DrawVerticalLine(xLeft, yTop + 1, yBottom - 1, vertical, true);
        renderer.DrawVerticalLine(xRight, yTop + 1, yBottom - 1, vertical, true);

        renderer.DrawHorizontalLine(xLeft + 1, yTop, xRight - 1, horizontal, true);
        renderer.DrawHorizontalLine(xLeft + 1, yBottom, xRight - 1, horizontal, true);
    };

    if (context->hoveredCellIndex != InvalidCellIndex &&
        ((context->flags & GridFlags::HideHoveredCell) == GridFlags::None))
    {
        drawLines(context->hoveredCellIndex, CellType::Hovered);
    }

    if (context->selectedCellsIndexes.size() > 0 && ((context->flags & GridFlags::HideSelectedCell) == GridFlags::None))
    {
        for (const auto& cellIndex : context->selectedCellsIndexes)
        {
            drawLines(cellIndex, CellType::Selected);
        }
    }
}

unsigned int Grid::ComputeCellNumber(int x, int y)
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);

    const auto endX = static_cast<unsigned int>(context->Layout.Width - context->offsetX);
    const auto endY = static_cast<unsigned int>(context->Layout.Height - context->offsetY);

    if (static_cast<unsigned int>(x) <= context->offsetX || static_cast<unsigned int>(x) >= endX)
    {
        return InvalidCellIndex;
    }

    if (static_cast<unsigned int>(y) <= context->offsetY || static_cast<unsigned int>(y) >= endY)
    {
        return InvalidCellIndex;
    }

    const auto columnIndex = (static_cast<unsigned int>(x) - context->offsetX) / context->cWidth;
    const auto rowIndex    = (static_cast<unsigned int>(y) - context->offsetY) / context->cHeight;
    const auto cellIndex   = context->columnsNo * rowIndex + columnIndex;

    return cellIndex;
}

SpecialChars Grid::ComputeBoxType(
      unsigned int colIndex,
      unsigned int rowIndex,
      unsigned int startColumnsIndex,
      unsigned int startRowsIndex,
      unsigned int endColumnsIndex,
      unsigned int endRowsIndex)
{
    if (colIndex == startColumnsIndex)
    {
        if (rowIndex == startRowsIndex)
        {
            return SpecialChars::BoxTopLeftCornerSingleLine;
        }
        else if (rowIndex == endRowsIndex)
        {
            return SpecialChars::BoxBottomLeftCornerSingleLine;
        }
        else
        {
            return SpecialChars::BoxMidleLeft;
        }
    }
    else if (colIndex == endColumnsIndex)
    {
        if (rowIndex == startRowsIndex)
        {
            return SpecialChars::BoxTopRightCornerSingleLine;
        }
        else if (rowIndex == endRowsIndex)
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
        if (rowIndex == startRowsIndex)
        {
            return SpecialChars::BoxMidleTop;
        }
        else if (rowIndex == endRowsIndex)
        {
            return SpecialChars::BoxMidleBottom;
        }
    }

    return SpecialChars::BoxCrossSingleLine;
}

void Grid::DrawCellsBackground(Graphics::Renderer& renderer)
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);

    for (auto i = 0U; i < context->columnsNo; i++)
    {
        for (auto j = 0U; j < context->rowsNo; j++)
        {
            DrawCellBackground(renderer, CellType::Normal, i, j);
        }
    }

    if (context->hoveredCellIndex != InvalidCellIndex &&
        ((context->flags & GridFlags::HideHoveredCell) == GridFlags::None))
    {
        DrawCellBackground(renderer, CellType::Hovered, context->hoveredCellIndex);
    }

    if (context->selectedCellsIndexes.size() > 0 && ((context->flags & GridFlags::HideSelectedCell) == GridFlags::None))
    {
        for (const auto& cellIndex : context->selectedCellsIndexes)
        {
            DrawCellBackground(renderer, CellType::Selected, cellIndex);
        }
    }
}

void AppCUI::Controls::Grid::DrawCellBackground(
      Graphics::Renderer& renderer, CellType cellType, unsigned int i, unsigned int j)
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);

    const auto xLeft  = context->offsetX + i * context->cWidth;
    const auto xRight = context->offsetX + (i + 1) * context->cWidth;

    const auto yTop    = context->offsetY + j * context->cHeight;
    const auto yBottom = context->offsetY + (j + 1) * context->cHeight;

    ColorPair color = context->Cfg->Grid.Background.Cell.Normal;
    switch (cellType)
    {
    case AppCUI::Controls::Grid::CellType::Normal:
        color = context->Cfg->Grid.Background.Cell.Normal;
        break;
    case AppCUI::Controls::Grid::CellType::Selected:
        color = context->Cfg->Grid.Background.Cell.Selected;
        break;
    case AppCUI::Controls::Grid::CellType::Hovered:
        color = context->Cfg->Grid.Background.Cell.Hovered;
        break;
    default:
        color = context->Cfg->Grid.Background.Cell.Normal;
        break;
    }

    renderer.FillRect(xLeft + 1, yTop + 1, xRight - 1, yBottom - 1, ' ', color);
}

void AppCUI::Controls::Grid::DrawCellBackground(Graphics::Renderer& renderer, CellType cellType, unsigned int cellIndex)
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);

    const auto columnIndex = cellIndex % context->columnsNo;
    const auto rowIndex    = cellIndex / context->columnsNo;

    DrawCellBackground(renderer, cellType, columnIndex, rowIndex);
}

void AppCUI::Controls::Grid::UpdateCellData()
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);

    // define cell dimensions
    context->cWidth  = static_cast<unsigned int>(context->Layout.Width / context->columnsNo);
    context->cHeight = static_cast<unsigned int>(context->Layout.Height / context->rowsNo);

    // center matrix
    context->offsetX = static_cast<unsigned int>((context->Layout.Width - context->cWidth * context->columnsNo) / 2);
    context->offsetY = static_cast<unsigned int>((context->Layout.Height - context->cHeight * context->rowsNo) / 2);

    // sort selected cells for better drawing
    struct
    {
        const GridControlContext* gcc;
        bool operator()(unsigned int a, unsigned int b)
        {
            const auto aci = a % gcc->columnsNo;
            const auto ari = a / gcc->columnsNo;

            const auto bci = b % gcc->columnsNo;
            const auto bri = b / gcc->columnsNo;

            if (ari < bri)
            {
                return true;
            }
            if (ari == bri)
            {
                if (aci < bci)
                {
                    return true;
                }
            }

            return false;
        };
    } sortingComparator{ context };
    std::sort(context->selectedCellsIndexes.begin(), context->selectedCellsIndexes.end(), sortingComparator);
}
