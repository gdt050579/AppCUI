#include "ControlContext.hpp"

using namespace AppCUI::Input;
using namespace AppCUI::Graphics;

constexpr auto InvalidCellIndex = 0xFFFFFFFFU;

constexpr auto MenuCommandMergeCells = 0x01U;

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

    UpdateGridParameters();

    context->rightClickMenu.AddCommandItem("&Merge Cells", MenuCommandMergeCells, Key::F2);
    context->headerValues.reserve(context->columnsNo);
}

void Grid::Paint(Renderer& renderer)
{
    UpdateGridParameters();

    auto context = reinterpret_cast<GridControlContext*>(Context);

    if ((context->flags & GridFlags::HideHeader) == GridFlags::None)
    {
        DrawHeader(renderer);
    }

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

    for (auto const& [key, val] : context->data)
    {
        DrawCellContent(renderer, key);
    }
}

bool AppCUI::Controls::Grid::OnKeyEvent(AppCUI::Input::Key keyCode, char16_t UnicodeChar)
{
    auto context = reinterpret_cast<GridControlContext*>(Context);

    switch (keyCode)
    {
    case AppCUI::Input::Key::Space:
        if (context->selectedCellsIndexes.size() != 1)
        {
            break;
        }
        {
            const auto index    = context->selectedCellsIndexes[0];
            auto& cellData      = context->data[index];
            const auto cellType = cellData.first;
            if (cellType != Grid::CellType::Boolean)
            {
                break;
            }

            auto& content = cellData.second;
            if (std::holds_alternative<bool>(content) == false)
            {
                break;
            }

            const auto value = std::get<bool>(content);
            content          = !value;
            return true;
        }
        break;
    case AppCUI::Input::Key::Left:
    case AppCUI::Input::Key::Right:
    case AppCUI::Input::Key::Up:
    case AppCUI::Input::Key::Down:
        if (context->selectedCellsIndexes.size() == 0)
        {
            context->anchorCellIndex = 0;
            context->selectedCellsIndexes.emplace_back(0);
            return true;
        }

        if (context->selectedCellsIndexes.size() == 1)
        {
            const auto index = context->selectedCellsIndexes[0];
            auto columnIndex = index % context->columnsNo;
            auto rowIndex    = index / context->columnsNo;

            if (columnIndex > 0)
            {
                columnIndex -= (keyCode == AppCUI::Input::Key::Left);
            }
            if (columnIndex < context->columnsNo - 1)
            {
                columnIndex += (keyCode == AppCUI::Input::Key::Right);
            }

            if (rowIndex > 0)
            {
                rowIndex -= (keyCode == AppCUI::Input::Key::Up);
            }
            if (rowIndex < context->rowsNo - 1)
            {
                rowIndex += (keyCode == AppCUI::Input::Key::Down);
            }

            const auto newCellIndex = context->columnsNo * rowIndex + columnIndex;
            if (newCellIndex != index)
            {
                context->selectedCellsIndexes[0] = newCellIndex;
                return true;
            }
        }
        break;
    case AppCUI::Input::Key::Shift | AppCUI::Input::Key::Left:
    case AppCUI::Input::Key::Shift | AppCUI::Input::Key::Right:
    case AppCUI::Input::Key::Shift | AppCUI::Input::Key::Up:
    case AppCUI::Input::Key::Shift | AppCUI::Input::Key::Down:
        if (context->selectedCellsIndexes.size() == 0)
        {
            context->anchorCellIndex = 0;
            context->selectedCellsIndexes.emplace_back(0);
            return true;
        }

        {
            const auto anchorColumnIndex = context->anchorCellIndex % context->columnsNo;
            const auto anchorRowIndex    = context->anchorCellIndex / context->columnsNo;

            auto xLeft  = anchorColumnIndex;
            auto xRight = anchorColumnIndex;

            auto yTop = anchorRowIndex;
            auto yBot = anchorRowIndex;

            for (const auto& i : context->selectedCellsIndexes)
            {
                const auto colIndex = i % context->columnsNo;
                const auto rowIndex = i / context->columnsNo;

                xLeft  = std::min<>(xLeft, colIndex);
                xRight = std::max<>(xRight, colIndex);

                yTop = std::min<>(yTop, rowIndex);
                yBot = std::max<>(yBot, rowIndex);
            }

            const auto topLeft     = context->columnsNo * yTop + xLeft;
            const auto topRight    = context->columnsNo * yTop + xRight;
            const auto bottomLeft  = context->columnsNo * yBot + xLeft;
            const auto bottomRight = context->columnsNo * yBot + xRight;

            if (context->selectedCellsIndexes.size() == 1)
            {
                switch (keyCode)
                {
                case AppCUI::Input::Key::Shift | AppCUI::Input::Key::Left:
                    if (xLeft > 0)
                        xLeft -= 1;
                    break;
                case AppCUI::Input::Key::Shift | AppCUI::Input::Key::Right:
                    if (xRight < context->columnsNo - 1)
                        xRight += 1;
                    break;
                case AppCUI::Input::Key::Shift | AppCUI::Input::Key::Up:
                    if (yBot > 0)
                        yBot -= 1;
                    break;
                case AppCUI::Input::Key::Shift | AppCUI::Input::Key::Down:
                    if (yBot < context->rowsNo - 1)
                        yBot += 1;
                    break;
                default:
                    break;
                }
            }
            else if (topLeft == context->anchorCellIndex)
            {
                switch (keyCode)
                {
                case AppCUI::Input::Key::Shift | AppCUI::Input::Key::Left:
                    if (xRight > 0)
                        xRight -= 1;
                    break;
                case AppCUI::Input::Key::Shift | AppCUI::Input::Key::Right:
                    if (xRight < context->columnsNo - 1)
                        xRight += 1;
                    break;
                case AppCUI::Input::Key::Shift | AppCUI::Input::Key::Up:
                    if (yBot > 0)
                        yBot -= 1;
                    break;
                case AppCUI::Input::Key::Shift | AppCUI::Input::Key::Down:
                    if (yBot < context->rowsNo - 1)
                        yBot += 1;
                    break;
                default:
                    break;
                }
            }
            else if (topRight == context->anchorCellIndex)
            {
                switch (keyCode)
                {
                case AppCUI::Input::Key::Shift | AppCUI::Input::Key::Left:
                    if (xLeft > 0)
                        xLeft -= 1;
                    break;
                case AppCUI::Input::Key::Shift | AppCUI::Input::Key::Right:
                    if (xLeft < context->columnsNo - 1)
                        xLeft += 1;
                    break;
                case AppCUI::Input::Key::Shift | AppCUI::Input::Key::Up:
                    if (yBot > 0)
                        yBot -= 1;
                    break;
                case AppCUI::Input::Key::Shift | AppCUI::Input::Key::Down:
                    if (yBot < context->rowsNo - 1)
                        yBot += 1;
                    break;
                default:
                    break;
                }
            }
            else if (bottomLeft == context->anchorCellIndex)
            {
                switch (keyCode)
                {
                case AppCUI::Input::Key::Shift | AppCUI::Input::Key::Left:
                    if (xRight > 0)
                        xRight -= 1;
                    break;
                case AppCUI::Input::Key::Shift | AppCUI::Input::Key::Right:
                    if (xRight < context->columnsNo - 1)
                        xRight += 1;
                    break;
                case AppCUI::Input::Key::Shift | AppCUI::Input::Key::Up:
                    if (yTop > 0)
                        yTop -= 1;
                    break;
                case AppCUI::Input::Key::Shift | AppCUI::Input::Key::Down:
                    if (yTop < context->rowsNo - 1)
                        yTop += 1;
                    break;
                default:
                    break;
                }
            }
            else if (bottomRight == context->anchorCellIndex)
            {
                switch (keyCode)
                {
                case AppCUI::Input::Key::Shift | AppCUI::Input::Key::Left:
                    if (xLeft > 0)
                        xLeft -= 1;
                    break;
                case AppCUI::Input::Key::Shift | AppCUI::Input::Key::Right:
                    if (xLeft < context->columnsNo - 1)
                        xLeft += 1;
                    break;
                case AppCUI::Input::Key::Shift | AppCUI::Input::Key::Up:
                    if (yTop > 0)
                        yTop -= 1;
                    break;
                case AppCUI::Input::Key::Shift | AppCUI::Input::Key::Down:
                    if (yTop < context->rowsNo - 1)
                        yTop += 1;
                    break;
                default:
                    break;
                }
            }

            context->selectedCellsIndexes.clear();
            for (auto i = std::min<>(xLeft, xRight); i <= std::max<>(xLeft, xRight); i++)
            {
                for (auto j = std::min<>(yBot, yTop); j <= std::max<>(yBot, yTop); j++)
                {
                    const auto current = context->columnsNo * j + i;
                    context->selectedCellsIndexes.emplace_back(current);
                }
            }

            return true;
        }
        break;
    case AppCUI::Input::Key::Escape:
        if (context->selectedCellsIndexes.size() > 0)
        {
            context->selectedCellsIndexes.clear();
            return true;
        }
    case AppCUI::Input::Key::Ctrl | AppCUI::Input::Key::C:
    {
        auto xLeft  = 0xFFFFFFFFU;
        auto xRight = 0U;

        auto yTop = 0xFFFFFFFFU;
        auto yBot = 0U;

        for (const auto& i : context->selectedCellsIndexes)
        {
            const auto colIndex = i % context->columnsNo;
            const auto rowIndex = i / context->columnsNo;

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
                const auto current = context->columnsNo * j + i;
                const auto& it     = context->data.find(current);
                if (it != context->data.end())
                {
                    const auto& content = it->second;
                    switch (content.first)
                    {
                    case CellType::Boolean:
                        if (std::holds_alternative<bool>(content.second))
                        {
                            const auto value = std::get<bool>(content.second);
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
                    case CellType::String:
                        if (std::holds_alternative<std::u16string>(content.second))
                        {
                            const auto& value = std::get<std::u16string>(content.second);
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
                    lusb.Add(context->separator);
                }
            }
            lusb.Add("\n");
        }

        if (AppCUI::OS::Clipboard::SetText(lusb) == false)
        {
            const std::string input{ lusb };
            LOG_WARNING("Fail to copy string [%s] to the clipboard!", input.c_str());
        }
    }
    break;

    case AppCUI::Input::Key::Ctrl | AppCUI::Input::Key::V:
    {
        // seems slow - a lower level parser might be better - we'll see
        LocalUnicodeStringBuilder<2048> lusb{};
        AppCUI::OS::Clipboard::GetText(lusb);

        const std::u16string input{ lusb };

        size_t last = 0;
        size_t next = 0;
        std::vector<std::u16string> lines;
        lines.reserve(50);
        while ((next = input.find(u"\n", last)) != std::string::npos)
        {
            lines.emplace_back(input.substr(last, next - last));
            last = next + context->separator.length();
        }
        const auto lastLine = input.substr(last);
        if (lastLine != u"")
        {
            lines.emplace_back(lastLine);
        }

        std::vector<std::u16string> tokens;
        tokens.reserve(50);
        for (const auto& line : lines)
        {
            size_t last = 0;
            size_t next = 0;
            while ((next = line.find(context->separator, last)) != std::string::npos)
            {
                tokens.emplace_back(line.substr(last, next - last));
                last = next + context->separator.length();
            }
            tokens.emplace_back(line.substr(last));
        }

        if (tokens.size() > context->selectedCellsIndexes.size())
        {
            const auto delta = context->selectedCellsIndexes.size() - tokens.size() + 1;
            const auto start = tokens.begin() + context->selectedCellsIndexes.size() - 1U;

            LocalUnicodeStringBuilder<2048> lusbLastToken;
            for (std::vector<std::u16string>::iterator i = start; i != tokens.end(); i++)
            {
                lusbLastToken.Add(*i);
            }

            tokens.erase(start, tokens.end());

            std::u16string lastToken{ lusbLastToken };
            tokens.emplace_back(lastToken);
        }

        auto index = context->selectedCellsIndexes.begin();
        for (const auto& token : tokens)
        {
            auto& content = context->data.at(*index);

            switch (content.first)
            {
            case CellType::Boolean:
                if (std::holds_alternative<bool>(content.second))
                {
                    content.second = (token.compare(u"True") == 0);
                }
                break;
            case CellType::String:
                if (std::holds_alternative<std::u16string>(content.second))
                {
                    content.second = token;
                }
                break;
            default:
                break;
            }

            std::advance(index, 1);
        }
    }
    break;
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
    case MouseButton::Left | MouseButton::DoubleClicked:
    {
        const auto index = ComputeCellNumber(x, y);
        if (index == InvalidCellIndex)
        {
            break;
        }

        auto& cellData      = context->data[index];
        const auto cellType = cellData.first;
        if (cellType != Grid::CellType::Boolean)
        {
            break;
        }

        auto& content = cellData.second;
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

    const auto drawLines = [&](unsigned int cellIndex, CellStatus cellType)
    {
        ColorPair vertical   = context->Cfg->Grid.Lines.Vertical.Normal;
        ColorPair horizontal = context->Cfg->Grid.Lines.Horizontal.Normal;

        switch (cellType)
        {
        case AppCUI::Controls::Grid::CellStatus::Normal:
            vertical   = context->Cfg->Grid.Lines.Vertical.Normal;
            horizontal = context->Cfg->Grid.Lines.Horizontal.Normal;
            break;
        case AppCUI::Controls::Grid::CellStatus::Selected:
            vertical   = context->Cfg->Grid.Lines.Vertical.Selected;
            horizontal = context->Cfg->Grid.Lines.Horizontal.Selected;
            break;
        case AppCUI::Controls::Grid::CellStatus::Hovered:
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
        drawLines(context->hoveredCellIndex, CellStatus::Hovered);
    }

    if (context->selectedCellsIndexes.size() > 0 && ((context->flags & GridFlags::HideSelectedCell) == GridFlags::None))
    {
        for (const auto& cellIndex : context->selectedCellsIndexes)
        {
            drawLines(cellIndex, CellStatus::Selected);
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
            DrawCellBackground(renderer, CellStatus::Normal, i, j);
        }
    }

    if (context->hoveredCellIndex != InvalidCellIndex &&
        ((context->flags & GridFlags::HideHoveredCell) == GridFlags::None))
    {
        DrawCellBackground(renderer, CellStatus::Hovered, context->hoveredCellIndex);
    }

    if (context->selectedCellsIndexes.size() > 0 && ((context->flags & GridFlags::HideSelectedCell) == GridFlags::None))
    {
        for (const auto& cellIndex : context->selectedCellsIndexes)
        {
            DrawCellBackground(renderer, CellStatus::Selected, cellIndex);
        }
    }
}

void AppCUI::Controls::Grid::DrawCellBackground(
      Graphics::Renderer& renderer, CellStatus cellType, unsigned int i, unsigned int j)
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);

    const auto xLeft  = context->offsetX + i * context->cWidth;
    const auto xRight = context->offsetX + (i + 1) * context->cWidth;

    const auto yTop    = context->offsetY + j * context->cHeight;
    const auto yBottom = context->offsetY + (j + 1) * context->cHeight;

    ColorPair color = context->Cfg->Grid.Background.Cell.Normal;
    switch (cellType)
    {
    case AppCUI::Controls::Grid::CellStatus::Normal:
        color = context->Cfg->Grid.Background.Cell.Normal;
        break;
    case AppCUI::Controls::Grid::CellStatus::Selected:
        color = context->Cfg->Grid.Background.Cell.Selected;
        break;
    case AppCUI::Controls::Grid::CellStatus::Hovered:
        color = context->Cfg->Grid.Background.Cell.Hovered;
        break;
    default:
        color = context->Cfg->Grid.Background.Cell.Normal;
        break;
    }

    renderer.FillRect(xLeft + 1, yTop + 1, xRight - 1, yBottom - 1, ' ', color);
}

void AppCUI::Controls::Grid::DrawCellBackground(
      Graphics::Renderer& renderer, CellStatus cellType, unsigned int cellIndex)
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);

    const auto columnIndex = cellIndex % context->columnsNo;
    const auto rowIndex    = cellIndex / context->columnsNo;

    DrawCellBackground(renderer, cellType, columnIndex, rowIndex);
}

bool AppCUI::Controls::Grid::DrawCellContent(Graphics::Renderer& renderer, unsigned int cellIndex)
{
    auto context = reinterpret_cast<GridControlContext*>(Context);

    const auto cellColumn = cellIndex % context->columnsNo;
    const auto cellRow    = cellIndex / context->columnsNo;

    const auto x = context->offsetX + cellColumn * context->cWidth + 1; // + 1 -> line
    const auto y = context->offsetY + cellRow * context->cHeight + 1;   // + 1 -> line

    const auto& content = context->data[cellIndex];

    switch (content.first)
    {
    case CellType::Boolean:
        if (std::holds_alternative<bool>(content.second))
        {
            const auto value = std::get<bool>(content.second);
            if (value)
            {
                renderer.WriteSingleLineText(x, y, "True", context->Cfg->Grid.Text.Normal);
            }
            else
            {
                renderer.WriteSingleLineText(x, y, "False", context->Cfg->Grid.Text.Normal);
            }
            return true;
        }
        return false;
    case CellType::String:
        if (std::holds_alternative<std::u16string>(content.second))
        {
            const auto& value = std::get<std::u16string>(content.second);
            renderer.WriteSingleLineText(x, y, value, context->Cfg->Grid.Text.Normal);
            return true;
        }
        return false;
    default:
        break;
    }

    return false;
}

bool AppCUI::Controls::Grid::DrawHeader(Graphics::Renderer& renderer)
{
    auto context = reinterpret_cast<GridControlContext*>(Context);

    renderer.FillRect(
          context->offsetX,
          context->offsetY - context->headerSize,
          context->cWidth * context->columnsNo + context->offsetX,
          context->offsetY - context->headerSize,
          ' ',
          context->Cfg->Grid.Header);

    for (auto i = 0U; i <= context->columnsNo; i++)
    {
        const auto x = context->offsetX + i * context->cWidth;
        const auto y = context->offsetY + 0 * context->cHeight - context->headerSize;

        if ((context->flags & GridFlags::HideVerticalLines) == GridFlags::None)
        {
            const auto endY = context->offsetY + context->headerSize;
            renderer.DrawVerticalLine(x, y, endY - 1, context->Cfg->Grid.Lines.Vertical.Normal, true);
        }
    }

    auto it = context->headerValues.begin();
    for (auto i = 0U; i <= context->columnsNo; i++)
    {
        if (it == context->headerValues.end())
        {
            break;
        }

        const auto x = context->offsetX + i * context->cWidth;
        const auto y = context->offsetY + 0 * context->cHeight - context->headerSize;

        if ((context->flags & GridFlags::HideVerticalLines) == GridFlags::None)
        {
            const auto endY = context->offsetY + context->headerSize;
            renderer.WriteSingleLineText(x + 1, y, *it, context->Cfg->Grid.Text.Normal);
        }

        std::advance(it, 1);
    }

    return true;
}

void AppCUI::Controls::Grid::UpdateGridParameters()
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);

    // define cell dimensions
    context->cWidth  = static_cast<unsigned int>(context->Layout.Width / context->columnsNo);
    context->cHeight = static_cast<unsigned int>(context->Layout.Height / context->rowsNo);

    // center matrix
    context->offsetX = static_cast<unsigned int>((context->Layout.Width - context->cWidth * context->columnsNo) / 2);

    auto deltaHeight = (context->Layout.Height - context->cHeight * context->rowsNo);
    if ((context->flags & GridFlags::HideHeader) == GridFlags::None)
    {
        deltaHeight -= context->headerSize;
    }
    context->offsetY = static_cast<unsigned int>(deltaHeight / 2);

    if ((context->flags & GridFlags::HideHeader) == GridFlags::None)
    {
        context->offsetY += context->headerSize;
    }

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
    context->selectedCellsIndexes.erase(
          std::unique(context->selectedCellsIndexes.begin(), context->selectedCellsIndexes.end()),
          context->selectedCellsIndexes.end());
}

unsigned int AppCUI::Controls::Grid::GetCellsCount() const
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);
    return context->columnsNo * context->rowsNo;
}

std::pair<unsigned int, unsigned int> AppCUI::Controls::Grid::GetGridDimensions() const
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);
    return { context->columnsNo, context->rowsNo };
}

bool AppCUI::Controls::Grid::UpdateCell(
      unsigned int index, const std::pair<CellType, std::variant<bool, ConstString>>& data)
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);
    CHECK(index < context->columnsNo * context->rowsNo, false, "");

    std::variant<bool, std::u16string> cellData;
    if (std::holds_alternative<bool>(data.second))
    {
        cellData = std::get<bool>(data.second);
    }
    else if (std::holds_alternative<ConstString>(data.second))
    {
        const auto& cs = std::get<ConstString>(data.second);
        Utils::UnicodeStringBuilder usb{ cs };
        std::u16string u16s(usb);
        cellData = u16s;
    }

    context->data[index] = { data.first, cellData };

    return true;
}

bool AppCUI::Controls::Grid::UpdateCells(
      const std::map<unsigned int, const std::pair<CellType, std::variant<bool, ConstString>>>& data)
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);

    // transaction so check all bounds/indexes first
    for (auto const& [key, val] : data)
    {
        CHECK(key < context->columnsNo * context->rowsNo, false, "");
    }

    // start making actual update
    for (auto const& [key, val] : data)
    {
        UpdateCell(key, val);
    }

    return true;
}

const ConstString AppCUI::Controls::Grid::GetSeparator() const
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);
    return context->separator;
}

void AppCUI::Controls::Grid::SetSeparator(ConstString separator)
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);
    Utils::UnicodeStringBuilder usb{ separator };
    context->separator = usb;
}

bool AppCUI::Controls::Grid::UpdateHeaderValues(const std::vector<ConstString>& headerValues)
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);

    context->headerValues.clear();
    for (const auto& value : headerValues)
    {
        LocalUnicodeStringBuilder<1024> lusb{ value };
        context->headerValues.emplace_back(lusb);
    }

    return true;
}
