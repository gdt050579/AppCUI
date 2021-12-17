#include "ControlContext.hpp"
#include <set>

namespace AppCUI
{
constexpr uint32 CATEGORY_FLAG             = 0x80000000;
constexpr uint32 CATEGORY_INDEX_MASK       = 0x7FFFFFFF;
constexpr static string_view color_names[] = {
    "Black", "DarkBlue", "DarkGreen", "Teal", "DarkRed", "Magenta", "Olive", "Silver",      "Gray",
    "Blue",  "Green",    "Aqua",      "Red",  "Pink",    "Yellow",  "White", "Transparent",
};
int32 SortWithCategories(int32 i1, int32 i2, void* context)
{
    auto* PC = reinterpret_cast<PropertyListContext*>(context);
    int32 result;
    const auto& p1 = PC->properties[i1];
    const auto& p2 = PC->properties[i2];
    const auto* c1 = PC->categories[p1.category].name.GetText();
    const auto* c2 = PC->categories[p2.category].name.GetText();
    result         = String::Compare(c1, c2, true);
    if (result != 0)
        return result;
    return String::Compare(p1.name.GetText(), p2.name.GetText(), true);
}
int32 SortWithoutCategories(int32 i1, int32 i2, void* context)
{
    auto* PC       = reinterpret_cast<PropertyListContext*>(context);
    const auto& p1 = PC->properties[i1];
    const auto& p2 = PC->properties[i2];

    return String::Compare(p1.name.GetText(), p2.name.GetText(), true);
}
void PropertyListContext::DrawCategory(uint32 index, int32 y, Graphics::Renderer& renderer)
{
    if (index >= this->categories.size())
        return;
    const auto& cat = this->categories[index];
    int32 x         = this->hasBorder ? 1 : 0;
    int32 w         = this->hasBorder ? this->Layout.Width - 2 : this->Layout.Width;

    renderer.FillHorizontalLine(x, y, w, ' ', Colors.Category.Text);

    WriteTextParams params(
          WriteTextFlags::OverwriteColors | WriteTextFlags::SingleLine | WriteTextFlags::FitTextToWidth);
    params.Y = y;
    if (w >= 5)
    {
        params.X     = x + 2;
        params.Color = Colors.Category.Text;
        params.Width = w - 5;
        renderer.WriteText(cat.name, params);
    }

    LocalString<32> tmp;
    if (cat.filteredItems < cat.totalItems)
    {
        tmp.SetFormat("[%u/%u]", cat.filteredItems, cat.totalItems);
    }
    else
    {
        tmp.SetFormat("[%u]", cat.totalItems);
    }
    if ((int32) tmp.Len() + 5 <= w)
    {
        params.Align = TextAlignament::Right;
        params.Width = tmp.Len();
        params.X     = w - 1;
        params.Color = Colors.Category.Stats;
        renderer.WriteText(tmp, params);
    }
    if (w > 0)
        renderer.WriteSpecialCharacter(
              x, y, cat.folded ? SpecialChars::TriangleRight : SpecialChars::TriangleDown, Colors.Category.Arrow);
}
void PropertyListContext::DrawProperty(uint32 index, int32 y, Graphics::Renderer& renderer)
{
    if (index >= this->properties.size())
        return;
    const auto& prop = this->properties[index];

    int32 x = this->hasBorder ? 1 : 0;
    NumericFormatter n;
    WriteTextParams params(WriteTextFlags::OverwriteColors | WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth);
    if ((x + 3 + this->propertyNameWidth) < (int32) this->Layout.Width)
    {
        params.X     = x + 3;
        params.Y     = y;
        params.Color = Colors.Item.Text;
        params.Width = this->propertyNameWidth;
        renderer.WriteText(prop.name, params);
        renderer.WriteSpecialCharacter(
              x + 3 + this->propertyNameWidth, y, SpecialChars::BoxVerticalSingleLine, Colors.Item.LineSeparator);
    }
    if (this->object->GetPropertyValue(prop.id, tempPropValue))
    {
        string_view tmpAscii;
        NumericFormatter n;
        switch (prop.type)
        {
        case PropertyType::UInt8:
            tmpAscii = n.ToDec(std::get<uint8>(tempPropValue));
            break;
        case PropertyType::UInt16:
            tmpAscii = n.ToDec(std::get<uint16>(tempPropValue));
            break;
        case PropertyType::UInt32:
            tmpAscii = n.ToDec(std::get<uint32>(tempPropValue));
            break;
        case PropertyType::UInt64:
            tmpAscii = n.ToDec(std::get<uint64>(tempPropValue));
            break;
        case PropertyType::Int8:
            tmpAscii = n.ToDec(std::get<int8>(tempPropValue));
            break;
        case PropertyType::Int16:
            tmpAscii = n.ToDec(std::get<int16>(tempPropValue));
            break;
        case PropertyType::Int32:
            tmpAscii = n.ToDec(std::get<int32>(tempPropValue));
            break;
        case PropertyType::Int64:
            tmpAscii = n.ToDec(std::get<int64>(tempPropValue));
            break;
        case PropertyType::Ascii:
            tmpAscii = std::get<string_view>(tempPropValue);
            break;
        case PropertyType::Color:
            tmpAscii = color_names[static_cast<uint8>(std::get<Graphics::Color>(tempPropValue))];
            break;
        case PropertyType::Key:
            tmpAscii = KeyUtils::GetKeyName(std::get<Input::Key>(tempPropValue));
            break;
        default:
            tmpAscii = "<error>";
            break;
        }
        params.X     = x + 4 + this->propertyNameWidth;
        params.Y     = y;
        params.Color = Colors.Item.Value;
        params.Width = (int32)this->Layout.Width - (x + 3 + this->propertyNameWidth);
        params.Flags = WriteTextFlags::OverwriteColors | WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth;
        renderer.WriteText(tmpAscii, params);
    }
}
void PropertyListContext::Paint(Graphics::Renderer& renderer)
{
    uint32 value;
    int32 y     = 1;
    int32 max_y = this->Layout.Height;
    auto c      = this->Cfg->PropertList.Border;

    if ((this->Flags & GATTR_ENABLE) == 0)
    {
        c                               = this->Cfg->PropertList.Inactive;
        this->Colors.Category.Arrow     = c;
        this->Colors.Category.Stats     = c;
        this->Colors.Category.Text      = c;
        this->Colors.Item.LineSeparator = c;
        this->Colors.Item.ReadOnly      = c;
        this->Colors.Item.Text          = c;
        this->Colors.Item.Value         = c;
    }
    else
    {
        this->Colors.Category = this->Cfg->PropertList.Category;
        this->Colors.Item     = this->Cfg->PropertList.Item;
    }
    renderer.Clear(' ', c);
    if (this->hasBorder)
    {
        renderer.DrawRectSize(0, 0, this->Layout.Width, this->Layout.Height, c, false);
        y++;
        max_y--;
    }
    for (auto idx = this->startView; (idx < this->items.Len()) && (y < max_y); idx++, y++)
    {
        if (this->items.Get(idx, value) == false)
            break;
        if (value & CATEGORY_FLAG)
            DrawCategory(value & CATEGORY_INDEX_MASK, y, renderer);
        else
            DrawProperty(value, y, renderer);
        if ((this->Focused) && (idx == this->currentPos))
        {
            if (this->hasBorder)
                renderer.FillHorizontalLine(1, y, this->Layout.Width - 2, -1, ColorPair{ Color::Black, Color::White });
            else
                renderer.FillHorizontalLine(0, y, this->Layout.Width, -1, ColorPair{ Color::Black, Color::White });
        }
    }
}
void PropertyListContext::MoveTo(uint32 newPos)
{
    if (this->items.Len() == 0)
    {
        this->currentPos = 0;
        this->startView  = 0;
        return;
    }
    if (newPos >= this->items.Len())
        newPos = this->items.Len() - 1;
    auto h = this->hasBorder ? this->Layout.Height - 3 : this->Layout.Height - 1;
    if (h < 1)
        return; // sanity check
    uint32 height = (uint32) h;
    if ((startView <= newPos) && ((startView + height) > newPos))
    {
        this->currentPos = newPos;
        return;
    }
    // adjust start view --> if before scroll up
    if (newPos < startView)
    {
        this->currentPos = newPos;
        this->startView  = newPos;
        return;
    }
    // otherwise scroll down
    this->currentPos = newPos;
    if (newPos >= startView + height - 1)
        this->startView = newPos - (height - 1);
    else
        this->startView = 0;
}
bool PropertyListContext::OnKeyEvent(Input::Key keyCode, char16 UnicodeChar)
{
    auto h = this->hasBorder ? this->Layout.Height - 3 : this->Layout.Height - 1;
    uint32 idx;

    switch (keyCode)
    {
    case Key::Up:
        if (this->currentPos > 0)
            MoveTo(this->currentPos - 1);
        return true;
    case Key::Down:
        if (this->currentPos + 1 < this->items.Len())
            MoveTo(this->currentPos + 1);
        return true;
    case Key::Home:
        MoveTo(0);
        return true;
    case Key::End:
        MoveTo(this->items.Len());
        return true;
    case Key::PageUp:
        if ((h >= 1) && (((uint32) h) > this->currentPos))
            MoveTo(this->currentPos - (uint32) h);
        else
            MoveTo(0);
        return true;
    case Key::PageDown:
        MoveTo(this->currentPos + (uint32) std::max<int>(1, h));
        return true;
    case Key::Space:
        if (this->items.Get(this->currentPos, idx))
        {
            if (idx & CATEGORY_FLAG)
            {
                idx &= CATEGORY_INDEX_MASK;
                this->categories[idx].folded = !this->categories[idx].folded;
                Refilter();
            }
        }
        return true;
    }
    return false;
}
bool PropertyListContext::IsItemFiltered(const PropertyInfo& p)
{
    if (this->filterText.Empty())
        return true;
    return String::Contains(p.name.GetText(), this->filterText.GetText(), true);
}
void PropertyListContext::Refilter()
{
    this->items.Clear();
    for (uint32 i = 0U; i < this->properties.size(); i++)
    {
        if (IsItemFiltered(this->properties[i]))
            this->items.Push(i);
    }
    // sort the data
    if (this->showCategories)
    {
        this->items.Sort(SortWithCategories, true, this);
        // clear categories filtered items
        for (auto& cat : this->categories)
            cat.filteredItems = 0;
        // insert categories
        uint32 idx      = 0;
        uint32 value    = 0;
        uint32 last_cat = 0xFFFFFFFF;
        while (idx < this->items.Len())
        {
            if (this->items.Get(idx, value))
            {
                if (this->properties[value].category != last_cat)
                {
                    last_cat = this->properties[value].category;
                    this->items.Insert(idx, last_cat | CATEGORY_FLAG);
                }
                this->categories[this->properties[value].category].filteredItems++;
            }
            idx++;
        }
        // fold categories if case
        idx = 0;
        while (idx < this->items.Len())
        {
            if ((this->items.Get(idx, value)) && (value & CATEGORY_FLAG) &&
                (this->categories[value & CATEGORY_INDEX_MASK].folded))
            {
                // search for the next category
                uint32 next = idx + 1;
                while ((next < this->items.Len()) && (this->items.Get(next, value)) && ((value & CATEGORY_FLAG) == 0))
                    next++;
                if (next > (idx + 1))
                {
                    this->items.Delete(idx + 1, (next - (idx + 1)));
                }
            }
            idx++;
        }
    }
    else
    {
        this->items.Sort(SortWithoutCategories, true, this);
    }
}

PropertyList::PropertyList(string_view layout, Reference<PropertiesInterface> obj)
    : Control(new PropertyListContext(), "", layout, false)
{
    auto* Members = (PropertyListContext*) this->Context;
    Members->properties.reserve(64);
    Members->categories.reserve(8);
    Members->showCategories    = true;
    Members->hasBorder         = true;
    Members->propertyNameWidth = 14;
    Members->startView         = 0;
    Members->currentPos        = 0;
    Members->Flags             = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
    Members->Layout.MinWidth   = 7;
    Members->Layout.MinHeight  = 4;
    
    SetObject(obj);
}
void PropertyList::SetObject(Reference<PropertiesInterface> obj)
{
    auto* Members   = (PropertyListContext*) this->Context;
    Members->object = obj;
    Members->categories.clear();
    Members->properties.clear();

    if (obj.IsValid())
    {
        std::map<string_view, uint32> s;
        // items
        for (auto& e : obj->GetPropertiesList())
        {
            auto& pi = Members->properties.emplace_back();
            pi.name  = e.name;
            pi.type  = e.type;
            pi.id    = e.id;
            auto it  = s.find(e.category);
            if (it != s.cend())
            {
                pi.category = it->second;
                Members->categories[pi.category].totalItems++;
            }
            else
            {
                pi.category   = (uint32) Members->categories.size();
                s[e.category] = pi.category;
                // add categoy
                auto& cat         = Members->categories.emplace_back();
                cat.filteredItems = 0; // it will be recomputed on Members->Refilter()
                cat.totalItems    = 1;
                cat.folded        = false;
                cat.name          = e.category;
            }
        }
    }

    Members->items.Resize((uint32) Members->properties.size() * 2); // assume that each item has its own category
    Members->Refilter();
}
PropertyList::~PropertyList()
{
}
void PropertyList::Paint(Graphics::Renderer& renderer)
{
    reinterpret_cast<PropertyListContext*>(this->Context)->Paint(renderer);
}
bool PropertyList::OnKeyEvent(Input::Key keyCode, char16 UnicodeChar)
{
    return reinterpret_cast<PropertyListContext*>(this->Context)->OnKeyEvent(keyCode, UnicodeChar);
}
void PropertyList::OnMouseReleased(int x, int y, Input::MouseButton button)
{
}
void PropertyList::OnMousePressed(int x, int y, Input::MouseButton button)
{
}
bool PropertyList::OnMouseDrag(int x, int y, Input::MouseButton button)
{
    NOT_IMPLEMENTED(false);
}
bool PropertyList::OnMouseWheel(int x, int y, Input::MouseWheel direction)
{
    NOT_IMPLEMENTED(false);
}
bool PropertyList::OnMouseOver(int x, int y)
{
    NOT_IMPLEMENTED(false);
}
bool PropertyList::OnMouseLeave()
{
    NOT_IMPLEMENTED(false);
}

void PropertyList::OnUpdateScrollBars()
{
}

} // namespace AppCUI