#include "ControlContext.hpp"

namespace AppCUI
{
using namespace Input;
using namespace Graphics;

constexpr auto InvalidCellIndex = 0xFFFFFFFFU;

constexpr auto MenuCommandMergeCells = 0x01U;

constexpr auto minCellWidth  = 0x03U;
constexpr auto minCellHeight = 0x03U;

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

    context->CenterMatrix();
    context->UpdateGridParameters();

    context->rightClickMenu.AddCommandItem("&Merge Cells", MenuCommandMergeCells, Key::F2);
    context->headers.reserve(context->columnsNo);
}

void Grid::Paint(Renderer& renderer)
{
    auto context = reinterpret_cast<GridControlContext*>(Context);
    context->UpdateGridParameters(true);

    if ((context->flags & GridFlags::HideHeader) == GridFlags::None)
    {
        context->DrawHeader(renderer);
    }

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

bool Grid::OnKeyEvent(Input::Key keyCode, char16_t UnicodeChar)
{
    auto context = reinterpret_cast<GridControlContext*>(Context);

    switch (keyCode)
    {
    case Input::Key::Space:
        if (context->ToggleBooleanCell())
        {
            return true;
        }
        break;
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
            return true;
        }
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
    case MouseButton::Left | MouseButton::DoubleClicked:
    {
        const auto index = context->ComputeCellNumber(x, y);
        if (index == InvalidCellIndex)
        {
            break;
        }

        auto& cellData      = context->cells[index];
        const auto cellType = cellData.ct;
        if (cellType != Grid::CellType::Boolean)
        {
            break;
        }

        auto& content = cellData.content;
        if (std::holds_alternative<bool>(content) == false)
        {
            break;
        }

        const auto value = std::get<bool>(content);
        content          = !value;
    }
    break;
    case MouseButton::Center | MouseButton::DoubleClicked:
        break;
    case MouseButton::Right | MouseButton::DoubleClicked:
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

        return true;
    }
    break;
    case Input::MouseButton::Center:
        break;
    case Input::MouseButton::Right:
        break;
    case Input::MouseButton::DoubleClicked:
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

uint32 Grid::GetCellsCount() const
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);
    return context->columnsNo * context->rowsNo;
}

Size Grid::GetGridDimensions() const
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);
    return { context->columnsNo, context->rowsNo };
}

bool Grid::UpdateCell(
      uint32 index, CellType cellType, const std::variant<bool, ConstString>& content, TextAlignament textAlignment)
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);
    CHECK(index < context->columnsNo * context->rowsNo, false, "");

    std::variant<bool, std::u16string> cellData;
    if (std::holds_alternative<bool>(content))
    {
        cellData = std::get<bool>(content);
    }
    else if (std::holds_alternative<ConstString>(content))
    {
        const auto& cs = std::get<ConstString>(content);
        Utils::UnicodeStringBuilder usb{ cs };
        std::u16string u16s(usb);
        cellData = u16s;
    }

    context->cells[index] = { textAlignment, cellType, cellData };

    return true;
}

bool Grid::UpdateCell(
      uint32 x,
      uint32 y,
      CellType cellType,
      const std::variant<bool, ConstString>& content,
      Graphics::TextAlignament textAlignment)
{
    const auto context   = reinterpret_cast<GridControlContext*>(Context);
    const auto cellIndex = context->columnsNo * y + x;
    CHECK(UpdateCell(cellIndex, cellType, content, textAlignment), false, "");

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

    return true;
}

void GridControlContext::DrawBoxes(Renderer& renderer)
{
    for (auto i = 0U; i <= columnsNo; i++)
    {
        const auto x = offsetX + i * cWidth;
        for (auto j = 0U; j <= rowsNo; j++)
        {
            const auto y  = offsetY + j * cHeight;
            const auto sc = ComputeBoxType(i, j, 0U, 0U, columnsNo, rowsNo);
            renderer.WriteSpecialCharacter(x, y, sc, Cfg->Grid.Lines.Box.Normal);
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

        const auto& color = Cfg->Grid.Lines.Box.Hovered;

        renderer.WriteSpecialCharacter(xLeft, yTop, SpecialChars::BoxTopLeftCornerSingleLine, color);
        renderer.WriteSpecialCharacter(xRight, yTop, SpecialChars::BoxTopRightCornerSingleLine, color);
        renderer.WriteSpecialCharacter(xLeft, yBottom, SpecialChars::BoxBottomLeftCornerSingleLine, color);
        renderer.WriteSpecialCharacter(xRight, yBottom, SpecialChars::BoxBottomRightCornerSingleLine, color);
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

        for (auto i = sci; i <= eci; i++)
        {
            const auto x = offsetX + i * cWidth;
            for (auto j = sri; j <= eri; j++)
            {
                const auto y  = offsetY + j * cHeight;
                const auto sc = ComputeBoxType(i, j, sci, sri, eci, eri);
                renderer.WriteSpecialCharacter(x, y, sc, Cfg->Grid.Lines.Box.Selected);
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
                    renderer.DrawHorizontalLine(x + 1, y, endX - 1, Cfg->Grid.Lines.Horizontal.Normal, true);
                }
            }

            if ((flags & GridFlags::HideVerticalLines) == GridFlags::None)
            {
                if (j < rowsNo)
                {
                    const auto endY = offsetY + (j + 1) * cHeight;
                    renderer.DrawVerticalLine(x, y + 1, endY - 1, Cfg->Grid.Lines.Vertical.Normal, true);
                }
            }
        }
    }

    const auto drawLines = [&](uint32 cellIndex, GridCellStatus cellType)
    {
        ColorPair vertical   = Cfg->Grid.Lines.Vertical.Normal;
        ColorPair horizontal = Cfg->Grid.Lines.Horizontal.Normal;

        switch (cellType)
        {
        case GridCellStatus::Normal:
            vertical   = Cfg->Grid.Lines.Vertical.Normal;
            horizontal = Cfg->Grid.Lines.Horizontal.Normal;
            break;
        case GridCellStatus::Selected:
            vertical   = Cfg->Grid.Lines.Vertical.Selected;
            horizontal = Cfg->Grid.Lines.Horizontal.Selected;
            break;
        case GridCellStatus::Hovered:
            vertical   = Cfg->Grid.Lines.Vertical.Hovered;
            horizontal = Cfg->Grid.Lines.Horizontal.Hovered;
            break;
        default:
            vertical   = Cfg->Grid.Lines.Vertical.Normal;
            horizontal = Cfg->Grid.Lines.Horizontal.Normal;
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
            if ((flags & GridFlags::HideHeader) == GridFlags::None)
            {
                return SpecialChars::BoxMidleLeft;
            }
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
            if ((flags & GridFlags::HideHeader) == GridFlags::None)
            {
                return SpecialChars::BoxMidleRight;
            }
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
            if ((flags & GridFlags::HideHeader) == GridFlags::None)
            {
                return SpecialChars::BoxCrossSingleLine;
            }
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

    WriteTextParams wtp;
    wtp.Flags = WriteTextFlags::MultipleLines | WriteTextFlags::ClipToWidth | WriteTextFlags::FitTextToWidth;
    wtp.Color = Cfg->Grid.Text.Normal;
    wtp.X     = x;
    wtp.Y     = y;
    wtp.Width = cWidth - 1;
    wtp.Align = data.ta;

    switch (data.ct)
    {
    case Grid::CellType::Boolean:
        if (std::holds_alternative<bool>(data.content))
        {
            const auto value = std::get<bool>(data.content);
            if (value)
            {
                renderer.WriteText("True", wtp);
            }
            else
            {
                renderer.WriteText("False", wtp);
            }
            return true;
        }
        return false;
    case Grid::CellType::String:
        if (std::holds_alternative<std::u16string>(data.content))
        {
            const auto& value = std::get<std::u16string>(data.content);
            renderer.WriteText(value, wtp);
            return true;
        }
        return false;
    default:
        break;
    }

    return false;
}

bool GridControlContext::DrawHeader(Graphics::Renderer& renderer)
{
    renderer.FillRect(
          offsetX + 1, offsetY - cHeight + 1, cWidth * columnsNo + offsetX - 1, offsetY - 1, ' ', Cfg->Grid.Header);

    for (auto i = 0U; i <= columnsNo; i++)
    {
        const auto x = offsetX + i * cWidth;
        const auto y = offsetY - cHeight;

        if ((flags & GridFlags::HideVerticalLines) == GridFlags::None)
        {
            const auto endY = offsetY + cHeight;
            renderer.DrawVerticalLine(x, y, endY - 1, Cfg->Grid.Lines.Vertical.Normal, true);
        }
    }

    if ((flags & GridFlags::HideHorizontalLines) == GridFlags::None)
    {
        renderer.DrawHorizontalLine(
              offsetX, offsetY - cHeight, cWidth * columnsNo + offsetX, Cfg->Grid.Lines.Vertical.Normal, true);
    }

    if ((flags & GridFlags::HideBoxes) == GridFlags::None)
    {
        for (auto i = 0U; i <= columnsNo; i++)
        {
            const auto x = offsetX + i * cWidth;
            const auto y = offsetY - cHeight;

            if (i == 0)
            {
                renderer.WriteSpecialCharacter(
                      x, y, SpecialChars::BoxTopLeftCornerSingleLine, Cfg->Grid.Lines.Vertical.Normal);
            }
            else if (i == columnsNo)
            {
                renderer.WriteSpecialCharacter(
                      x, y, SpecialChars::BoxTopRightCornerSingleLine, Cfg->Grid.Lines.Vertical.Normal);
            }
            else
            {
                renderer.WriteSpecialCharacter(x, y, SpecialChars::BoxMidleTop, Cfg->Grid.Lines.Vertical.Normal);
            }
        }
    }

    WriteTextParams wtp;
    wtp.Flags = WriteTextFlags::MultipleLines | WriteTextFlags::ClipToWidth | WriteTextFlags::FitTextToWidth;
    wtp.Color = Cfg->Grid.Text.Normal;

    auto it = headers.begin();
    for (auto i = 0U; i <= columnsNo && it != headers.end(); i++)
    {
        wtp.X     = offsetX + i * cWidth + 1; // 1 -> line
        wtp.Y     = offsetY - cHeight / 2 - 1;
        wtp.Width = cWidth - 1; // 1 -> line
        wtp.Align = it->ta;

        renderer.WriteText(it->content, wtp);

        std::advance(it, 1);
    }

    return true;
}

void GridControlContext::UpdateGridParameters(bool dontRecomputeDimensions)
{
    // define cell dimensions
    if (dontRecomputeDimensions == false || cWidth == 0 || cHeight == 0)
    {
        cWidth  = static_cast<uint32>(Layout.Width / columnsNo);
        cHeight = static_cast<uint32>(Layout.Height / rowsNo);
    }

    if (((flags & GridFlags::DisableMove) != GridFlags::DisableMove) && startedMoving == false)
    {
        CenterMatrix();
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

void GridControlContext::CenterMatrix()
{
    // center matrix
    offsetX = static_cast<uint32>((Layout.Width - cWidth * columnsNo) / 2);

    auto deltaHeight = (Layout.Height - cHeight * rowsNo);
    if ((flags & GridFlags::HideHeader) == GridFlags::None)
    {
        if (deltaHeight > cHeight)
        {
            deltaHeight -= cHeight;
        }
        else
        {
            deltaHeight = 0;
        }
    }

    offsetY = static_cast<uint32>(deltaHeight / 2);

    if ((flags & GridFlags::HideHeader) == GridFlags::None)
    {
        offsetY += cHeight;
    }
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

    return true;
}

bool GridControlContext::ToggleBooleanCell()
{
    if (selectedCellsIndexes.size() != 1)
    {
        return false;
    }

    const auto index    = selectedCellsIndexes[0];
    auto& cellData      = cells[index];
    const auto cellType = cellData.ct;
    if (cellType != Grid::CellType::Boolean)
    {
        return false;
    }

    auto& content = cellData.content;
    if (std::holds_alternative<bool>(content) == false)
    {
        return false;
    }

    const auto value = std::get<bool>(content);
    content          = !value;

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
                switch (data.ct)
                {
                case Grid::CellType::Boolean:
                    if (std::holds_alternative<bool>(data.content))
                    {
                        const auto value = std::get<bool>(data.content);
                        if (value)
                        {
                            cs = "True";
                        }
                        else
                        {
                            cs = "False";
                        }
                    }
                    break;
                case Grid::CellType::String:
                    if (std::holds_alternative<std::u16string>(data.content))
                    {
                        const auto& value = std::get<std::u16string>(data.content);
                        cs                = value;
                    }
                    break;
                default:
                    break;
                }
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
        const auto delta = selectedCellsIndexes.size() - tokens.size() + 1;
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
        auto& data = cells.at(*index);

        switch (data.ct)
        {
        case Grid::CellType::Boolean:
            if (std::holds_alternative<bool>(data.content))
            {
                data.content = (token.compare(u"True") == 0);
            }
            break;
        case Grid::CellType::String:
            if (std::holds_alternative<std::u16string>(data.content))
            {
                data.content = token;
            }
            break;
        default:
            break;
        }

        std::advance(index, 1);
    }

    return true;
}
} // namespace AppCUI
