#include "ControlContext.hpp"

using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;
using namespace AppCUI::Input;

#define CHECK_INDEX(idx, returnValue)                                                                                  \
    CHECK(idx < (unsigned int) Members->Indexes.Len(),                                                                 \
          returnValue,                                                                                                 \
          "Invalid index (%d) , should be smaller than %d",                                                            \
          (int) idx,                                                                                                   \
          (int) Members->Indexes.Len());                                                                               \
    auto& i = Members->Items[Members->Indexes.GetUInt32Array()[idx]];

static const ItemData null_combobox_item = { 0 };

AppCUI::Graphics::CharacterBuffer __temp_comboxitem_reference_object__; // use this as std::option<const T&> is not available yet

ComboBoxItem::ComboBoxItem()
{
    this->Separator = false;
    this->Index     = ComboBox::NO_ITEM_SELECTED;
    this->Data      = { 0 };
}
ComboBoxItem::ComboBoxItem(const AppCUI::Utils::ConstString& caption, ItemData userData, unsigned int index, bool separator)
{
    this->Text.Set(caption);
    this->Data      = userData;
    this->Separator = separator;
    this->Index     = index;
}
ComboBoxItem::~ComboBoxItem()
{
    //LOG_INFO("ComBoxItem(dtor) at %p [Text:Buffer = %p]", this, this->Text.GetBuffer());
    this->Text.Destroy();
    //LOG_INFO("           after delete = %p", this->Text.GetBuffer());
}
ComboBoxItem::ComboBoxItem(const ComboBoxItem& obj)
{
    //LOG_INFO("ComBoxItem(copy-ctor) at %p (from %p)", this, &obj);
    //LOG_INFO("           this->Buffer = %p", this->Text.GetBuffer());
    //LOG_INFO("           from->Buffer = %p", obj.Text.GetBuffer());
    this->Data = obj.Data;
    this->Text.Set(obj.Text);
    this->Separator = obj.Separator;
    this->Index     = obj.Index;
    //LOG_INFO("           after-copy   = %p", this->Text.GetBuffer());
}
ComboBoxItem::ComboBoxItem(ComboBoxItem&& obj) noexcept 
{
    //LOG_INFO("ComBoxItem(move-ctor) at %p (from %p)", this, &obj);
    std::swap(this->Data, obj.Data);
    std::swap(this->Separator, obj.Separator);
    std::swap(this->Index, obj.Index);
    this->Text.Swap(obj.Text);
}
ComboBoxItem& ComboBoxItem::operator=(const ComboBoxItem& obj)
{
    //LOG_INFO("ComBoxItem(op-eq) at %p (from %p)", this, &obj);
    this->Data      = obj.Data;
    this->Separator = obj.Separator;
    this->Index     = obj.Index;
    this->Text.Set(obj.Text);
    return *this;
}
ComboBoxItem& ComboBoxItem::operator=(ComboBoxItem&& obj) noexcept
{
    //LOG_INFO("ComBoxItem(op-move) at %p (from %p)", this, &obj);
    this->Data      = obj.Data;
    this->Separator = obj.Separator;
    this->Index     = obj.Index;
    this->Text.Swap(obj.Text);    
    return *this;
}

bool ComboBox_AddItem(ComboBox* control, const AppCUI::Utils::ConstString& caption, bool separator, ItemData userData = { 0 })
{
    CREATE_TYPE_CONTEXT(ComboBoxControlContext, control, Members, false);
    unsigned int itemID = (unsigned int) Members->Items.size();
    unsigned int indexID = Members->Indexes.Len();
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

unsigned int ComboBox_MousePosToIndex(ComboBox* control, int x, int y)
{
    CREATE_TYPE_CONTEXT(ComboBoxControlContext, control, Members, ComboBox::NO_ITEM_SELECTED);
    if ((y > 1) && (y < (int) (2 + Members->VisibleItemsCount)))
    {
        unsigned int newItem= ((unsigned int) (y - 2)) + Members->FirstVisibleItem;
        if (newItem >= Members->Items.size())
            return ComboBox::NO_ITEM_SELECTED;
        if (Members->Items[newItem].Separator)
            return ComboBox::NO_ITEM_SELECTED;
        return Members->Items[newItem].Index;
    }
    return ComboBox::NO_ITEM_SELECTED;
}
void ComboBox_SetCurrentIndex(ComboBox* control, unsigned int newIndex)
{
    CREATE_TYPE_CONTEXT(ComboBoxControlContext, control, Members, );
    unsigned int old          = Members->CurentItemIndex;
    unsigned int indexesCount = (unsigned int) Members->Indexes.Len();
    unsigned int itemsCount   = (unsigned int) Members->Items.size();

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
        unsigned int cItem = Members->Indexes.GetUInt32Array()[newIndex];
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
        control->RaiseEvent(Event::EVENT_COMBOBOX_SELECTED_ITEM_CHANGED);
}
//====================================================================================================
template <typename T>
unsigned int ComputeItemsCount(const T* start, size_t len, char separator)
{
    if (start == nullptr)
        return 0;
    const T* end       = start + len;
    unsigned int count = 1; // assume at least one element (if not item separator is found)
    while (start<end)
    {
        if ((*start) == separator)
            count++;
        start++;
    }
    return count;
}

template <typename T,typename SV_T>
bool AddItemsFromList(ComboBox* cbx, const T* p, size_t len, const char separator)
{    
    const T* end   = p + len;
    const T* start = p;
    for (;p<end; p++)
    {
        if ((*p) == separator)
        {
            if (start < p)
            {
                CHECK(ComboBox_AddItem(cbx, SV_T(start, p - start),false), false, "");
            }
            start = p + 1;
        }
    }
    if (start < p)
    {
        CHECK(ComboBox_AddItem(cbx, SV_T(start, p - start),false), false, "");
    }
    return true;
}

//====================================================================================================
ComboBox::~ComboBox()
{
    DELETE_CONTROL_CONTEXT(ComboBoxControlContext);
}
bool ComboBox::Create(Control* parent, const std::string_view& layout, const AppCUI::Utils::ConstString& text, char itemsSeparator)
{
    CONTROL_INIT_CONTEXT(ComboBoxControlContext);
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    Members->Layout.MinWidth  = 7;
    Members->Layout.MinHeight = 1;
    Members->Layout.MaxHeight = 1;
    CHECK(Init(parent, "", layout, false), false, "Failed to create combo box !");

    Members->Flags                        = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
    unsigned int initialAllocatedElements = 16;
    unsigned int count                    = 0;
    
    AppCUI::Utils::ConstStringObject listItems(text);
    switch (listItems.Encoding)
    {
        case StringEncoding::Ascii:
            count = ComputeItemsCount<char>((const char *)listItems.Data, listItems.Length, itemsSeparator);
            break;
        case StringEncoding::UTF8:
            count = ComputeItemsCount<char8_t>((const char8_t*) listItems.Data, listItems.Length, itemsSeparator);
            break;
        case StringEncoding::Unicode16:
            count = ComputeItemsCount<char16_t>((const char16_t*) listItems.Data, listItems.Length, itemsSeparator);
            break;
        case StringEncoding::CharacterBuffer:
            count = ComputeItemsCount<Character>((const Character*) listItems.Data, listItems.Length, itemsSeparator);
            break;
        default:
            RETURNERROR(false, "Invalid string type !");
    }
    if (count > initialAllocatedElements)
        initialAllocatedElements = (count | 3) + 1;

    Members->Items.reserve(initialAllocatedElements);
    CHECK(Members->Indexes.Create(initialAllocatedElements), false, "Fail to initialize %d elements indexes !", initialAllocatedElements);
    if (count>0)
    {
        bool result = false;
        switch (listItems.Encoding)
        {
        case StringEncoding::Ascii:
            result = AddItemsFromList<char, std::string_view>(this,(const char*) listItems.Data,listItems.Length,itemsSeparator);
            break;
        case StringEncoding::UTF8:
            result = AddItemsFromList<char8_t, std::u8string_view>(this, (const char8_t*) listItems.Data, listItems.Length, itemsSeparator);
            break;
        case StringEncoding::Unicode16:
            result = AddItemsFromList<char16_t, std::u16string_view>(this, (const char16_t*) listItems.Data, listItems.Length, itemsSeparator);
            break;
        case StringEncoding::CharacterBuffer:
            result = AddItemsFromList<Character, CharacterView>(this, (const Character*) listItems.Data, listItems.Length, itemsSeparator);
            break;
        default:
            RETURNERROR(false, "Invalid string type !");
        }
        CHECK(result, false, "Fail to add items to ComboBox !");
    }
    Members->VisibleItemsCount = 1;
    Members->CurentItemIndex   = ComboBox::NO_ITEM_SELECTED;
    Members->FirstVisibleItem  = 0;
    return true;
}
unsigned int ComboBox::GetItemsCount()
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, 0);
    return (unsigned int) Members->Indexes.Len();
}

unsigned int ComboBox::GetCurrentItemIndex()
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, ComboBox::NO_ITEM_SELECTED);
    return Members->CurentItemIndex;
}
ItemData ComboBox::GetCurrentItemUserData()
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, null_combobox_item);
    return GetItemUserData(Members->CurentItemIndex);
}
ItemData ComboBox::GetItemUserData(unsigned int index)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, null_combobox_item);
    CHECK_INDEX(index, null_combobox_item);
    return i.Data;
}

const AppCUI::Graphics::CharacterBuffer& ComboBox::GetCurrentItemText()
{
    __temp_comboxitem_reference_object__.Destroy();
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, __temp_comboxitem_reference_object__);
    return GetItemText(Members->CurentItemIndex);
}
const AppCUI::Graphics::CharacterBuffer& ComboBox::GetItemText(unsigned int index)
{
    __temp_comboxitem_reference_object__.Destroy();
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, __temp_comboxitem_reference_object__);
    CHECK_INDEX(index, __temp_comboxitem_reference_object__);
    return i.Text;
}
bool ComboBox::SetItemUserData(unsigned int index, ItemData userData)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    CHECK_INDEX(index, false);
    i.Data = userData;
    return true;
}
bool ComboBox::AddItem(const AppCUI::Utils::ConstString& caption, ItemData userData)
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
bool ComboBox::AddSeparator(const AppCUI::Utils::ConstString& caption)
{
    return ComboBox_AddItem(this, caption, true, { 0 });
}
void ComboBox::DeleteAllItems()
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, );
    Members->Items.clear();
    Members->Indexes.Clear();
    Members->CurentItemIndex  = ComboBox::NO_ITEM_SELECTED;
    Members->FirstVisibleItem = 0;
}
bool ComboBox::SetCurentItemIndex(unsigned int index)
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
    RaiseEvent(Event::EVENT_COMBOBOX_SELECTED_ITEM_CHANGED);
}

bool ComboBox::OnKeyEvent(AppCUI::Input::Key keyCode, char16_t UnicodeChar)
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
void ComboBox::OnExpandView(AppCUI::Graphics::Clip& expandedClip)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, );
    AppCUI::Graphics::Size appSize;
    Members->VisibleItemsCount = 4;
    Members->HoveredIndexItem  = ComboBox::NO_ITEM_SELECTED;
    if ((Application::GetApplicationSize(appSize)) && (expandedClip.ClipRect.Y >= 0))
    {
        if (appSize.Height > (unsigned int) (expandedClip.ClipRect.Y + 3))
            Members->VisibleItemsCount = (appSize.Height - (unsigned int) (expandedClip.ClipRect.Y + 3));
    }
    if (Members->VisibleItemsCount > Members->Items.size())
        Members->VisibleItemsCount = (unsigned int) Members->Items.size();
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
        if ((Members->FirstVisibleItem + Members->VisibleItemsCount)>=Members->Items.size())
        {
            if (Members->VisibleItemsCount < Members->Items.size())
                Members->FirstVisibleItem = (unsigned int)(Members->Items.size() - Members->VisibleItemsCount);
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
        RaiseEvent(Event::EVENT_COMBO_CLOSED);
    }
}
void ComboBox::OnMousePressed(int x, int y, AppCUI::Input::MouseButton button)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, );
    unsigned int idx = ComboBox_MousePosToIndex(this, x, y);
    if (idx != ComboBox::NO_ITEM_SELECTED)
        ComboBox_SetCurrentIndex(this, idx);
    OnHotKey();
}
bool ComboBox::OnMouseWheel(int x, int y, AppCUI::Input::MouseWheel direction)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);

    if (Members->Flags & GATTR_EXPANDED)
    {
        switch (direction)
        {
        case AppCUI::Input::MouseWheel::Up:
            if (Members->FirstVisibleItem > 0)
                Members->FirstVisibleItem--;
            return true;
        case AppCUI::Input::MouseWheel::Down:
            if ((size_t)Members->FirstVisibleItem + (size_t)Members->VisibleItemsCount < Members->Items.size())
                Members->FirstVisibleItem++;
            return true;
        }
    } else
    {
        switch (direction)
        {
            case AppCUI::Input::MouseWheel::Up:
                return OnKeyEvent(Key::Up, 0);
            case AppCUI::Input::MouseWheel::Down:
                return OnKeyEvent(Key::Down, 0);
        }
    }
    return false;
}
bool ComboBox::OnMouseOver(int x, int y)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    unsigned int newIndex = ComboBox_MousePosToIndex(this, x, y);
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

    WriteTextParams params(WriteTextFlags::OverwriteColors | WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth |WriteTextFlags::FitTextToWidth,
                           TextAlignament::Left);
    WriteTextParams paramsSeparator(
          WriteTextFlags::OverwriteColors | WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth |
          WriteTextFlags::FitTextToWidth | WriteTextFlags::LeftMargin | WriteTextFlags::RightMargin,
          TextAlignament::Center);

    paramsSeparator.Width = Members->Layout.Width - 7;
    paramsSeparator.X     = 3;
    paramsSeparator.Color = Members->Cfg->ComboBox.Inactive.Text;
    auto* cbc = &Members->Cfg->ComboBox.Normal;  
    if (!this->IsEnabled())
        cbc = &Members->Cfg->ComboBox.Inactive;
    if (Members->Focused)
        cbc = &Members->Cfg->ComboBox.Focus;
    else if (Members->MouseIsOver)
        cbc = &Members->Cfg->ComboBox.Hover;

    auto itemsCount = Members->Items.size();
    renderer.FillHorizontalLine(0, 0, Members->Layout.Width - 5, ' ', cbc->Text);
    if (Members->CurentItemIndex < Members->Indexes.Len())
    {
        params.X     = 1;
        params.Y     = 0;
        params.Width = Members->Layout.Width - 6;
        params.Color = cbc->Text;
        renderer.WriteText(Members->Items[Members->Indexes.GetUInt32Array()[Members->CurentItemIndex]].Text, params);
    }        
    renderer.WriteSingleLineText(Members->Layout.Width - 3, 0, "   ", cbc->Button);
    renderer.WriteSpecialCharacter(Members->Layout.Width - 2, 0, SpecialChars::TriangleDown, cbc->Button);

    if (Members->Flags & GATTR_EXPANDED)
    {
        renderer.FillRect(0, 1, Members->Layout.Width - 1, Members->ExpandedHeight, ' ', Members->Cfg->ComboBox.Focus.Text);
        renderer.DrawRect(0, 1, Members->Layout.Width - 1, Members->ExpandedHeight, Members->Cfg->ComboBox.Focus.Text, false);
        params.X     = 1;
        params.Width = Members->Layout.Width - 2;
        params.Color = Members->Cfg->ComboBox.Focus.Text;
        for (unsigned int tr = 0; tr < Members->VisibleItemsCount; tr++)
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
                        Members->Cfg->ComboBox.Inactive.Text);
                if (i.Text.Len()>0)
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
                    renderer.FillHorizontalLine(1, tr + 2, Members->Layout.Width - 2, -1, Members->Cfg->ComboBox.Selection);
                else if (i.Index == Members->HoveredIndexItem)
                    renderer.FillHorizontalLine(1, tr + 2, Members->Layout.Width - 2, -1, Members->Cfg->ComboBox.HoverOveItem);
            }                           
        }
    }
}
