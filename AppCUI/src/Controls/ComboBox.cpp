#include "ControlContext.hpp"

using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;
using namespace AppCUI::Input;

#define CHECK_INDEX(idx, returnValue)                                                                                  \
    CHECK(idx < (unsigned int) Members->Items.size(),                                                                  \
          returnValue,                                                                                                 \
          "Invalid index (%d) , should be smaller than %d",                                                            \
          (int) idx,                                                                                                   \
          (int) Members->Items.size());
static ItemData null_combobox_item = { 0 };

AppCUI::Graphics::CharacterBuffer __temp_comboxitem_reference_object__; // use this as std::option<const T&> is not available yet

ComboBoxItem::ComboBoxItem()
{
    //LOG_INFO("ComBoxItem(ctor) at %p [Text:Buffer = %p]",this, this->Text.GetBuffer());
}
ComboBoxItem::ComboBoxItem(const AppCUI::Utils::ConstString& caption, ItemData userData)
{
    this->Text.Set(caption);
    this->Data = userData;
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
    //LOG_INFO("           after-copy   = %p", this->Text.GetBuffer());
}
ComboBoxItem::ComboBoxItem(ComboBoxItem&& obj)
{
    //LOG_INFO("ComBoxItem(move-ctor) at %p (from %p)", this, &obj);
    std::swap(this->Data, obj.Data);
    this->Text.Swap(obj.Text);
}
ComboBoxItem& ComboBoxItem::operator=(const ComboBoxItem& obj)
{
    //LOG_INFO("ComBoxItem(op-eq) at %p (from %p)", this, &obj);
    this->Data = obj.Data;
    this->Text.Set(obj.Text);
    return *this;
}
ComboBoxItem& ComboBoxItem::operator=(ComboBoxItem&& obj) noexcept
{
    //LOG_INFO("ComBoxItem(op-move) at %p (from %p)", this, &obj);
    this->Data = obj.Data;
    this->Text.Swap(obj.Text);
    return *this;
}

bool ComboBox_AddItem(ComboBox* control, const AppCUI::Utils::ConstString& caption, ItemData userData = { 0 })
{
    //LOG_INFO("ENTER --------------------------------------------------------------- ComboBox_AddItem ");
    CREATE_TYPE_CONTEXT(ComboBoxControlContext, control, Members, false);
    Members->Items.push_back(ComboBoxItem(caption,userData));
    //LOG_INFO("EXIT ---------------------------------------------------------------- ComboBox_AddItem ");
    return true;
}
void ComboBox_MoveTo(ComboBox* control, unsigned int newPoz)
{
    CREATE_TYPE_CONTEXT(ComboBoxControlContext, control, Members, );
    unsigned int old        = Members->CurentItemIndex;
    unsigned int itemsCount = (unsigned int) Members->Items.size();
    if (itemsCount == 0)
        return;
    if ((itemsCount > 0) && (newPoz > itemsCount - 1))
        newPoz = itemsCount - 1;

    while (Members->CurentItemIndex != newPoz)
    {
        if (Members->CurentItemIndex > newPoz)
            Members->CurentItemIndex--;
        if (Members->CurentItemIndex < newPoz)
            Members->CurentItemIndex++;
        if (Members->CurentItemIndex < Members->FirstVisibleItem)
            Members->FirstVisibleItem = Members->CurentItemIndex;
        if (Members->CurentItemIndex >= Members->FirstVisibleItem + Members->VisibleItems)
        {
            Members->FirstVisibleItem = (Members->CurentItemIndex - Members->VisibleItems) + 1;
        }
    }
    if (old != Members->CurentItemIndex)
        control->RaiseEvent(Event::EVENT_COMBOBOX_SELECTED_ITEM_CHANGED);
}
//====================================================================================================
template <typename T>
unsigned int ComputeItemsCount(const T* start, size_t len, const T separator)
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
bool AddItemsFromList(ComboBox* cbx, const T* p, size_t len, const T separator)
{    
    const T* end   = p + len;
    const T* start = p;
    for (;p<end; p++)
    {
        if ((*p) == separator)
        {
            if (start < p)
            {
                CHECK(ComboBox_AddItem(cbx, SV_T(start, p - start)), false, "");
            }
            start = p + 1;
        }
    }
    if (start < p)
    {
        CHECK(ComboBox_AddItem(cbx, SV_T(start, p - start)), false, "");
    }
    return true;
}

//====================================================================================================
ComboBox::~ComboBox()
{
    DELETE_CONTROL_CONTEXT(ComboBoxControlContext);
}
bool ComboBox::Create(
      Control* parent, const std::string_view& layout, const AppCUI::Utils::ConstString& text, char itemsSeparator)
{
    CONTROL_INIT_CONTEXT(ComboBoxControlContext);
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    Members->Layout.MinWidth  = 7;
    Members->Layout.MinHeight = 1;
    Members->Layout.MaxHeight = 1;
    CHECK(Init(parent, "", layout, false), false, "Failed to create combo box !");

    Members->Flags                        = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
    unsigned int initialAllocatedElements = 8;
    unsigned int count                    = 0;
    
    if (std::holds_alternative<std::string_view>(text))
        count = ComputeItemsCount<char>(
              std::get<std::string_view>(text).data(), std::get<std::string_view>(text).length(), itemsSeparator);
    else 
        count = ComputeItemsCount<char8_t>(
              std::get<std::u8string_view>(text).data(), std::get<std::u8string_view>(text).length(), (char8_t)itemsSeparator);
    if (count > initialAllocatedElements)
        initialAllocatedElements = (count | 3) + 1;

    Members->Items.reserve(initialAllocatedElements);
    if (count>0)
    {
        bool result = false;
        if (std::holds_alternative<std::string_view>(text))
        {
            result = AddItemsFromList<char, std::string_view>(
                  this,
                  std::get<std::string_view>(text).data(),
                  std::get<std::string_view>(text).length(),
                  itemsSeparator);
        }
        else
        {
            result = AddItemsFromList<char8_t, std::u8string_view>(
                  this,
                  std::get<std::u8string_view>(text).data(),
                  std::get<std::u8string_view>(text).length(),
                  (char8_t) itemsSeparator);   
        }
        CHECK(result, false, "Fail to add items to ComboBox !");
    }
    Members->VisibleItems     = 1;
    Members->CurentItemIndex  = ComboBox::NO_ITEM_SELECTED;
    Members->FirstVisibleItem = 0;
    return true;
}
unsigned int ComboBox::GetItemsCount()
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, 0);
    return (unsigned int) Members->Items.size();
}
//const char* ComboBox::GetUnsafeCurrentItemText()
//{
//    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, nullptr);
//    return GetUnsafeItemText(Members->CurentItemIndex);
//}
//bool ComboBox::GetCurrentItemtext(Utils::String& itemText)
//{
//    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
//    return GetItemText(Members->CurentItemIndex, itemText);
//}
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
    return Members->Items[index].Data;
}

const AppCUI::Graphics::CharacterBuffer& ComboBox::GetCurrentItemText()
{
    __temp_comboxitem_reference_object__.Destroy();
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, __temp_comboxitem_reference_object__);
    CHECK(Members->CurentItemIndex < Members->Items.size(), __temp_comboxitem_reference_object__, "No item selected !");
    return Members->Items[Members->CurentItemIndex].Text;
}
const AppCUI::Graphics::CharacterBuffer& ComboBox::GetItemText(unsigned int index)
{
    __temp_comboxitem_reference_object__.Destroy();
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, __temp_comboxitem_reference_object__);
    CHECK_INDEX(index, __temp_comboxitem_reference_object__);
    return Members->Items[index].Text;
}
bool ComboBox::SetItemUserData(unsigned int index, ItemData userData)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    CHECK_INDEX(index, false);
    Members->Items[index].Data = userData;
    return true;
}
bool ComboBox::AddItem(const AppCUI::Utils::ConstString& caption, ItemData userData)
{
    CHECK(ComboBox_AddItem(this, caption, userData), false, "");
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    if ((Members->CurentItemIndex < 0) && (Members->Items.size() > 0))
    {
        Members->CurentItemIndex  = 0;
        Members->FirstVisibleItem = 0;
    }
    return true;
}
void ComboBox::DeleteAllItems()
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, );
    Members->Items.clear();
    Members->CurentItemIndex  = ComboBox::NO_ITEM_SELECTED;
    Members->FirstVisibleItem = 0;
}
bool ComboBox::SetCurentItemIndex(unsigned int index)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    CHECK_INDEX(index, false);
    if ((Members->CurentItemIndex == ComboBox::NO_ITEM_SELECTED) && (Members->Items.size() > 0))
        Members->CurentItemIndex = 0;
    ComboBox_MoveTo(this, index);
    return true;
}
void ComboBox::SetNoIndexSelected()
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, );
    Members->CurentItemIndex  = ComboBox::NO_ITEM_SELECTED;
    Members->FirstVisibleItem = 0;
    RaiseEvent(Event::EVENT_COMBOBOX_SELECTED_ITEM_CHANGED);
}

bool ComboBox::OnKeyEvent(AppCUI::Input::Key keyCode, char AsciiCode)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    switch (keyCode)
    {
    case Key::Up:
        if ((Members->CurentItemIndex == ComboBox::NO_ITEM_SELECTED) && (Members->Items.size() > 0))
            Members->CurentItemIndex = 0;
        else if (Members->CurentItemIndex > 0)
            ComboBox_MoveTo(this, Members->CurentItemIndex - 1);
        return true;
    case Key::Down:
        if ((Members->CurentItemIndex == ComboBox::NO_ITEM_SELECTED) && (Members->Items.size() > 0))
            Members->CurentItemIndex = 0;
        else if (Members->CurentItemIndex != ComboBox::NO_ITEM_SELECTED)
            ComboBox_MoveTo(this, Members->CurentItemIndex + 1);
        return true;
    case Key::Home:
        ComboBox_MoveTo(this, 0);
        return true;
    case Key::End:
        ComboBox_MoveTo(this, 0xFFFF);
        return true;

    case Key::Space:
    case Key::Enter:
        OnHotKey();
        return true;
    }
    return false;
}
void ComboBox::OnExpandView(AppCUI::Graphics::Clip& expandedClip)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, );
    AppCUI::Graphics::Size appSize;
    Members->VisibleItems     = 4;
    Members->HoveredIndexItem = ComboBox::NO_ITEM_SELECTED;
    if ((Application::GetApplicationSize(appSize)) && (expandedClip.ClipRect.Y >= 0))
    {
        if (appSize.Height > (unsigned int) (expandedClip.ClipRect.Y + 3))
            Members->VisibleItems = (appSize.Height - (unsigned int) (expandedClip.ClipRect.Y + 3));
    }
    if (Members->VisibleItems > Members->Items.size())
        Members->VisibleItems = (unsigned int) Members->Items.size();
    expandedClip.ClipRect.Height += Members->VisibleItems + 2;
    Members->ExpandedHeight = Members->VisibleItems + 2;
    // compute best FirstVisibleItem
    if (Members->CurentItemIndex != ComboBox::NO_ITEM_SELECTED)
    {
        if ((Members->VisibleItems / 2) <= Members->CurentItemIndex)
            Members->FirstVisibleItem = Members->CurentItemIndex - (Members->VisibleItems / 2);
        else
            Members->FirstVisibleItem = 0;
        if ((Members->FirstVisibleItem + Members->VisibleItems)>=Members->Items.size())
        {
            if (Members->VisibleItems < Members->Items.size())
                Members->FirstVisibleItem = Members->Items.size() - Members->VisibleItems;
            else
                Members->FirstVisibleItem = 0;
        }
    }
}
void ComboBox::OnPackView()
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, );
    Members->VisibleItems     = 1;
    Members->HoveredIndexItem = ComboBox::NO_ITEM_SELECTED;
    if (Members->CurentItemIndex != ComboBox::NO_ITEM_SELECTED)
        Members->FirstVisibleItem = Members->CurentItemIndex;
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

    if ((y > 1) && (y < (int) (2 + Members->VisibleItems)))
    {
        if ((Members->CurentItemIndex == ComboBox::NO_ITEM_SELECTED) && (Members->Items.size() > 0))
            Members->CurentItemIndex = 0;
        ComboBox_MoveTo(this, Members->FirstVisibleItem + y - 2); // MenuIsVisible=false;
    }
    OnHotKey();
}
bool ComboBox::OnMouseOver(int x, int y)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    if ((y > 1) && (y < (int) (2 + Members->VisibleItems)))
    {
        unsigned int newIndex = ((unsigned int) (y - 2)) + Members->FirstVisibleItem;
        if (newIndex >= Members->Items.size())
            newIndex = ComboBox::NO_ITEM_SELECTED;
            
        if (newIndex != Members->HoveredIndexItem)
        {
            Members->HoveredIndexItem = newIndex;
            return true; // repaint required
        }
    }
    else
    {
        if (ComboBox::NO_ITEM_SELECTED != Members->HoveredIndexItem)
        {
            Members->HoveredIndexItem = ComboBox::NO_ITEM_SELECTED;
            return true; // repaint required
        }
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

    auto* cbc = &Members->Cfg->ComboBox.Normal;
    if (!this->IsEnabled())
        cbc = &Members->Cfg->ComboBox.Inactive;
    if (Members->Focused)
        cbc = &Members->Cfg->ComboBox.Focus;
    else if (Members->MouseIsOver)
        cbc = &Members->Cfg->ComboBox.Hover;

    auto itemsCount = Members->Items.size();
    renderer.DrawHorizontalLine(0, 0, Members->Layout.Width - 5, ' ', cbc->Text);
    if (Members->CurentItemIndex < itemsCount)
    {
        //renderer.WriteSingleLineText(1, 0, txt, Members->Layout.Width - 6, cbc->Text);
        renderer.WriteCharacterBuffer(
              0,
              0,
              Members->Layout.Width - 6,
              Members->Items[Members->CurentItemIndex].Text,
              cbc->Text,
              TextAlignament::Left);
    }        
    renderer.WriteSingleLineText(Members->Layout.Width - 3, 0, "   ", cbc->Button);
    renderer.WriteSpecialCharacter(Members->Layout.Width - 2, 0, SpecialChars::TriangleDown, cbc->Button);

    if (Members->Flags & GATTR_EXPANDED)
    {
        renderer.FillRect(
              0, 1, Members->Layout.Width - 1, Members->ExpandedHeight, ' ', Members->Cfg->ComboBox.Focus.Text);
        renderer.DrawRect(
              0, 1, Members->Layout.Width - 1, Members->ExpandedHeight, Members->Cfg->ComboBox.Focus.Text, false);
        for (unsigned int tr = 0; tr < Members->VisibleItems; tr++)
        {
            if ((tr + Members->FirstVisibleItem)<itemsCount)
                renderer.WriteCharacterBuffer(
                      1,
                      tr+2,
                      Members->Layout.Width - 2,
                      Members->Items[tr + Members->FirstVisibleItem].Text,
                      Members->Cfg->ComboBox.Focus.Text,
                      TextAlignament::Left);
            //renderer.WriteSingleLineText(
            //      2,
            //      tr + 2,
            //      GetUnsafeItemText(tr + Members->FirstVisibleItem),
            //      Members->Layout.Width - 2,
            //      Members->Cfg->ComboBox.Focus.Text,
            //      TextAlignament::Left);
            if ((tr + Members->FirstVisibleItem) == Members->CurentItemIndex)
                renderer.DrawHorizontalLine(1, tr + 2, Members->Layout.Width - 2, -1, Members->Cfg->ComboBox.Selection);
            else if ((tr + Members->FirstVisibleItem) == Members->HoveredIndexItem)
                renderer.DrawHorizontalLine(1, tr + 2, Members->Layout.Width - 2, -1, Members->Cfg->ComboBox.HoverOveItem);
        }
    }
}
