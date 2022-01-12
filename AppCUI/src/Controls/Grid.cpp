#include "ControlContext.hpp"

namespace AppCUI
{
using namespace Input;
using namespace Graphics;

constexpr auto InvalidCellIndex = 0xFFFFFFFFU;

constexpr auto minCellWidth  = 0x03U;
constexpr auto minCellHeight = 0x02U;

Grid::Grid(string_view layout, uint32 columnsNo, uint32 rowsNo, GridFlags flags)
    : Control(new GridControlContext(), "", layout, false)
{
    auto context              = reinterpret_cast<GridControlContext*>(Context);
    context->Layout.MinHeight = 20;
    context->Layout.MinWidth  = 20;
    context->Flags            = GATTR_ENABLE | GATTR_VISIBLE;

    context->columnsNo = columnsNo;
    context->rowsNo    = rowsNo;
    context->flags     = flags;

    context->columnsSort.insert(context->columnsSort.end(), columnsNo, true);

    context->ResetMatrixPosition();
    context->UpdateGridParameters();

    context->headers.reserve(context->columnsNo);
    context->SetDefaultHeaderValues();

    context->ReserveMap();
    Sort();
}

void Grid::Paint(Renderer& renderer)
{
    auto context = reinterpret_cast<GridControlContext*>(Context);
    context->UpdateGridParameters(true);

    renderer.Clear(' ', context->Cfg->Grid.Background.Grid);

    context->DrawHeader(renderer);

    if ((context->flags & GridFlags::TransparentBackground) == GridFlags::None)
    {
        context->DrawCellsBackground(renderer);
    }

    if ((context->flags & GridFlags::HideHorizontalLines) == GridFlags::None ||
        (context->flags & GridFlags::HideVerticalLines) == GridFlags::None)
    {
        context->DrawLines(renderer);
    }

    if ((context->flags & GridFlags::HideBoxes) == GridFlags::None)
    {
        context->DrawBoxes(renderer);
    }

    for (auto const& [key, val] : context->cells)
    {
        context->DrawCellContent(renderer, key);
    }
}

bool Grid::OnKeyEvent(Input::Key keyCode, char16_t /*UnicodeChar*/)
{
    auto context = reinterpret_cast<GridControlContext*>(Context);

    switch (keyCode)
    {
    case Input::Key::Left:
    case Input::Key::Right:
    case Input::Key::Up:
    case Input::Key::Down:
        if (context->MoveSelectedCellByKeys(keyCode))
        {
            return true;
        }
        break;
    case Input::Key::Shift | Input::Key::Left:
    case Input::Key::Shift | Input::Key::Right:
    case Input::Key::Shift | Input::Key::Up:
    case Input::Key::Shift | Input::Key::Down:
        if (context->SelectCellsByKeys(keyCode))
        {
            return true;
        }
        break;
    case Input::Key::Escape:
        if (context->selectedCellsIndexes.size() > 0)
        {
            context->selectedCellsIndexes.clear();
            context->duplicatedCellsIndexes.clear();
            return true;
        }
        break;
    case Input::Key::Ctrl | Input::Key::C:
        if (context->CopySelectedCellsContent())
        {
            return true;
        }
        break;
    case Input::Key::Ctrl | Input::Key::V:
        if (context->PasteContentToSelectedCells())
        {
            return true;
        }
        break;

    case Input::Key::Ctrl | Input::Key::Alt | Input::Key::Up:
        if ((context->flags & GridFlags::DisableZoom) == GridFlags::None)
        {
            context->startedMoving = true;
            context->UpdateDimensions(0, 1);
            return true;
        }
        break;
    case Input::Key::Ctrl | Input::Key::Alt | Input::Key::Down:
        if ((context->flags & GridFlags::DisableZoom) == GridFlags::None)
        {
            context->startedMoving = true;
            context->UpdateDimensions(0, -1);
            return true;
        }
        break;
    case Input::Key::Ctrl | Input::Key::Alt | Input::Key::Left:
        if ((context->flags & GridFlags::DisableZoom) == GridFlags::None)
        {
            context->startedMoving = true;
            context->UpdateDimensions(-1, 0);
            return true;
        }
        break;
    case Input::Key::Ctrl | Input::Key::Alt | Input::Key::Right:
        if ((context->flags & GridFlags::DisableZoom) == GridFlags::None)
        {
            context->startedMoving = true;
            context->UpdateDimensions(1, 0);
            return true;
        }
        break;

    case Input::Key::Ctrl | Input::Key::Up:
        if ((context->flags & GridFlags::DisableMove) == GridFlags::None)
        {
            context->UpdatePositions(0, -1);
            context->startedMoving = true;
            return true;
        }
        break;
    case Input::Key::Ctrl | Input::Key::Down:
        if ((context->flags & GridFlags::DisableMove) == GridFlags::None)
        {
            context->UpdatePositions(0, 1);
            context->startedMoving = true;
            return true;
        }
        break;
    case Input::Key::Ctrl | Input::Key::Left:
        if ((context->flags & GridFlags::DisableMove) == GridFlags::None)
        {
            context->UpdatePositions(-1, 0);
            context->startedMoving = true;
            return true;
        }
        break;
    case Input::Key::Ctrl | Input::Key::Right:
        if ((context->flags & GridFlags::DisableMove) == GridFlags::None)
        {
            context->UpdatePositions(1, 0);
            context->startedMoving = true;
            return true;
        }
        break;

    case Input::Key::Ctrl | Input::Key::Space:
        context->startedMoving = false;
        context->UpdateGridParameters();
        return true;

    default:
        break;
    }

    return false;
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
        const auto index = context->ComputeCellNumber(x, y);
        if (index != InvalidCellIndex)
        {
            context->anchorCellIndex = index;
            context->selectedCellsIndexes.emplace_back(index);
        }

        if ((context->flags & GridFlags::Sort) != GridFlags::None)
        {
            context->ToggleSorting(x, y);
        }

        if ((context->flags & GridFlags::DisableDuplicates) == GridFlags::None)
        {
            context->FindDuplicates();
        }
    }
    break;
    case MouseButton::Center:
        break;
    case MouseButton::Right:
        context->lastLocationDraggedRightClicked = { x, y };
        break;
    case MouseButton::Left | MouseButton::DoubleClicked:
        break;
    case MouseButton::Center | MouseButton::DoubleClicked:
        break;
    case MouseButton::Right | MouseButton::DoubleClicked:
        break;
    default:
        break;
    }
}

void Grid::OnMouseReleased(int /*x*/, int /*y*/, MouseButton button)
{
    auto context = reinterpret_cast<GridControlContext*>(Context);
    switch (button)
    {
    case AppCUI::Input::MouseButton::None:
        break;
    case AppCUI::Input::MouseButton::Left:
        break;
    case AppCUI::Input::MouseButton::Center:
        break;
    case AppCUI::Input::MouseButton::Right:
        context->lastLocationDraggedRightClicked = { 0, 0 };
        break;
    case AppCUI::Input::MouseButton::DoubleClicked:
        break;
    default:
        break;
    }
}

bool Grid::OnMouseDrag(int x, int y, MouseButton button)
{
    auto context = reinterpret_cast<GridControlContext*>(Context);

    switch (button)
    {
    case Input::MouseButton::None:
        break;
    case Input::MouseButton::Left:
    {
        context->hoveredCellIndex = InvalidCellIndex;
        const auto currentIndex   = context->ComputeCellNumber(x, y);
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

        if ((context->flags & GridFlags::DisableDuplicates) == GridFlags::None)
        {
            context->FindDuplicates();
        }
        return true;
    }
    break;
    case Input::MouseButton::Center:
        break;
    case Input::MouseButton::Right:
        if ((context->flags & GridFlags::DisableMove) == GridFlags::None)
        {
            const bool left   = x < context->lastLocationDraggedRightClicked.X;
            const bool right  = x > context->lastLocationDraggedRightClicked.X;
            const bool top    = y < context->lastLocationDraggedRightClicked.Y;
            const bool bottom = y > context->lastLocationDraggedRightClicked.Y;
            const bool noH    = x == context->lastLocationDraggedRightClicked.X;
            const bool noV    = y == context->lastLocationDraggedRightClicked.Y;

            if (left == false && right == false && top == false && bottom == false)
            {
                break;
            }

            int xDelta = 0;
            if (noH == false)
            {
                xDelta = left - right;
            }

            int yDelta = 0;
            if (noV == false)
            {
                yDelta = top - bottom;
            }

            context->UpdatePositions(xDelta, yDelta);
            context->startedMoving                   = true;
            context->lastLocationDraggedRightClicked = { x, y };
            return true;
        }
        break;
    case Input::MouseButton::DoubleClicked:
        break;
    default:
        break;
    }

    return false;
}

bool Controls::Grid::OnMouseWheel(int x, int y, Input::MouseWheel direction)
{
    auto context = reinterpret_cast<GridControlContext*>(Context);

    switch (direction)
    {
    case AppCUI::Input::MouseWheel::None:
        break;
    case AppCUI::Input::MouseWheel::Up:
        if ((context->flags & GridFlags::DisableZoom) == GridFlags::None)
        {
            context->startedMoving = true;
            context->UpdateDimensions(1, 1);
            return true;
        }
        break;
    case AppCUI::Input::MouseWheel::Down:
        if ((context->flags & GridFlags::DisableZoom) == GridFlags::None)
        {
            context->startedMoving = true;
            context->UpdateDimensions(-1, -1);
            return true;
        }
        break;
    case AppCUI::Input::MouseWheel::Left:
        break;
    case AppCUI::Input::MouseWheel::Right:
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
    context->hoveredCellIndex           = context->ComputeCellNumber(x, y);

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

bool Grid::OnEvent(Controls::Reference<Control>, Event eventType, int controlID)
{
    switch (eventType)
    {
    case Event::Command:
        // switch (controlID)
        //{
        // default:
        //    break;
        //}
        break;
    default:
        break;
    }

    return false;
}

uint32 Grid::GetCellsCount() const
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);
    return context->columnsNo * context->rowsNo;
}

void Controls::Grid::SetGridDimensions(const Graphics::Size& dimensions)
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);
    context->columnsNo = dimensions.Width;
    context->rowsNo    = dimensions.Height;

    context->UpdateGridParameters();

    context->columnsSort.insert(context->columnsSort.end(), context->columnsNo, true);
    context->cells.clear();
}

Size Grid::GetGridDimensions() const
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);
    return { context->columnsNo, context->rowsNo };
}

bool Grid::UpdateCell(uint32 index, ConstString content, TextAlignament textAlignment, bool sort)
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);
    CHECK(index < context->columnsNo * context->rowsNo, false, "");

    Utils::UnicodeStringBuilder usb{ content };
    std::u16string u16s(usb);
    context->cells[index] = { textAlignment, u16s };

    if ((context->flags & GridFlags::Sort) != GridFlags::None)
    {
        if (sort)
        {
            context->SortColumn(index % context->rowsNo);
        }
    }

    if ((context->flags & GridFlags::DisableDuplicates) == GridFlags::None)
    {
        context->FindDuplicates();
    }

    return true;
}

bool Grid::UpdateCell(uint32 x, uint32 y, ConstString content, Graphics::TextAlignament textAlignment, bool sort)
{
    const auto context   = reinterpret_cast<GridControlContext*>(Context);
    const auto cellIndex = context->columnsNo * y + x;
    CHECK(UpdateCell(cellIndex, content, textAlignment), false, "");

    if ((context->flags & GridFlags::Sort) != GridFlags::None)
    {
        if (sort)
        {
            context->SortColumn(x);
        }
    }

    return true;
}

const ConstString Grid::GetSeparator() const
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);
    return context->separator;
}

void Grid::SetSeparator(ConstString separator)
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);
    Utils::UnicodeStringBuilder usb{ separator };
    context->separator = usb;
}

bool Grid::UpdateHeaderValues(const std::vector<ConstString>& headerValues, TextAlignament textAlignment)
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);

    context->headers.clear();
    for (const auto& value : headerValues)
    {
        LocalUnicodeStringBuilder<1024> lusb{ value };
        context->headers.push_back({ textAlignment, lusb });
    }

    if ((context->flags & GridFlags::Sort) != GridFlags::None)
    {
        for (auto i = 0U; i < context->columnsNo; i++)
        {
            context->SortColumn(i);
        }
    }

    if ((context->flags & GridFlags::DisableDuplicates) == GridFlags::None)
    {
        context->FindDuplicates();
    }

    return true;
}

Point Controls::Grid::GetHoveredLocation() const
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);

    if (context->hoveredCellIndex == 0xFFFFFFFF)
    {
        return { -1, -1 };
    }

    return { static_cast<int32>(context->hoveredCellIndex / context->columnsNo),
             static_cast<int32>(context->hoveredCellIndex % context->columnsNo) };
}

AppCUI::Graphics::Point Controls::Grid::GetSelectionLocationsStart() const
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);

    if (context->selectedCellsIndexes.size() == 0)
    {
        return { -1, -1 };
    }

    const auto cell = context->selectedCellsIndexes[0];
    if (cell == 0xFFFFFFFF)
    {
        return { -1, -1 };
    }

    return { static_cast<int32>(cell / context->columnsNo), static_cast<int32>(cell % context->columnsNo) };
}

AppCUI::Graphics::Point Controls::Grid::GetSelectionLocationsEnd() const
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);

    if (context->selectedCellsIndexes.size() == 0)
    {
        return { -1, -1 };
    }

    const auto cell = context->selectedCellsIndexes[context->selectedCellsIndexes.size() - 1];
    if (cell == 0xFFFFFFFF)
    {
        return { -1, -1 };
    }

    return { static_cast<int32>(cell / context->columnsNo), static_cast<int32>(cell % context->columnsNo) };
}

void Controls::Grid::SetDefaultHeaderValues()
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);
    context->SetDefaultHeaderValues();
}

void Controls::Grid::ToggleHorizontalLines()
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);
    if ((context->flags & GridFlags::HideHorizontalLines) == GridFlags::None)
    {
        context->flags |= GridFlags::HideHorizontalLines;
    }
    else
    {
        context->flags = context->flags & (~static_cast<uint32>(GridFlags::HideHorizontalLines));
    }

    context->UpdateGridParameters();
    Application::Repaint();
}

void Controls::Grid::ToggleVerticalLines()
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);
    if ((context->flags & GridFlags::HideVerticalLines) == GridFlags::None)
    {
        context->flags |= GridFlags::HideVerticalLines;
    }
    else
    {
        context->flags = context->flags & (~static_cast<uint32>(GridFlags::HideVerticalLines));
    }

    context->UpdateGridParameters();
    Application::Repaint();
}

void Controls::Grid::Sort()
{
    auto context = reinterpret_cast<GridControlContext*>(Context);
    if ((context->flags & GridFlags::Sort) != GridFlags::None)
    {
        for (auto i = 0U; i < context->columnsNo; i++)
        {
            context->SortColumn(i);
        }
    }

    if ((context->flags & GridFlags::DisableDuplicates) == GridFlags::None)
    {
        context->FindDuplicates();
    }
}

void GridControlContext::DrawBoxes(Renderer& renderer)
{
    if ((flags & GridFlags::HideHorizontalLines) == GridFlags::None &&
        (flags & GridFlags::HideVerticalLines) == GridFlags::None)
    {
        for (auto i = 0U; i <= columnsNo; i++)
        {
            const auto x = offsetX + i * cWidth;
            for (auto j = 0U; j <= rowsNo; j++)
            {
                const auto y = offsetY + j * cHeight;

                const auto sc = ComputeBoxType(i, j, 0U, 0U, columnsNo, rowsNo);
                renderer.WriteSpecialCharacter(x, y, sc, Cfg->Grid.Lines.Normal);
            }
        }
    }
    else if ((flags & GridFlags::HideVerticalLines) == GridFlags::None)
    {
        const auto y1 = offsetY + 1;
        const auto y2 = offsetY + rowsNo * cHeight - 1;
        for (auto i = 0U; i <= columnsNo; i++)
        {
            const auto x = offsetX + i * cWidth;
            renderer.DrawVerticalLine(x, y1, y2, Cfg->Grid.Lines.Normal);
        }
    }

    if (hoveredCellIndex != InvalidCellIndex && ((flags & GridFlags::HideHoveredCell) == GridFlags::None))
    {
        const auto columnIndex = hoveredCellIndex % columnsNo;
        const auto rowIndex    = hoveredCellIndex / columnsNo;

        const auto xLeft  = offsetX + columnIndex * cWidth;
        const auto xRight = offsetX + (columnIndex + 1) * cWidth;

        const auto yTop    = offsetY + rowIndex * cHeight;
        const auto yBottom = offsetY + (rowIndex + 1) * cHeight;

        const auto& color = Cfg->Grid.Lines.Hovered;

        renderer.WriteSpecialCharacter(xLeft, yTop, SpecialChars::BoxTopLeftCornerSingleLine, color);
        renderer.WriteSpecialCharacter(xRight, yTop, SpecialChars::BoxTopRightCornerSingleLine, color);
        renderer.WriteSpecialCharacter(xLeft, yBottom, SpecialChars::BoxBottomLeftCornerSingleLine, color);
        renderer.WriteSpecialCharacter(xRight, yBottom, SpecialChars::BoxBottomRightCornerSingleLine, color);

        renderer.DrawVerticalLine(xLeft, yTop + 1, yBottom - 1, color);
        renderer.DrawVerticalLine(xRight, yTop + 1, yBottom - 1, color);
    }

    if (selectedCellsIndexes.size() > 0 && ((flags & GridFlags::HideSelectedCell) == GridFlags::None))
    {
        // we assume these are already sorted
        const auto startCellIndex = selectedCellsIndexes[0];
        const auto endCellIndex   = selectedCellsIndexes[selectedCellsIndexes.size() - 1];

        const auto sci = startCellIndex % columnsNo;
        const auto sri = startCellIndex / columnsNo;

        const auto eci = endCellIndex % columnsNo + 1U;
        const auto eri = endCellIndex / columnsNo + 1U;

        const auto y1 = offsetY + sri * cHeight;
        const auto y2 = y1 + (eri - sri) * cHeight - 1;
        for (auto i = sci; i <= eci; i++)
        {
            const auto x = offsetX + i * cWidth;
            renderer.DrawVerticalLine(x, y1, y2, Cfg->Grid.Lines.Selected);
        }

        for (auto i = sci; i <= eci; i++)
        {
            const auto x = offsetX + i * cWidth;
            for (auto j = sri; j <= eri; j++)
            {
                const auto y  = offsetY + j * cHeight;
                const auto sc = ComputeBoxType(i, j, sci, sri, eci, eri);
                renderer.WriteSpecialCharacter(x, y, sc, Cfg->Grid.Lines.Selected);
            }
        }
    }
}

void GridControlContext::DrawLines(Renderer& renderer)
{
    for (auto i = 0U; i <= columnsNo; i++)
    {
        const auto x = offsetX + i * cWidth;
        for (auto j = 0U; j <= rowsNo; j++)
        {
            const auto y = offsetY + j * cHeight;

            if ((flags & GridFlags::HideHorizontalLines) == GridFlags::None)
            {
                if (i < columnsNo)
                {
                    const auto endX = offsetX + (i + 1) * cWidth;
                    renderer.DrawHorizontalLine(x + 1, y, endX - 1, Cfg->Grid.Lines.Normal, true);
                }
            }

            if ((flags & GridFlags::HideVerticalLines) == GridFlags::None)
            {
                if (j < rowsNo)
                {
                    const auto endY = offsetY + (j + 1) * cHeight;
                    renderer.DrawVerticalLine(x, y + 1, endY - 1, Cfg->Grid.Lines.Normal, true);
                }
            }
        }
    }

    const auto drawLines = [&](uint32 cellIndex, GridCellStatus cellType)
    {
        ColorPair vertical   = Cfg->Grid.Lines.Normal;
        ColorPair horizontal = Cfg->Grid.Lines.Normal;

        switch (cellType)
        {
        case GridCellStatus::Normal:
            vertical   = Cfg->Grid.Lines.Normal;
            horizontal = Cfg->Grid.Lines.Normal;
            break;
        case GridCellStatus::Selected:
            vertical   = Cfg->Grid.Lines.Selected;
            horizontal = Cfg->Grid.Lines.Selected;
            break;
        case GridCellStatus::Hovered:
            vertical   = Cfg->Grid.Lines.Hovered;
            horizontal = Cfg->Grid.Lines.Hovered;
            break;
        case GridCellStatus::Duplicate:
            vertical   = Cfg->Grid.Lines.Duplicate;
            horizontal = Cfg->Grid.Lines.Duplicate;
            break;
        default:
            vertical   = Cfg->Grid.Lines.Normal;
            horizontal = Cfg->Grid.Lines.Normal;
            break;
        }

        const auto columnIndex = cellIndex % columnsNo;
        const auto rowIndex    = cellIndex / columnsNo;

        const auto xLeft  = offsetX + columnIndex * cWidth;
        const auto xRight = offsetX + (columnIndex + 1) * cWidth;

        const auto yTop    = offsetY + rowIndex * cHeight;
        const auto yBottom = offsetY + (rowIndex + 1) * cHeight;

        renderer.DrawVerticalLine(xLeft, yTop + 1, yBottom - 1, vertical, true);
        renderer.DrawVerticalLine(xRight, yTop + 1, yBottom - 1, vertical, true);

        renderer.DrawHorizontalLine(xLeft + 1, yTop, xRight - 1, horizontal, true);
        renderer.DrawHorizontalLine(xLeft + 1, yBottom, xRight - 1, horizontal, true);
    };

    if (hoveredCellIndex != InvalidCellIndex && ((flags & GridFlags::HideHoveredCell) == GridFlags::None))
    {
        drawLines(hoveredCellIndex, GridCellStatus::Hovered);
    }

    if (duplicatedCellsIndexes.size() > 0 && ((flags & GridFlags::DisableDuplicates) == GridFlags::None))
    {
        for (const auto& cellIndex : duplicatedCellsIndexes)
        {
            drawLines(cellIndex, GridCellStatus::Duplicate);
        }
    }

    if (selectedCellsIndexes.size() > 0 && ((flags & GridFlags::HideSelectedCell) == GridFlags::None))
    {
        for (const auto& cellIndex : selectedCellsIndexes)
        {
            drawLines(cellIndex, GridCellStatus::Selected);
        }
    }
}

uint32 GridControlContext::ComputeCellNumber(int32 x, int32 y)
{
    const auto endX = static_cast<uint32>(offsetX + cWidth * columnsNo);
    const auto endY = static_cast<uint32>(offsetY + cHeight * rowsNo);

    if (x <= offsetX || static_cast<uint32>(x) >= endX)
    {
        return InvalidCellIndex;
    }

    if (y <= offsetY || static_cast<uint32>(y) >= endY)
    {
        return InvalidCellIndex;
    }

    const auto columnIndex = (static_cast<uint32>(x) - offsetX) / cWidth;
    const auto rowIndex    = (static_cast<uint32>(y) - offsetY) / cHeight;
    const auto cellIndex   = columnsNo * rowIndex + columnIndex;

    return cellIndex;
}

SpecialChars GridControlContext::ComputeBoxType(
      uint32 colIndex,
      uint32 rowIndex,
      uint32 startColumnsIndex,
      uint32 startRowsIndex,
      uint32 endColumnsIndex,
      uint32 endRowsIndex)
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

void GridControlContext::DrawCellsBackground(Graphics::Renderer& renderer)
{
    for (auto i = 0U; i < columnsNo; i++)
    {
        for (auto j = 0U; j < rowsNo; j++)
        {
            DrawCellBackground(renderer, GridCellStatus::Normal, i, j);
        }
    }

    if (duplicatedCellsIndexes.size() > 0 && ((flags & GridFlags::DisableDuplicates) == GridFlags::None))
    {
        for (const auto& cellIndex : duplicatedCellsIndexes)
        {
            DrawCellBackground(renderer, GridCellStatus::Duplicate, cellIndex);
        }
    }

    if (hoveredCellIndex != InvalidCellIndex && ((flags & GridFlags::HideHoveredCell) == GridFlags::None))
    {
        DrawCellBackground(renderer, GridCellStatus::Hovered, hoveredCellIndex);
    }

    if (selectedCellsIndexes.size() > 0 && ((flags & GridFlags::HideSelectedCell) == GridFlags::None))
    {
        for (const auto& cellIndex : selectedCellsIndexes)
        {
            DrawCellBackground(renderer, GridCellStatus::Selected, cellIndex);
        }
    }
}

void GridControlContext::DrawCellBackground(Graphics::Renderer& renderer, GridCellStatus cellType, uint32 i, uint32 j)
{
    const auto xLeft  = offsetX + i * cWidth;
    const auto xRight = offsetX + (i + 1) * cWidth;

    const auto yTop    = offsetY + j * cHeight;
    const auto yBottom = offsetY + (j + 1) * cHeight;

    ColorPair color = Cfg->Grid.Background.Cell.Normal;
    switch (cellType)
    {
    case GridCellStatus::Normal:
        color = Cfg->Grid.Background.Cell.Normal;
        break;
    case GridCellStatus::Selected:
        color = Cfg->Grid.Background.Cell.Selected;
        break;
    case GridCellStatus::Hovered:
        color = Cfg->Grid.Background.Cell.Hovered;
        break;
    case GridCellStatus::Duplicate:
        color = Cfg->Grid.Background.Cell.Duplicate;
        break;
    default:
        color = Cfg->Grid.Background.Cell.Normal;
        break;
    }

    renderer.FillRect(xLeft + 1, yTop + 1, xRight - 1, yBottom - 1, ' ', color);
}

void GridControlContext::DrawCellBackground(Graphics::Renderer& renderer, GridCellStatus cellType, uint32 cellIndex)
{
    const auto columnIndex = cellIndex % columnsNo;
    const auto rowIndex    = cellIndex / columnsNo;

    DrawCellBackground(renderer, cellType, columnIndex, rowIndex);
}

bool GridControlContext::DrawCellContent(Graphics::Renderer& renderer, uint32 cellIndex)
{
    const auto cellColumn = cellIndex % columnsNo;
    const auto cellRow    = cellIndex / columnsNo;

    const auto x = offsetX + cellColumn * cWidth + 1; // + 1 -> line
    const auto y = offsetY + cellRow * cHeight + 1;   // + 1 -> line

    const auto& data = cells[cellIndex];

    auto color = Cfg->Grid.Text.Normal;
    if (cellIndex == hoveredCellIndex)
    {
        color = Cfg->Grid.Text.Hovered;
    }
    else if (
          std::find(selectedCellsIndexes.begin(), selectedCellsIndexes.end(), cellIndex) != selectedCellsIndexes.end())
    {
        color = Cfg->Grid.Text.Selected;
    }
    else if (
          std::find(duplicatedCellsIndexes.begin(), duplicatedCellsIndexes.end(), cellIndex) !=
          duplicatedCellsIndexes.end())
    {
        color = Cfg->Grid.Text.Duplicate;
    }

    WriteTextParams wtp;
    wtp.Flags = WriteTextFlags::MultipleLines | WriteTextFlags::ClipToWidth | WriteTextFlags::FitTextToWidth;
    wtp.Color = color;
    wtp.X     = x;
    wtp.Y     = y;
    wtp.Width = cWidth - 1;
    wtp.Align = data.ta;

    renderer.WriteText(data.content, wtp);

    return false;
}

bool GridControlContext::DrawHeader(Graphics::Renderer& renderer)
{
    renderer.FillRect(
          offsetX + 1, offsetY - cHeight + 1, cWidth * columnsNo + offsetX - 1, offsetY - 1, ' ', Cfg->Grid.Header);

    for (auto i = 0U; i <= columnsNo; i++)
    {
        const auto x    = offsetX + i * cWidth;
        const auto y    = offsetY - cHeight + 1;
        const auto endY = offsetY + cHeight;

        if ((flags & GridFlags::HideVerticalLines) == GridFlags::None)
        {
            renderer.DrawVerticalLine(x, y, endY + 10, Cfg->Grid.Lines.Normal, true);
        }
    }

    if ((flags & GridFlags::HideHorizontalLines) == GridFlags::None)
    {
        renderer.DrawHorizontalLine(
              offsetX, offsetY - cHeight, cWidth * columnsNo + offsetX, Cfg->Grid.Lines.Normal, true);
    }

    if ((flags & GridFlags::HideBoxes) == GridFlags::None)
    {
        if ((flags & GridFlags::HideHorizontalLines) == GridFlags::None)
        {
            for (auto i = 0U; i <= columnsNo; i++)
            {
                const auto x = offsetX + i * cWidth;
                const auto y = offsetY - cHeight;

                if (i == 0)
                {
                    renderer.WriteSpecialCharacter(
                          x, y, SpecialChars::BoxTopLeftCornerSingleLine, Cfg->Grid.Lines.Normal);
                }
                else if (i == columnsNo)
                {
                    renderer.WriteSpecialCharacter(
                          x, y, SpecialChars::BoxTopRightCornerSingleLine, Cfg->Grid.Lines.Normal);
                }
                else
                {
                    renderer.WriteSpecialCharacter(x, y, SpecialChars::BoxMidleTop, Cfg->Grid.Lines.Normal);
                }
            }
        }
    }

    WriteTextParams wtp;
    wtp.Flags = WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth | WriteTextFlags::FitTextToWidth;
    wtp.Color = Cfg->Grid.Text.Normal;

    auto it = headers.begin();
    for (auto i = 0U; i <= columnsNo && it != headers.end(); i++)
    {
        wtp.X     = offsetX + i * cWidth + 1; // 1 -> line
        wtp.Y     = offsetY - cHeight / 2;
        wtp.Width = cWidth - 1; // 1 -> line
        wtp.Align = it->ta;

        renderer.WriteText(it->content, wtp);

        if ((flags & GridFlags::Sort) != GridFlags::None)
        {
            const auto endX = wtp.X + cWidth - 1;
            renderer.WriteSpecialCharacter(
                  endX - 1,
                  wtp.Y,
                  columnsSort[i] ? SpecialChars::TriangleUp : SpecialChars::TriangleDown,
                  { Color::Black, Color::Transparent });
        }

        std::advance(it, 1);
    }

    return true;
}

void GridControlContext::UpdateGridParameters(bool dontRecomputeDimensions)
{
    // define cell dimensions
    if (dontRecomputeDimensions == false || cWidth == 0 || cHeight == 0)
    {
        cWidth  = columnsNo == 0 ? 0 : static_cast<uint32>(Layout.Width / columnsNo);
        cHeight = rowsNo == 0 ? 0 : static_cast<uint32>(Layout.Height / rowsNo);
        if (Layout.Width != 0 || Layout.Height != 0)
        {
            cWidth  = std::max<>(cWidth + 1, minCellWidth);
            cHeight = std::max<>(
                  cHeight + 1 - ((flags & GridFlags::HideHorizontalLines) != GridFlags::None), minCellHeight);
        }
    }

    if (((flags & GridFlags::DisableMove) != GridFlags::DisableMove) && startedMoving == false)
    {
        ResetMatrixPosition();
    }

    // sort selected cells for better drawing
    struct
    {
        const GridControlContext* gcc;
        bool operator()(uint32 a, uint32 b)
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
    } sortingComparator{ this };

    std::sort(selectedCellsIndexes.begin(), selectedCellsIndexes.end(), sortingComparator);
    selectedCellsIndexes.erase(
          std::unique(selectedCellsIndexes.begin(), selectedCellsIndexes.end()), selectedCellsIndexes.end());
}

void GridControlContext::UpdateDimensions(int32 offsetX, int32 offsetY)
{
    cWidth += offsetX;
    cHeight += offsetY;

    cWidth  = std::max<>(minCellWidth, cWidth);
    cHeight = std::max<>(minCellHeight, cHeight);

    UpdateGridParameters(true);
}

void GridControlContext::ResetMatrixPosition()
{
    offsetX = -1;
    offsetY = cHeight - 1; /* header */
}

void GridControlContext::UpdatePositions(int32 offsetX, int32 offsetY)
{
    this->offsetX += offsetX;
    this->offsetY += offsetY;

    UpdateGridParameters(true);
}

bool GridControlContext::MoveSelectedCellByKeys(Input::Key keyCode)
{
    if (selectedCellsIndexes.size() == 0)
    {
        anchorCellIndex = 0;
        selectedCellsIndexes.emplace_back(0);
        return true;
    }

    if (selectedCellsIndexes.size() == 1)
    {
        const auto index = selectedCellsIndexes[0];
        auto columnIndex = index % columnsNo;
        auto rowIndex    = index / columnsNo;

        if (columnIndex > 0)
        {
            columnIndex -= (keyCode == Input::Key::Left);
        }
        if (columnIndex < columnsNo - 1)
        {
            columnIndex += (keyCode == Input::Key::Right);
        }

        if (rowIndex > 0)
        {
            rowIndex -= (keyCode == Input::Key::Up);
        }
        if (rowIndex < rowsNo - 1)
        {
            rowIndex += (keyCode == Input::Key::Down);
        }

        const auto newCellIndex = columnsNo * rowIndex + columnIndex;
        if (newCellIndex != index)
        {
            anchorCellIndex         = newCellIndex;
            selectedCellsIndexes[0] = newCellIndex;

            const int xLeft  = offsetX + columnIndex * cWidth;
            const int xRight = offsetX + (columnIndex + 1) * cWidth;

            const int yTop    = offsetY + rowIndex * cHeight;
            const int yBottom = offsetY + (rowIndex + 1) * cHeight;

            if (xLeft < 0)
            {
                const auto partial = std::abs(offsetX) % cWidth;
                offsetX += (partial != 0 ? partial : cWidth);
                startedMoving = true;
            }

            if (xRight > Layout.Width)
            {
                offsetX -= xRight - Layout.Width + 1;
                startedMoving = true;
            }

            if (yTop < 0)
            {
                const auto partial = std::abs(offsetY) % cHeight;
                offsetY += (partial != 0 ? partial : cHeight);
                startedMoving = true;
            }

            if (yBottom > Layout.Height)
            {
                offsetY -= yBottom - Layout.Height + 1;
                startedMoving = true;
            }

            if ((flags & GridFlags::DisableDuplicates) == GridFlags::None)
            {
                if (selectedCellsIndexes.size() == 1)
                {
                    FindDuplicates();
                }
            }

            return true;
        }
    }

    return false;
}

bool GridControlContext::SelectCellsByKeys(Input::Key keyCode)
{
    if (selectedCellsIndexes.size() == 0)
    {
        anchorCellIndex = 0;
        selectedCellsIndexes.emplace_back(0);
        return true;
    }

    const auto anchorColumnIndex = anchorCellIndex % columnsNo;
    const auto anchorRowIndex    = anchorCellIndex / columnsNo;

    auto xLeft  = anchorColumnIndex;
    auto xRight = anchorColumnIndex;

    auto yTop = anchorRowIndex;
    auto yBot = anchorRowIndex;

    for (const auto& i : selectedCellsIndexes)
    {
        const auto colIndex = i % columnsNo;
        const auto rowIndex = i / columnsNo;

        xLeft  = std::min<>(xLeft, colIndex);
        xRight = std::max<>(xRight, colIndex);

        yTop = std::min<>(yTop, rowIndex);
        yBot = std::max<>(yBot, rowIndex);
    }

    const auto topLeft     = columnsNo * yTop + xLeft;
    const auto topRight    = columnsNo * yTop + xRight;
    const auto bottomLeft  = columnsNo * yBot + xLeft;
    const auto bottomRight = columnsNo * yBot + xRight;

    if (selectedCellsIndexes.size() == 1)
    {
        switch (keyCode)
        {
        case Input::Key::Shift | Input::Key::Left:
            if (xLeft > 0)
                xLeft -= 1;
            break;
        case Input::Key::Shift | Input::Key::Right:
            if (xRight < columnsNo - 1)
                xRight += 1;
            break;
        case Input::Key::Shift | Input::Key::Up:
            if (yBot > 0)
                yBot -= 1;
            break;
        case Input::Key::Shift | Input::Key::Down:
            if (yBot < rowsNo - 1)
                yBot += 1;
            break;
        default:
            break;
        }
    }
    else if (topLeft == anchorCellIndex)
    {
        switch (keyCode)
        {
        case Input::Key::Shift | Input::Key::Left:
            if (xRight > 0)
                xRight -= 1;
            break;
        case Input::Key::Shift | Input::Key::Right:
            if (xRight < columnsNo - 1)
                xRight += 1;
            break;
        case Input::Key::Shift | Input::Key::Up:
            if (yBot > 0)
                yBot -= 1;
            break;
        case Input::Key::Shift | Input::Key::Down:
            if (yBot < rowsNo - 1)
                yBot += 1;
            break;
        default:
            break;
        }
    }
    else if (topRight == anchorCellIndex)
    {
        switch (keyCode)
        {
        case Input::Key::Shift | Input::Key::Left:
            if (xLeft > 0)
                xLeft -= 1;
            break;
        case Input::Key::Shift | Input::Key::Right:
            if (xLeft < columnsNo - 1)
                xLeft += 1;
            break;
        case Input::Key::Shift | Input::Key::Up:
            if (yBot > 0)
                yBot -= 1;
            break;
        case Input::Key::Shift | Input::Key::Down:
            if (yBot < rowsNo - 1)
                yBot += 1;
            break;
        default:
            break;
        }
    }
    else if (bottomLeft == anchorCellIndex)
    {
        switch (keyCode)
        {
        case Input::Key::Shift | Input::Key::Left:
            if (xRight > 0)
                xRight -= 1;
            break;
        case Input::Key::Shift | Input::Key::Right:
            if (xRight < columnsNo - 1)
                xRight += 1;
            break;
        case Input::Key::Shift | Input::Key::Up:
            if (yTop > 0)
                yTop -= 1;
            break;
        case Input::Key::Shift | Input::Key::Down:
            if (yTop < rowsNo - 1)
                yTop += 1;
            break;
        default:
            break;
        }
    }
    else if (bottomRight == anchorCellIndex)
    {
        switch (keyCode)
        {
        case Input::Key::Shift | Input::Key::Left:
            if (xLeft > 0)
                xLeft -= 1;
            break;
        case Input::Key::Shift | Input::Key::Right:
            if (xLeft < columnsNo - 1)
                xLeft += 1;
            break;
        case Input::Key::Shift | Input::Key::Up:
            if (yTop > 0)
                yTop -= 1;
            break;
        case Input::Key::Shift | Input::Key::Down:
            if (yTop < rowsNo - 1)
                yTop += 1;
            break;
        default:
            break;
        }
    }

    selectedCellsIndexes.clear();
    for (auto i = std::min<>(xLeft, xRight); i <= std::max<>(xLeft, xRight); i++)
    {
        for (auto j = std::min<>(yBot, yTop); j <= std::max<>(yBot, yTop); j++)
        {
            const auto current = columnsNo * j + i;
            selectedCellsIndexes.emplace_back(current);
        }
    }

    if ((flags & GridFlags::DisableDuplicates) == GridFlags::None)
    {
        if (selectedCellsIndexes.size() == 1)
        {
            FindDuplicates();
        }
    }

    return true;
}

bool GridControlContext::CopySelectedCellsContent() const
{
    auto xLeft  = 0xFFFFFFFFU;
    auto xRight = 0U;

    auto yTop = 0xFFFFFFFFU;
    auto yBot = 0U;

    for (const auto& i : selectedCellsIndexes)
    {
        const auto colIndex = i % columnsNo;
        const auto rowIndex = i / columnsNo;

        xLeft  = std::min<>(xLeft, colIndex);
        xRight = std::max<>(xRight, colIndex);

        yTop = std::min<>(yTop, rowIndex);
        yBot = std::max<>(yBot, rowIndex);
    }

    LocalUnicodeStringBuilder<2048> lusb;
    for (auto j = std::min<>(yBot, yTop); j <= std::max<>(yBot, yTop); j++)
    {
        for (auto i = std::min<>(xLeft, xRight); i <= std::max<>(xLeft, xRight); i++)
        {
            ConstString cs;
            const auto current = columnsNo * j + i;
            const auto& it     = cells.find(current);
            if (it != cells.end())
            {
                const auto& data = it->second;
                cs               = data.content;
            }

            lusb.Add(cs);

            if (i < std::max<>(xLeft, xRight))
            {
                lusb.Add(separator);
            }
        }
        lusb.Add("\n");
    }

    if (OS::Clipboard::SetText(lusb) == false)
    {
        const std::string input{ lusb };
        LOG_WARNING("Failed to copy string [%s] to the clipboard!", input.c_str());
        return false;
    }

    return false;
}

bool GridControlContext::PasteContentToSelectedCells()
{
    // seems slow - a lower level parser might be better - we'll see
    LocalUnicodeStringBuilder<2048> lusb{};
    OS::Clipboard::GetText(lusb);

    const std::u16string input{ lusb };

    uint64 last = 0;
    uint64 next = 0;
    vector<std::u16string> lines;
    lines.reserve(50);
    while ((next = input.find(u"\n", last)) != std::string::npos)
    {
        lines.emplace_back(input.substr(last, next - last));
        last = next + separator.length();
    }
    const auto lastLine = input.substr(last);
    if (lastLine != u"")
    {
        lines.emplace_back(lastLine);
    }

    vector<std::u16string> tokens;
    tokens.reserve(50);
    for (const auto& line : lines)
    {
        uint64 last = 0;
        uint64 next = 0;
        while ((next = line.find(separator, last)) != std::string::npos)
        {
            tokens.emplace_back(line.substr(last, next - last));
            last = next + separator.length();
        }
        tokens.emplace_back(line.substr(last));
    }

    if (tokens.size() > selectedCellsIndexes.size())
    {
        const auto start = tokens.begin() + selectedCellsIndexes.size() - 1U;

        LocalUnicodeStringBuilder<2048> lusbLastToken;
        for (vector<std::u16string>::iterator i = start; i != tokens.end(); i++)
        {
            lusbLastToken.Add(*i);
        }

        tokens.erase(start, tokens.end());

        std::u16string lastToken{ lusbLastToken };
        tokens.emplace_back(lastToken);
    }

    auto index = selectedCellsIndexes.begin();
    for (const auto& token : tokens)
    {
        auto& data   = cells.at(*index);
        data.content = token;

        std::advance(index, 1);
    }

    if ((flags & GridFlags::Sort) != GridFlags::None)
    {
        for (auto i = 0U; i < columnsNo; i++)
        {
            SortColumn(i);
        }
    }

    return true;
}

void GridControlContext::SetDefaultHeaderValues()
{
    headers.clear();
    LocalUnicodeStringBuilder<256> lusb;
    LocalString<256> ls;
    for (auto i = 0ULL; i < columnsNo; i++)
    {
        ls.SetFormat("Column_%llu", i);
        lusb.Set(ls);
        headers.push_back({ TextAlignament::Left, lusb });
    }
}

void GridControlContext::ReserveMap()
{
    for (auto i = 0U; i < columnsNo * rowsNo; i++)
    {
        cells[i] = { Graphics::TextAlignament::Left, u"" };
    }
}

void GridControlContext::ToggleSorting(int x, int y)
{
    auto it = headers.begin();
    for (auto i = 0U; i <= columnsNo && it != headers.end(); i++)
    {
        const auto xHeader    = offsetX + i * cWidth + 1; // 1 -> line
        const auto yHeader    = offsetY - cHeight / 2;
        const auto endXHeader = xHeader + cWidth - 2;

        if (x == endXHeader && y == yHeader)
        {
            columnsSort[i] = !columnsSort[i];
            SortColumn(i);
            break;
        }

        std::advance(it, 1);
    }
}

void GridControlContext::SortColumn(int colIndex)
{
    CHECKRET(cells.size() > 0, "");

    // this is not that efficient - you could replace it with indexes, sort and then swap
    std::vector<GridCellData> column;
    column.reserve(rowsNo);
    for (auto j = 0U; j < rowsNo; j++)
    {
        const auto cell = colIndex + j * columnsNo;
        column.emplace_back(cells.at(cell));
    }

    std::sort(
          column.begin(),
          column.end(),
          [](const GridCellData& a, const GridCellData& b) -> bool { return a.content.compare(b.content) < 0; });

    if (columnsSort[colIndex] == false)
    {
        std::reverse(column.begin(), column.end());
    }

    for (auto j = 0U; j < rowsNo; j++)
    {
        const auto cell = colIndex + j * columnsNo;
        cells[cell]     = column[j];
    }
}

void GridControlContext::FindDuplicates()
{
    duplicatedCellsIndexes.clear();
    CHECKRET(selectedCellsIndexes.size() == 1, "");

    const auto& content = cells[selectedCellsIndexes[0]].content;
    for (const auto& [key, value] : cells)
    {
        if (content.compare(value.content) == 0)
        {
            duplicatedCellsIndexes.emplace_back(key);
        }
    }

    // sort duplicate cells for better drawing
    struct
    {
        const GridControlContext* gcc;
        bool operator()(uint32 a, uint32 b)
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
    } sortingComparator{ this };
    std::sort(duplicatedCellsIndexes.begin(), duplicatedCellsIndexes.end(), sortingComparator);
    duplicatedCellsIndexes.erase(
          std::unique(duplicatedCellsIndexes.begin(), duplicatedCellsIndexes.end()), duplicatedCellsIndexes.end());
}
} // namespace AppCUI
