#include "ControlContext.hpp"

namespace AppCUI
{
#define CHECK_INDEX(idx, returnValue)                                                                                  \
    CHECK(idx < (uint32) Members->Indexes.Len(),                                                                       \
          returnValue,                                                                                                 \
          "Invalid index (%d) , should be smaller than %d",                                                            \
          (int) idx,                                                                                                   \
          (int) Members->Indexes.Len());                                                                               \
    auto& i = Members->Items[Members->Indexes.GetUInt32Array()[idx]];                                                  \
    static_cast<void>(i);

Graphics::CharacterBuffer
      __temp_comboxitem_reference_object__; // use this as std::option<const T&> is not available yet

ComboBoxItem::ComboBoxItem() : Data(nullptr)
{
    this->Separator = false;
    this->Index     = ComboBox::NO_ITEM_SELECTED;
}
ComboBoxItem::ComboBoxItem(
      const ConstString& caption, variant<GenericRef, uint64> userData, uint32 index, bool separator)
    : Data(userData)
{
    this->Text.Set(caption);
    this->Separator = separator;
    this->Index     = index;
}
ComboBoxItem::~ComboBoxItem()
{
    // LOG_INFO("ComBoxItem(dtor) at %p [Text:Buffer = %p]", this, this->Text.GetBuffer());
    this->Text.Destroy();
    // LOG_INFO("           after delete = %p", this->Text.GetBuffer());
}
ComboBoxItem::ComboBoxItem(const ComboBoxItem& obj) : Data(obj.Data)
{
    // LOG_INFO("ComBoxItem(copy-ctor) at %p (from %p)", this, &obj);
    // LOG_INFO("           this->Buffer = %p", this->Text.GetBuffer());
    // LOG_INFO("           from->Buffer = %p", obj.Text.GetBuffer());
    this->Text.Set(obj.Text);
    this->Separator = obj.Separator;
    this->Index     = obj.Index;
    // LOG_INFO("           after-copy   = %p", this->Text.GetBuffer());
}
ComboBoxItem::ComboBoxItem(ComboBoxItem&& obj) noexcept : Data(nullptr)
{
    // LOG_INFO("ComBoxItem(move-ctor) at %p (from %p)", this, &obj);
    std::swap(this->Data, obj.Data);
    std::swap(this->Separator, obj.Separator);
    std::swap(this->Index, obj.Index);
    this->Text.Swap(obj.Text);
}
ComboBoxItem& ComboBoxItem::operator=(const ComboBoxItem& obj)
{
    // LOG_INFO("ComBoxItem(op-eq) at %p (from %p)", this, &obj);
    this->Data      = obj.Data;
    this->Separator = obj.Separator;
    this->Index     = obj.Index;
    this->Text.Set(obj.Text);
    return *this;
}
ComboBoxItem& ComboBoxItem::operator=(ComboBoxItem&& obj) noexcept
{
    // LOG_INFO("ComBoxItem(op-move) at %p (from %p)", this, &obj);
    this->Data      = obj.Data;
    this->Separator = obj.Separator;
    this->Index     = obj.Index;
    this->Text.Swap(obj.Text);
    return *this;
}

bool ComboBox_AddItem(
      ComboBox* control, const ConstString& caption, bool separator, variant<GenericRef, uint64> userData)
{
    CREATE_TYPE_CONTEXT(ComboBoxControlContext, control, Members, false);
    uint32 itemID  = (uint32) Members->Items.size();
    uint32 indexID = Members->Indexes.Len();
    CHECK(itemID < 0xFFFF, false, "A maximum of 0xFFFF indexes can be stored in a combobox !");
    if (!separator)
    {
        // only store available items (not separators)
        CHECK(Members->Indexes.Push(itemID), false, "Fail to add index: %u", itemID);
    }
    else
    {
        // separators items don't have an index
        indexID = ComboBox::NO_ITEM_SELECTED;
    }
    Members->Items.push_back(ComboBoxItem(caption, userData, indexID, separator));
    return true;
}

uint32 ComboBox_MousePosToIndex(ComboBox* control, int, int y)
{
    CREATE_TYPE_CONTEXT(ComboBoxControlContext, control, Members, ComboBox::NO_ITEM_SELECTED);
    if ((y > 1) && (y < (int) (2 + Members->VisibleItemsCount)))
    {
        uint32 newItem = ((uint32) (y - 2)) + Members->FirstVisibleItem;
        if (newItem >= Members->Items.size())
            return ComboBox::NO_ITEM_SELECTED;
        if (Members->Items[newItem].Separator)
            return ComboBox::NO_ITEM_SELECTED;
        return Members->Items[newItem].Index;
    }
    return ComboBox::NO_ITEM_SELECTED;
}
void ComboBox_SetCurrentIndex(ComboBox* control, uint32 newIndex)
{
    CREATE_TYPE_CONTEXT(ComboBoxControlContext, control, Members, );
    uint32 old          = Members->CurentItemIndex;
    uint32 indexesCount = (uint32) Members->Indexes.Len();

    if (indexesCount == 0)
    {
        Members->CurentItemIndex  = ComboBox::NO_ITEM_SELECTED;
        Members->FirstVisibleItem = 0;
        return;
    }

    if (newIndex != ComboBox::NO_ITEM_SELECTED)
    {
        newIndex = std::min<>(newIndex, indexesCount - 1);
        // compute FirstVisibleItem
        uint32 cItem = Members->Indexes.GetUInt32Array()[newIndex];
        if ((cItem >= Members->FirstVisibleItem) && (cItem < Members->FirstVisibleItem + Members->VisibleItemsCount))
        {
            // all good - nothing to do
        }
        else
        {
            if ((cItem + 1) > Members->VisibleItemsCount)
                Members->FirstVisibleItem = cItem + 1 - Members->VisibleItemsCount;
            else
                Members->FirstVisibleItem = 0;
        }
    }
    Members->CurentItemIndex = newIndex;
    if (old != newIndex)
        control->RaiseEvent(Event::ComboBoxSelectedItemChanged);
}
//====================================================================================================
template <typename T>
uint32 ComputeItemsCount(const T* start, size_t len, char separator)
{
    if (start == nullptr)
        return 0;
    const T* end = start + len;
    uint32 count = 1; // assume at least one element (if not item separator is found)
    while (start < end)
    {
        if ((*start) == separator)
            count++;
        start++;
    }
    return count;
}

template <typename T, typename SV_T>
bool AddItemsFromList(ComboBox* cbx, const T* p, size_t len, const char separator)
{
    const T* end   = p + len;
    const T* start = p;
    for (; p < end; p++)
    {
        if ((*p) == separator)
        {
            if (start < p)
            {
                CHECK(ComboBox_AddItem(cbx, SV_T(start, p - start), false, 0ULL), false, "");
            }
            start = p + 1;
        }
    }
    if (start < p)
    {
        CHECK(ComboBox_AddItem(cbx, SV_T(start, p - start), false, 0ULL), false, "");
    }
    return true;
}

//====================================================================================================
ComboBox::~ComboBox()
{
    DELETE_CONTROL_CONTEXT(ComboBoxControlContext);
}
ComboBox::ComboBox(string_view layout, const ConstString& text, char itemsSeparator)
    : Control(new ComboBoxControlContext(), "", layout, false)
{
    auto Members                    = reinterpret_cast<ComboBoxControlContext*>(this->Context);
    Members->Layout.MinWidth        = 7;
    Members->Layout.MinHeight       = 1;
    Members->Layout.MaxHeight       = 1;
    Members->Flags                  = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
    uint32 initialAllocatedElements = 16;
    uint32 count                    = 0;

    ConstStringObject listItems(text);
    switch (listItems.Encoding)
    {
    case StringEncoding::Ascii:
        count = ComputeItemsCount<char>((const char*) listItems.Data, listItems.Length, itemsSeparator);
        break;
    case StringEncoding::UTF8:
        count = ComputeItemsCount<char8_t>((const char8_t*) listItems.Data, listItems.Length, itemsSeparator);
        break;
    case StringEncoding::Unicode16:
        count = ComputeItemsCount<char16>((const char16*) listItems.Data, listItems.Length, itemsSeparator);
        break;
    case StringEncoding::CharacterBuffer:
        count = ComputeItemsCount<Character>((const Character*) listItems.Data, listItems.Length, itemsSeparator);
        break;
    default:
        ASSERT(false, "Invalid encoding type !");
    }
    if (count > initialAllocatedElements)
        initialAllocatedElements = (count | 3) + 1;

    Members->Items.reserve(initialAllocatedElements);
    ASSERT(Members->Indexes.Create(initialAllocatedElements), "Fail to initialize elements indexes !");

    if (count > 0)
    {
        bool result = false;
        switch (listItems.Encoding)
        {
        case StringEncoding::Ascii:
            result = AddItemsFromList<char, string_view>(
                  this, (const char*) listItems.Data, listItems.Length, itemsSeparator);
            break;
        case StringEncoding::UTF8:
            result = AddItemsFromList<char8_t, u8string_view>(
                  this, (const char8_t*) listItems.Data, listItems.Length, itemsSeparator);
            break;
        case StringEncoding::Unicode16:
            result = AddItemsFromList<char16, u16string_view>(
                  this, (const char16*) listItems.Data, listItems.Length, itemsSeparator);
            break;
        case StringEncoding::CharacterBuffer:
            result = AddItemsFromList<Character, CharacterView>(
                  this, (const Character*) listItems.Data, listItems.Length, itemsSeparator);
            break;
        default:
            ASSERT(false, "Invalid encoding type !");
        }
        ASSERT(result, "Fail to add items to ComboBox !");
    }
    Members->VisibleItemsCount = 1;
    Members->CurentItemIndex   = ComboBox::NO_ITEM_SELECTED;
    Members->FirstVisibleItem  = 0;
}

uint32 ComboBox::GetItemsCount() const
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, 0);
    return (uint32) Members->Indexes.Len();
}

uint32 ComboBox::GetCurrentItemIndex() const
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, ComboBox::NO_ITEM_SELECTED);
    return Members->CurentItemIndex;
}

uint64 ComboBox::GetItemUserData(uint32 index, uint64 errorValue) const
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, errorValue);
    CHECK_INDEX(index, errorValue);
    if (std::holds_alternative<uint64>(i.Data))
        return std::get<uint64>(i.Data);
    return errorValue;
}
GenericRef ComboBox::GetItemDataAsPointer(uint32 index) const
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, nullptr);
    CHECK_INDEX(index, nullptr);
    if (std::holds_alternative<GenericRef>(i.Data))
        return std::get<GenericRef>(i.Data);
    return nullptr;
}

const Graphics::CharacterBuffer& ComboBox::GetCurrentItemText()
{
    __temp_comboxitem_reference_object__.Destroy();
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, __temp_comboxitem_reference_object__);
    return GetItemText(Members->CurentItemIndex);
}
const Graphics::CharacterBuffer& ComboBox::GetItemText(uint32 index)
{
    __temp_comboxitem_reference_object__.Destroy();
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, __temp_comboxitem_reference_object__);
    CHECK_INDEX(index, __temp_comboxitem_reference_object__);
    return i.Text;
}
bool ComboBox::SetItemDataAsPointer(uint32 index, GenericRef userData)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    CHECK_INDEX(index, false);
    i.Data = userData;
    return true;
}
bool ComboBox::SetItemUserData(uint32 index, uint64 userData)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    CHECK_INDEX(index, false);
    i.Data = userData;
    return true;
}
bool ComboBox::AddItem(const ConstString& caption, uint64 userData)
{
    CHECK(ComboBox_AddItem(this, caption, false, userData), false, "");
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    if (Members->Items.size() > 0)
    {
        Members->CurentItemIndex  = 0;
        Members->FirstVisibleItem = 0;
    }
    return true;
}
bool ComboBox::AddItem(const ConstString& caption, GenericRef userData)
{
    CHECK(ComboBox_AddItem(this, caption, false, userData), false, "");
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    if (Members->Items.size() > 0)
    {
        Members->CurentItemIndex  = 0;
        Members->FirstVisibleItem = 0;
    }
    return true;
}
bool ComboBox::AddSeparator(const ConstString& caption)
{
    return ComboBox_AddItem(this, caption, true, { nullptr });
}
void ComboBox::DeleteAllItems()
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, );
    Members->Items.clear();
    Members->Indexes.Clear();
    Members->CurentItemIndex  = ComboBox::NO_ITEM_SELECTED;
    Members->FirstVisibleItem = 0;
}
bool ComboBox::SetCurentItemIndex(uint32 index)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    CHECK_INDEX(index, false);
    ComboBox_SetCurrentIndex(this, index);
    return true;
}
void ComboBox::SetNoIndexSelected()
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, );
    Members->CurentItemIndex  = ComboBox::NO_ITEM_SELECTED;
    Members->FirstVisibleItem = 0;
    RaiseEvent(Event::ComboBoxSelectedItemChanged);
}

bool ComboBox::OnKeyEvent(Input::Key keyCode, char16)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    switch (keyCode)
    {
    case Key::Up:
        if ((Members->CurentItemIndex == ComboBox::NO_ITEM_SELECTED) && (Members->Indexes.Len() > 0))
            ComboBox_SetCurrentIndex(this, 0);
        else if (Members->CurentItemIndex > 0)
            ComboBox_SetCurrentIndex(this, Members->CurentItemIndex - 1);
        return true;
    case Key::Down:
        if ((Members->CurentItemIndex == ComboBox::NO_ITEM_SELECTED) && (Members->Indexes.Len() > 0))
            ComboBox_SetCurrentIndex(this, 0);
        else if (Members->CurentItemIndex != ComboBox::NO_ITEM_SELECTED)
            ComboBox_SetCurrentIndex(this, Members->CurentItemIndex + 1);
        return true;
    case Key::Home:
        ComboBox_SetCurrentIndex(this, 0);
        return true;
    case Key::End:
        ComboBox_SetCurrentIndex(this, 0xFFFF);
        return true;

    case Key::Space:
    case Key::Enter:
        OnHotKey();
        return true;
    case Key::Escape:
        if (IsChecked())
        {
            OnHotKey();
            return true;
        }
        break;
    }
    return false;
}
void ComboBox::OnExpandView(Graphics::Clip& expandedClip)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, );
    Graphics::Size appSize;
    Members->VisibleItemsCount = 4;
    Members->HoveredIndexItem  = ComboBox::NO_ITEM_SELECTED;
    if ((Application::GetApplicationSize(appSize)) && (expandedClip.ClipRect.Y >= 0))
    {
        if (appSize.Height > (uint32) (expandedClip.ClipRect.Y + 3))
            Members->VisibleItemsCount = (appSize.Height - (uint32) (expandedClip.ClipRect.Y + 3));
    }
    if (Members->VisibleItemsCount > Members->Items.size())
        Members->VisibleItemsCount = (uint32) Members->Items.size();
    expandedClip.ClipRect.Height += Members->VisibleItemsCount + 2;
    Members->ExpandedHeight = Members->VisibleItemsCount + 2;
    // compute best FirstVisibleItem
    if (Members->CurentItemIndex != ComboBox::NO_ITEM_SELECTED)
    {
        auto cItem = Members->Indexes.GetUInt32Array()[Members->CurentItemIndex];
        if ((Members->VisibleItemsCount / 2) <= cItem)
            Members->FirstVisibleItem = cItem - (Members->VisibleItemsCount / 2);
        else
            Members->FirstVisibleItem = 0;
        if ((Members->FirstVisibleItem + Members->VisibleItemsCount) >= Members->Items.size())
        {
            if (Members->VisibleItemsCount < Members->Items.size())
                Members->FirstVisibleItem = (uint32) (Members->Items.size() - Members->VisibleItemsCount);
            else
                Members->FirstVisibleItem = 0;
        }
    }
}
void ComboBox::OnPackView()
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, );
    Members->VisibleItemsCount = 1;
    Members->HoveredIndexItem  = ComboBox::NO_ITEM_SELECTED;
    if (Members->CurentItemIndex != ComboBox::NO_ITEM_SELECTED)
        Members->FirstVisibleItem = Members->Indexes.GetUInt32Array()[Members->CurentItemIndex];
}
void ComboBox::OnHotKey()
{
    SetChecked(!IsChecked());
    if (IsChecked())
        this->ExpandView();
    else
    {
        this->PackView();
        RaiseEvent(Event::ComboBoxClosed);
    }
}
void ComboBox::OnMousePressed(int x, int y, Input::MouseButton)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, );
    uint32 idx = ComboBox_MousePosToIndex(this, x, y);
    if (idx != ComboBox::NO_ITEM_SELECTED)
        ComboBox_SetCurrentIndex(this, idx);
    OnHotKey();
}
bool ComboBox::OnMouseWheel(int, int, Input::MouseWheel direction)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);

    if (Members->Flags & GATTR_EXPANDED)
    {
        switch (direction)
        {
        case Input::MouseWheel::Up:
            if (Members->FirstVisibleItem > 0)
                Members->FirstVisibleItem--;
            return true;
        case Input::MouseWheel::Down:
            if ((size_t) Members->FirstVisibleItem + (size_t) Members->VisibleItemsCount < Members->Items.size())
                Members->FirstVisibleItem++;
            return true;
        }
    }
    else
    {
        switch (direction)
        {
        case Input::MouseWheel::Up:
            return OnKeyEvent(Key::Up, 0);
        case Input::MouseWheel::Down:
            return OnKeyEvent(Key::Down, 0);
        }
    }
    return false;
}
bool ComboBox::OnMouseOver(int x, int y)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    uint32 newIndex = ComboBox_MousePosToIndex(this, x, y);
    if (newIndex != Members->HoveredIndexItem)
    {
        Members->HoveredIndexItem = newIndex;
        return true; // repaint required
    }
    return false;
}
bool ComboBox::OnMouseLeave()
{
    return true;
}
bool ComboBox::OnMouseEnter()
{
    return true;
}
void ComboBox::Paint(Graphics::Renderer& renderer)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, );

    WriteTextParams params(
          WriteTextFlags::OverwriteColors | WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth |
                WriteTextFlags::FitTextToWidth,
          TextAlignament::Left);
    WriteTextParams paramsSeparator(
          WriteTextFlags::OverwriteColors | WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth |
                WriteTextFlags::FitTextToWidth | WriteTextFlags::LeftMargin | WriteTextFlags::RightMargin,
          TextAlignament::Center);

    paramsSeparator.Width = Members->Layout.Width - 7;
    paramsSeparator.X     = 3;
    paramsSeparator.Color = Members->Cfg->Menu.Text.Inactive;
    auto state            = Members->GetControlState(ControlStateFlags::ProcessHoverStatus);
    auto cbc              = Members->Cfg->Button.Text.GetColor(state);
    auto itemsCount       = Members->Items.size();

    renderer.FillHorizontalLine(0, 0, Members->Layout.Width - 5, ' ', cbc);
    if (Members->CurentItemIndex < Members->Indexes.Len())
    {
        params.X     = 1;
        params.Y     = 0;
        params.Width = Members->Layout.Width - 6;
        params.Color = cbc;
        renderer.WriteText(Members->Items[Members->Indexes.GetUInt32Array()[Members->CurentItemIndex]].Text, params);
    }
    renderer.WriteSingleLineText(Members->Layout.Width - 3, 0, "   ", cbc);
    renderer.WriteSpecialCharacter(Members->Layout.Width - 2, 0, SpecialChars::TriangleDown, cbc);

    if (Members->Flags & GATTR_EXPANDED)
    {
        renderer.FillRect(
              0, 1, Members->Layout.Width - 1, Members->ExpandedHeight, ' ', Members->Cfg->Menu.Text.Normal);
        renderer.DrawRect(
              0,
              1,
              Members->Layout.Width - 1,
              Members->ExpandedHeight,
              Members->Cfg->Menu.Text.Normal,
              LineType::Single);
        params.X     = 1;
        params.Width = Members->Layout.Width - 2;
        params.Color = Members->Cfg->Menu.Text.Normal;
        for (uint32 tr = 0; tr < Members->VisibleItemsCount; tr++)
        {
            if ((tr + Members->FirstVisibleItem) >= itemsCount)
                break;

            auto& i = Members->Items[tr + Members->FirstVisibleItem];
            if (i.Separator)
            {
                renderer.FillHorizontalLineWithSpecialChar(
                      1,
                      tr + 2,
                      Members->Layout.Width - 2,
                      SpecialChars::BoxHorizontalSingleLine,
                      Members->Cfg->Menu.Text.Normal);
                if (i.Text.Len() > 0)
                {
                    paramsSeparator.Y = tr + 2;
                    renderer.WriteText(i.Text, paramsSeparator);
                }
            }
            else
            {
                params.Y = tr + 2;
                renderer.WriteText(i.Text, params);
                // cursor or selection
                if (i.Index == Members->CurentItemIndex)
                    renderer.FillHorizontalLine(
                          1,
                          tr + 2,
                          Members->Layout.Width - 2,
                          -1,
                          Members->Cfg->Menu.Text.PressedOrSelected); // a different color
                else if (i.Index == Members->HoveredIndexItem)
                    renderer.FillHorizontalLine(
                          1, tr + 2, Members->Layout.Width - 2, -1, Members->Cfg->Menu.Text.Hovered);
            }
        }
    }
}
} // namespace AppCUI