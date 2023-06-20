#include <algorithm>
#include "ControlContext.hpp"

#ifdef MessageBox
#    undef MessageBox
#endif

#define START(offset, dim) (std::max<>(0, -offset - (int) dim) / (int) dim)
#define END(layoutDim, offset, dim, maxSize)                                                                           \
    (std::min<>((int) maxSize, (layoutDim - offset + (int) dim - 1) / (int) dim))

#define START_DRAW_REL(offset, delta, dim) std::max<>(0, offset + (int) delta - (int) dim + 1)
#define END_DRAW_REL(start, dim)           start + (int) dim - 2

#define END_DRAW(offset, no, dim)               offset + (int) no*(int) dim - 1
#define START_INDEX_DRAW(startRel, offset, dim) (startRel - offset - 2 + 2 * dim) / (int) dim

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
    context->columnsFilter.insert(context->columnsFilter.end(), columnsNo, u"");

    context->ResetMatrixPosition();
    context->UpdateGridParameters();

    context->headers.reserve(context->columnsNo);
    context->SetDefaultHeaderValues();
    context->ReserveMap();

    Application::Config* cfg = Application::GetAppConfig();

    Sort();
    Filter();
}

void Grid::Paint(Renderer& renderer)
{
    auto context = reinterpret_cast<GridControlContext*>(Context);
    context->UpdateGridParameters(true);

    renderer.Clear(' ');
    renderer.HideCursor();

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

    int32 toAddColumn = std::max<>(0, context->deltaX / (int) context->cWidth);
    int32 toAddRow    = std::max<>(0, context->deltaY / (int) context->cHeight);

    for (auto columnIndex = toAddColumn;
         columnIndex < END(context->Layout.Width, context->offsetX, context->cWidth, context->columnsNo);
         ++columnIndex)
    {
        for (auto rowIndex = toAddRow;
             rowIndex < END(context->Layout.Height, context->offsetY, context->cHeight, context->rowsNo);
             ++rowIndex)
        {
            const auto cellIndex = rowIndex * context->columnsNo + columnIndex;
            context->DrawCellContent(renderer, cellIndex);
        }
    }

    context->DrawIndexesColumn(renderer);
    context->DrawHeader(renderer);
    context->DrawHeaderForIndexesColumn(renderer);
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
            context->anchorCellIndex  = InvalidCellIndex;
            context->hoveredCellIndex = InvalidCellIndex;

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
            if (START_DRAW_REL(context->offsetY, context->deltaY, context->cHeight) <
                END_DRAW(context->offsetY, 0, context->cHeight) - (int) context->cHeight + 3)
            {
                break;
            }

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
            if (START_DRAW_REL(context->offsetX, context->deltaX, context->cWidth) 
                < END_DRAW(context->offsetX, 0, context->cWidth) - (int) context->cWidth + 3)
            {
                context->UpdatePositions(-1, 0);
                break;
            }

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

bool Controls::Grid::OnMouseWheel(int /*x*/, int /*y*/, Input::MouseWheel direction)
{
    auto context = reinterpret_cast<GridControlContext*>(Context);

    switch (direction)
    {
    case AppCUI::Input::MouseWheel::None:
        break;
    case AppCUI::Input::MouseWheel::Up:
        if ((context->flags & GridFlags::DisableZoom) == GridFlags::None)
        {
            context->startedMoving    = true;
            context->startedScrolling = true;
            context->UpdateDimensions(1, 1);
            context->startedScrolling = false;
            return true;
        }
        break;
    case AppCUI::Input::MouseWheel::Down:
        if ((context->flags & GridFlags::DisableZoom) == GridFlags::None)
        {
            context->startedMoving    = true;
            context->startedScrolling = true;
            context->UpdateDimensions(-1, -1);
            context->startedScrolling = false;
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

bool Grid::OnEvent(Controls::Reference<Control>, Event eventType, int /*controlID*/)
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
    context->ReserveMap();

    context->columnsFilter.clear();
    context->columnsFilter.reserve(context->columnsNo);
    context->columnsFilter.insert(context->columnsFilter.end(), context->columnsNo, u"");
}

void Controls::Grid::SetFilterOnCurrentColumn(const std::u16string& filter)
{
    const auto context        = reinterpret_cast<GridControlContext*>(Context);
    context->shouldPaintError = true;
    uint32 columnIndex;
    if (context->anchorCellIndex != InvalidCellIndex)
    {
        columnIndex = context->anchorCellIndex % context->columnsNo;
    }
    else if (context->hoveredCellIndex != InvalidCellIndex)
    {
        columnIndex = context->hoveredCellIndex % context->columnsNo;
    }
    else
    {
        return;
    }

    context->columnsFilter[columnIndex] = filter;
}

Size Grid::GetGridDimensions() const
{
    const auto context = reinterpret_cast<GridControlContext*>(Context);
    return { context->columnsNo, context->rowsNo };
}

bool Grid::UpdateCell(uint32 index, ConstString content, TextAlignament textAlignment, bool sort)
{
    const auto context    = reinterpret_cast<GridControlContext*>(Context);
    const auto cellColumn = index % context->columnsNo;
    const auto cellRow    = index / context->columnsNo;
    CHECK(index < context->columnsNo * context->rowsNo, false, "");

    Utils::UnicodeStringBuilder usb{ content };
    std::u16string u16s(usb);
    (*context->cells)[cellColumn][cellRow] = { textAlignment, u16s };

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
    context->headers.push_back({ TextAlignament::Center, LocalUnicodeStringBuilder<1024>("Index") });

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
    AppCUI::Application::GetApplication()->RepaintStatus = REPAINT_STATUS_DRAW;
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
    AppCUI::Application::GetApplication()->RepaintStatus = REPAINT_STATUS_DRAW;
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

void Controls::Grid::Filter()
{
    auto context = reinterpret_cast<GridControlContext*>(Context);
    if ((context->flags & GridFlags::Filter) != GridFlags::None)
    {
        uint32 columnIndex;
        if (context->anchorCellIndex != InvalidCellIndex)
        {
            columnIndex               = context->anchorCellIndex % context->columnsNo;
            context->anchorCellIndex  = InvalidCellIndex;
            context->hoveredCellIndex = InvalidCellIndex;
        }
        else if (context->hoveredCellIndex != InvalidCellIndex)
        {
            columnIndex               = context->hoveredCellIndex % context->columnsNo;
            context->hoveredCellIndex = InvalidCellIndex;
        }
        else
        {
            if (context->shouldPaintError)
            {
                AppCUI::Dialogs::MessageBox::ShowError("Error!", "No cell selected!");
                context->shouldPaintError = false;
            }
            return;
        }
        context->shouldPaintError = false;
        context->FilterColumn(columnIndex);
    }
}

optional<AppCUI::Utils::String> Grid::GetSelectedCellContent()
{
    auto context = reinterpret_cast<GridControlContext*>(Context);
    uint32 cellIndex;
    if (context->anchorCellIndex != InvalidCellIndex)
    {
        cellIndex = context->anchorCellIndex;
    }
    else if (context->hoveredCellIndex != InvalidCellIndex)
    {
        cellIndex = context->hoveredCellIndex;
    }
    else
    {
        return {};
    }

    const auto cellColumn = cellIndex % context->columnsNo;
    const auto cellRow    = cellIndex / context->columnsNo;
    std::u16string_view u16strView((*context->cells)[cellColumn][cellRow].content);

    std::string contentAsString = { u16strView.begin(), u16strView.end() };

    AppCUI::Utils::String cellContent(contentAsString);
    return cellContent;
}

void GridControlContext::FilterColumn(int columnIndex)
{
    std::vector<uint32> filteredRows;
    filteredRows.reserve(rowsNo);

    ProgressStatus::Init("Searching...", rowsNo);
    LocalString<512> ls;
    const char* format = "Reading [%d/%d] rows...";
    for (auto rowIndex = 0; rowIndex < rowsNo; rowIndex++)
    {
        ProgressStatus::Update(rowIndex, ls.Format(format, rowIndex, rowsNo));
        if ((*cells)[columnIndex][rowIndex].content.find(columnsFilter[columnIndex]) != std::u16string::npos)
        {
            filteredRows.push_back(rowIndex);
        }
    }

    if (filteredRows.empty())
    {
        AppCUI::Dialogs::MessageBox::ShowError("Error!", "No results found!");
        return;
    }

    auto newCells = std::vector<std::vector<GridCellData>>();
    auto tmpRow   = std::vector<GridCellData>(filteredRows.size());
    newCells.reserve(columnsNo);

    for (auto columnIndex = 0; columnIndex < columnsNo; columnIndex++)
    {
        for (auto filteredIndex = 0; filteredIndex < filteredRows.size(); filteredIndex++)
        {
            tmpRow[filteredIndex] = (*cells)[columnIndex][filteredRows[filteredIndex]];
        }
        newCells.push_back(tmpRow);
    }

    *cells = newCells;
    rowsNo = filteredRows.size();
}

void GridControlContext::DrawBoxes(Renderer& renderer)
{
    if (rowsNo == 0)
    {
        return;
    }

    const auto color = Cfg->Lines.GetColor(GetControlState(ControlStateFlags::All));
    if ((flags & GridFlags::HideHorizontalLines) == GridFlags::None &&
        (flags & GridFlags::HideVerticalLines) == GridFlags::None)
    {
        const int toAddColumn = std::max<>(0, deltaX / (int) cWidth);
        const int toAddRow    = std::max<>(0, deltaY / (int) cHeight);

        for (auto i = toAddColumn; i < END(Layout.Width, offsetX, cWidth, columnsNo) + 1; i++)
        {
            const auto x = offsetX + i * cWidth;
            for (auto j = toAddRow; j < END(Layout.Height, offsetY, cHeight, rowsNo) + 1; j++)
            {
                const auto y = offsetY + j * cHeight;

                const auto sc = ComputeBoxType(i, j, 0U, 0U, columnsNo, rowsNo);
                renderer.WriteSpecialCharacter(x, y, sc, color);
            }
        }
    }
    else if ((flags & GridFlags::HideVerticalLines) == GridFlags::None)
    {
        const auto y1 = offsetY + deltaY + 1;
        const auto y2 = offsetY + deltaY + rowsNo * cHeight - 1;
        for (auto i = 0U; i <= columnsNo; i++)
        {
            const auto x = offsetX + deltaX + i * cWidth;
            renderer.DrawVerticalLine(x, y1, y2, color);
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

        const auto color = Cfg->Cursor.Inactive;

        renderer.WriteSpecialCharacter(xLeft, yTop, SpecialChars::BoxTopLeftCornerSingleLine, color);
        renderer.WriteSpecialCharacter(xRight, yTop, SpecialChars::BoxTopRightCornerSingleLine, color);
        renderer.WriteSpecialCharacter(xLeft, yBottom, SpecialChars::BoxBottomLeftCornerSingleLine, color);
        renderer.WriteSpecialCharacter(xRight, yBottom, SpecialChars::BoxBottomRightCornerSingleLine, color);

        renderer.DrawVerticalLine(xLeft, yTop + 1, yBottom - 1, color);
        renderer.DrawVerticalLine(xRight, yTop + 1, yBottom - 1, color);
    }

    if (selectedCellsIndexes.size() > 0 && ((flags & GridFlags::HideSelectedCell) == GridFlags::None))
    {
        const auto color = Cfg->Cursor.Inactive;

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
            renderer.DrawVerticalLine(x, y1, y2, color);
        }

        for (auto i = sci; i <= eci; i++)
        {
            const auto x = offsetX + i * cWidth;
            for (auto j = sri; j <= eri; j++)
            {
                const auto y  = offsetY + j * cHeight;
                const auto sc = ComputeBoxType(i, j, sci, sri, eci, eri);
                renderer.WriteSpecialCharacter(x, y, sc, color);
            }
        }
    }
}

void GridControlContext::DrawLines(Renderer& renderer)
{
    if (rowsNo == 0)
    {
        return;
    }

    const int toAddColumn = std::max<>(0, deltaX / (int) cWidth);
    const int toAddRow    = std::max<>(0, deltaY / (int) cHeight);

    const auto color = Cfg->Lines.GetColor(GetControlState(ControlStateFlags::All));
    for (auto i = toAddColumn; i < END(Layout.Width, offsetX, cWidth, columnsNo) + 1; i++)
    {
        const auto x = offsetX + i * cWidth;
        for (auto j = toAddRow; j < END(Layout.Height, offsetY, cHeight, rowsNo) + 1; j++)
        {
            const auto y = offsetY + j * cHeight;

            if ((flags & GridFlags::HideHorizontalLines) == GridFlags::None)
            {
                if (i < columnsNo)
                {
                    const auto endX = offsetX + (i + 1) * cWidth;
                    renderer.DrawHorizontalLine(x + 1, y, endX - 1, color, true);
                }
            }

            if ((flags & GridFlags::HideVerticalLines) == GridFlags::None)
            {
                if (j < rowsNo)
                {
                    const auto endY = offsetY + (j + 1) * cHeight;
                    renderer.DrawVerticalLine(x, y + 1, endY - 1, color, true);
                }
            }
        }
    }

    const auto drawLines = [&](uint32 cellIndex, GridCellStatus cellType)
    {
        ColorPair vertical, horizontal;
        horizontal = vertical = Cfg->Cursor.Inactive;

        /*if (cellType == GridCellStatus::Duplicate)
        {
            vertical = horizontal = Cfg->Selection.SimilarText;
        }*/

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
    for (auto i = START(offsetX, cWidth); i < END(Layout.Width, offsetX, cWidth, columnsNo); i++)
    {
        for (auto j = START(offsetY, cHeight); j < END(Layout.Height, offsetY, cHeight, rowsNo); j++)
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

    int cellIndex   = j * columnsNo + i;
    ColorPair color = Cfg->Cursor.Inactive;
    if (cellIndex == anchorCellIndex)
    {
        color = Cfg->Cursor.OverInactiveItem;
 
    }
    
    for (auto& selectedCellIndex : selectedCellsIndexes)
    {
        if (selectedCellIndex == cellIndex)
        {
            color = Cfg->Cursor.OverInactiveItem;
        }
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

    const int x = offsetX + cellColumn * cWidth + 1; // + 1 -> line
    const int y = offsetY + cellRow * cHeight + 1;   // + 1 -> line

    const auto& data = (*cells)[cellColumn][cellRow];

    const auto state = GetComponentState(
          ControlStateFlags::All,
          cellIndex == hoveredCellIndex,
          std::find(selectedCellsIndexes.begin(), selectedCellsIndexes.end(), cellIndex) != selectedCellsIndexes.end());

    ColorPair color = Cfg->Text.Normal;

    switch (state)
    {
    case ControlState::Focused:
        color = Cfg->Text.Focused;
        break;
    case ControlState::Normal:
        color = Cfg->Text.Focused;
        break;
    case ControlState::Hovered:
        color = Cfg->Text.Focused;
        break;
    case ControlState::Inactive:
        color = Cfg->Text.Inactive;
        break;
    case ControlState::PressedOrSelected:
        color = this->Cfg->Cursor.Normal;
        break;
    default:
        color = Cfg->Text.Normal;
        break;
    }

    if (std::find(duplicatedCellsIndexes.begin(), duplicatedCellsIndexes.end(), cellIndex) !=
        duplicatedCellsIndexes.end() && state != ControlState::PressedOrSelected)
    {
        color = this->Cfg->Cursor.Normal;
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
    const auto augmentedColumnsNo = columnsNo + 1;

    const int32 top    = START_DRAW_REL(offsetY, deltaY, cHeight);
    const int32 bottom = END_DRAW_REL(top, cHeight);

    const int32 left  = START_DRAW_REL(offsetX, deltaX, cWidth);
    const int32 right = END_DRAW(offsetX, columnsNo, cWidth);

    renderer.FillRect(left, top, right, bottom, ' ', Cfg->Header.Text.Normal);

    const auto lineColor = Cfg->Lines.GetColor(GetControlState(ControlStateFlags::All));

    const int32 beginAdd = START_INDEX_DRAW(left, offsetX, cWidth);
    for (auto i = beginAdd + 1; i <= augmentedColumnsNo; i++)
    {
        const int x    = offsetX + (i - 1) * cWidth;
        const int y    = top;
        const int endY = top + cHeight;

        if ((flags & GridFlags::HideVerticalLines) == GridFlags::None)
        {
            renderer.DrawVerticalLine(x, y, endY - 1, lineColor, true);
        }
    }

    int firstVerticalLine = START_DRAW_REL(offsetX, deltaX, cWidth) + cWidth - 1;
    const int y           = top;
    const int endY        = top + cHeight;

    if ((flags & GridFlags::HideVerticalLines) == GridFlags::None)
    {
        renderer.DrawVerticalLine(firstVerticalLine, y, endY - 1, lineColor, true);
    }

    if ((flags & GridFlags::HideHorizontalLines) == GridFlags::None)
    {
        renderer.DrawHorizontalLine(left, top - 1, right + 1, lineColor, true);
        renderer.DrawHorizontalLine(left, bottom + 1, right + 1, lineColor, true);
    }

    if ((flags & GridFlags::HideBoxes) == GridFlags::None)
    {
        if ((flags & GridFlags::HideHorizontalLines) == GridFlags::None)
        {
            for (auto i = 0U; i <= augmentedColumnsNo; i++)
            {
                if (i == 0)
                {
                    i += beginAdd;
                }
                const int32 x = offsetX + i * cWidth - cWidth;
                const int32 y = offsetY - GetHeaderHeight();

                if (i == 0)
                {
                    renderer.WriteSpecialCharacter(x, y, SpecialChars::BoxTopLeftCornerSingleLine, lineColor);
                }
                else if (i == augmentedColumnsNo)
                {
                    renderer.WriteSpecialCharacter(x, y, SpecialChars::BoxTopRightCornerSingleLine, lineColor);
                }
                else
                {
                    renderer.WriteSpecialCharacter(x, y, SpecialChars::BoxMidleTop, lineColor);
                }
            }
        }
    }

    WriteTextParams wtp;
    wtp.Flags = WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth | WriteTextFlags::FitTextToWidth;
    wtp.Color = Cfg->Text.Normal;

    for (auto i = beginAdd + 1; i < headers.size(); i++)
    {
        const auto it = headers[i];
        wtp.X         = offsetX + (i - 1) * cWidth + 1; // 1 -> line
        wtp.Y         = top;
        wtp.Width     = cWidth - 1; // 1 -> line
        wtp.Align     = it.ta;

        renderer.WriteText(it.content, wtp);

        if ((flags & GridFlags::Sort) != GridFlags::None)
        {
            const int endX = wtp.X + cWidth - 1;
            renderer.WriteSpecialCharacter(
                  endX - 1,
                  wtp.Y,
                  columnsSort[i] ? SpecialChars::TriangleUp : SpecialChars::TriangleDown,
                  { Color::Black, Color::Transparent });
        }

        // draw header for index column, then skip those who should not be drawn
    }
    return true;
}

bool GridControlContext::DrawIndexesColumn(Graphics::Renderer& renderer)
{
    const int32 top    = START_DRAW_REL(offsetY, deltaY, cHeight);
    const int32 bottom = END_DRAW(offsetY, cHeight, rowsNo);

    const int32 left  = START_DRAW_REL(offsetX, deltaX, cWidth);
    const int32 right = END_DRAW_REL(left, cWidth);

    renderer.FillRect(left, top, right, bottom, ' ', Cfg->Text.Normal);
    const auto lineColor = Cfg->Lines.GetColor(GetControlState(ControlStateFlags::All));
    renderer.DrawVerticalLine(left - 1, top, bottom, lineColor);
    renderer.DrawVerticalLine(right + 1, top, bottom, lineColor);

    const int32 beginAdd = std::max<>(0, (int) deltaY / (int) cHeight - 1);
    for (auto i = beginAdd; i <= rowsNo; i++)
    {
        const int x    = left - 1;
        const int y    = offsetY + i * cHeight;
        const int endX = x + cWidth - 1;

        if ((flags & GridFlags::HideVerticalLines) == GridFlags::None)
        {
            renderer.DrawHorizontalLine(x, y, endX, lineColor, true);
        }

        if (i != rowsNo)
        {
            renderer.WriteSpecialCharacter(x, y, SpecialChars::BoxMidleLeft, lineColor);
        }
        else
        {
            renderer.WriteSpecialCharacter(x, y, SpecialChars::BoxBottomLeftCornerSingleLine, lineColor);
        }
    }

    WriteTextParams wtp;
    wtp.Flags = WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth | WriteTextFlags::FitTextToWidth;
    wtp.Color = Cfg->Text.Normal;
    wtp.X     = left;

    for (auto i = beginAdd; i < rowsNo; i++)
    {
        wtp.Y     = offsetY + i * cHeight + 1;
        wtp.Width = cWidth - 1; // 1 -> line
        wtp.Align = TextAlignament::Center;

        LocalString<10> ls;
        renderer.WriteText(ls.Format("%d", i), wtp);
    }

    return true;
}

bool GridControlContext::DrawHeaderForIndexesColumn(Graphics::Renderer& renderer)
{
    WriteTextParams wtp;
    wtp.Flags = WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth | WriteTextFlags::FitTextToWidth;
    wtp.Color = Cfg->Text.Normal;

    const int32 left   = START_DRAW_REL(offsetX, deltaX, cWidth);
    const int32 top    = START_DRAW_REL(offsetY, deltaY, cHeight);
    const int32 right  = END_DRAW_REL(left, cHeight);
    const int32 bottom = END_DRAW_REL(top, cHeight);
    renderer.FillRect(left, top, right, bottom, ' ', Cfg->Header.Text.Inactive);

    const auto indexHeader = headers[0];
    wtp.X                  = left; // 1 -> line
    wtp.Y                  = top;
    wtp.Width              = cWidth - 1; // 1 -> line
    wtp.Align              = indexHeader.ta;

    renderer.WriteText(indexHeader.content, wtp);
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

    if (((flags & GridFlags::DisableMove) != GridFlags::DisableMove) && startedScrolling == true)
    {

        int offset = offsetX - (int) cWidth + 1;

        offsetX = cWidth - 1 + offset / (int) cWidth * (int) cWidth;
        deltaX  = offsetX - cWidth + 1;

        offset  = offsetY - (int) cHeight + 1;
        offsetY = cHeight - 1 + offset / (int) cHeight * (int) cHeight; 
        deltaY  = offsetY - cHeight + 1;
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
    lastcWidth  = cWidth;
    lastcHeight = cHeight;
    cWidth += offsetX;
    cHeight += offsetY;

    cWidth  = std::max<>(minCellWidth, cWidth);
    cHeight = std::max<>(minCellHeight, cHeight);

    UpdateGridParameters(true);
}

void GridControlContext::ResetMatrixPosition()
{
    offsetX = cWidth - 1;
    deltaX  = 0;

    offsetY = cHeight - 1; /* header */
    deltaY  = 0;
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

            if (xLeft < std::max<>(0, offsetX + deltaX))
            {
                const auto currentDeltaX = cWidth;
                offsetX += currentDeltaX;
                deltaX -= cWidth;
                startedMoving = true;
            }

            if (xRight > Layout.Width)
            {
                const int currentDeltaX = xRight - Layout.Width;
                const int deltaOffsetX  = (currentDeltaX + cWidth - 1) / cWidth * cWidth;
                offsetX -= deltaOffsetX;
                deltaX += deltaOffsetX;
                startedMoving = true;
            }

            if (yTop < std::max<>(0, offsetY + deltaY))
            {
                const int currentDeltaY = cHeight;
                offsetY += currentDeltaY;
                deltaY -= cHeight;
                startedMoving = true;
            }

            if (yBottom > Layout.Height)
            {
                const int currentDeltaY = yBottom - Layout.Height + 1;
                offsetY -= (currentDeltaY + cHeight - 1) / cHeight * cHeight;
                deltaY += (currentDeltaY + cHeight - 1) / cHeight * cHeight;
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
            const auto& content = (*cells)[i][j].content;
            lusb.Add(content);

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
        const auto cellColumn = (*index) % columnsNo;
        const auto cellRow    = (*index) / columnsNo;
        auto& data            = cells->at(cellColumn).at(cellRow);
        data.content          = token;
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

    lusb.Set("Index");
    headers.push_back({ TextAlignament::Left, lusb });

    for (auto i = 0ULL; i < columnsNo; i++)
    {
        ls.SetFormat("Column_%llu", i);
        lusb.Set(ls);
        headers.push_back({ TextAlignament::Left, lusb });
    }
}

void GridControlContext::ReserveMap()
{
    (*cells) = std::vector<std::vector<GridCellData>>(columnsNo);
    for (auto& column : *cells)
    {
        column = std::vector<GridCellData>(rowsNo);
        for (auto& cellData : column)
        {
            cellData = { Graphics::TextAlignament::Left, u"" };
        }
    }
}

void GridControlContext::ToggleSorting(int x, int y)
{
    auto it = headers.begin();
    for (auto i = 0U; i <= columnsNo && it != headers.end(); i++)
    {
        const auto xHeader    = offsetX + i * cWidth + 1; // 1 -> line
        const auto yHeader    = offsetY - GetHeaderHeight() / 2;
        const auto endXHeader = xHeader + cWidth - 2;

        if ((x == (int) endXHeader) && (y == (int) yHeader))
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
    if (cells->empty())
    {
        return;
    }

    auto incSort = [](const GridCellData& a, const GridCellData& b) -> bool
    { return a.content.compare(b.content) < 0; };
    auto decSort = [](const GridCellData& a, const GridCellData& b) -> bool
    { return a.content.compare(b.content) > 0; };

    auto column = &(*cells)[colIndex];
    if (columnsSort[colIndex])
    {
        std::sort(column->begin(), column->end(), incSort);
    }
    else
    {
        std::sort(column->begin(), column->end(), decSort);
    }
}

void GridControlContext::FindDuplicates()
{
    duplicatedCellsIndexes.clear();
    CHECKRET(selectedCellsIndexes.size() == 1, "");

    const auto cellColumn = selectedCellsIndexes[0] % columnsNo;
    const auto cellRow    = selectedCellsIndexes[0] / columnsNo;

    const auto& content = (*cells)[cellColumn][cellRow].content;
    for (auto columnIndex = START(offsetX, cWidth); columnIndex < END(Layout.Width, offsetX, cWidth, columnsNo);
         columnIndex++)
    {
        for (auto rowIndex = START(offsetY, cHeight); rowIndex < END(Layout.Height, offsetY, cHeight, rowsNo);
             rowIndex++)
        {
            if (content.compare((*cells)[columnIndex][rowIndex].content) == 0)
            {
                const auto key = rowIndex * columnsNo + columnIndex;
                duplicatedCellsIndexes.emplace_back(key);
            }
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

uint32 GridControlContext::GetHeaderHeight() const
{
    return cHeight * (1 + ((flags & GridFlags::Filter) != GridFlags::None));
}

uint32 GridControlContext::GetColumnSelected() const
{
    CHECK(selectedCellsIndexes.size() == 1, -1, "");
    return static_cast<int32>(selectedCellsIndexes[0] / columnsNo);
}

} // namespace AppCUI
