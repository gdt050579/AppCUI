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

class ListItemsParser
{
    const char16* start;
    const char16* end;

    inline const char16* SkipSpaces(const char16* pos) const
    {
        while ((pos < end) && (((*pos) == ' ') || ((*pos) == '\t') || ((*pos) == '\n') || ((*pos) == '\r')))
            pos++;
        return pos;
    }
    inline const char16* SkipWord(const char16* pos, char c1, char c2) const
    {
        auto st = pos;
        while ((pos < end) && ((*pos) != c1) && ((*pos) != c2))
            pos++;
        if (pos < end) // I've reached either a `c1` or `c2` character
        {
            // go back and skip spaces/tabs or new lines
            pos--;
            while ((pos > st) && (((*pos) == ' ') || ((*pos) == '\t') || ((*pos) == '\n') || ((*pos) == '\r')))
                pos--;
            pos++; // next char
        }

        return pos;
    }
    inline bool CheckNextChar(const char16* pos, char c1, char c2, bool failIfEndOfBuffer) const
    {
        if ((pos < end) && (((*pos) == c1) || ((*pos) == c2)))
            return true;
        if ((pos >= end) && (!failIfEndOfBuffer))
            return true;
        return false;
    }

  public:
    ListItemsParser(const char16* _start, const char16* _end) : start(_start), end(_end)
    {
    }
    bool Create(std::map<uint64, FixSizeUnicode<48>>& result)
    {
        const char16* k_start;
        const char16* k_end;
        const char16* v_start;
        const char16* v_end;
        const char16* p = start;
        char asciiValue[128];
        char* v_ascii;

        CHECK(p, false, "Expecting a valid NON-NULL buffer for ListItemParser !");

        while (p < end)
        {
            k_start = SkipSpaces(p);
            k_end   = SkipWord(k_start, '=', '=');
            p       = SkipSpaces(k_end);
            CHECK(CheckNextChar(p, '=', '=', true), false, "");
            v_start = SkipSpaces(p + 1);
            v_end   = SkipWord(v_start, ',',';');
            p       = SkipSpaces(v_end);
            CHECK(CheckNextChar(p, ',', ';', false), false, "");
            p = SkipSpaces(p + 1);
            CHECK(v_start < v_end, false, "Expecting a value in list");
            CHECK(k_start < k_end, false, "Expecting a key in list");
            CHECK((v_end - v_start) < 127, false, "Numbers in list must not exceed 126 characters");

            v_ascii = asciiValue;
            while (v_start < v_end)
            {
                CHECK((((*v_start) > 32) && ((*v_start) < 127)),
                      false,
                      "Invalid character representation for a number");
                *v_ascii = (char) (*v_start);
                v_ascii++;
                v_start++;
            }
            string_view value((const char*) asciiValue, (size_t) (v_ascii - asciiValue));
            auto res = Number::ToUInt64(value);
            CHECK(res.has_value(), false, "Invalid value (or invalid uint64 value) in list");
            result[res.value()] = u16string_view{ k_start, (size_t) (k_end - k_start) };
        }
        return true;
    }
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
    bool readOnly = this->Flags && PropertyListFlags::ReadOnly ? true : this->object->IsPropertyValueReadOnly(prop.id);
    auto w        = this->hasBorder ? ((int32) this->Layout.Width - (x + 5 + this->propertyNameWidth))
                                    : ((int32) this->Layout.Width - (x + 4 + this->propertyNameWidth));
    params.X      = x + 4 + this->propertyNameWidth;
    params.Y      = y;
    params.Color  = readOnly ? Colors.Item.ReadOnly : Colors.Item.Value;
    params.Flags  = WriteTextFlags::OverwriteColors | WriteTextFlags::SingleLine | WriteTextFlags::FitTextToWidth;

    if (this->object->GetPropertyValue(prop.id, tempPropValue))
    {
        string_view tmpAscii;
        u16string_view tmpUnicode;
        u8string_view tmpUTF8;
        CharacterView tmpCharView;
        NumericFormatter n;
        LocalString<32> tmpString;
        Size tmpSize;
        switch (prop.type)
        {
        case PropertyType::Boolean:
            tmpAscii = std::get<bool>(tempPropValue) ? "(Yes)" : "(No)";
            break;
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
        case PropertyType::Float:
            tmpAscii = n.ToDec(std::get<float>(tempPropValue));
            break;
        case PropertyType::Double:
            tmpAscii = n.ToDec(std::get<double>(tempPropValue));
            break;
        case PropertyType::Ascii:
            tmpAscii = std::get<string_view>(tempPropValue);
            break;
        case PropertyType::Unicode:
            tmpUnicode = std::get<u16string_view>(tempPropValue);
            break;
        case PropertyType::UTF8:
            tmpUTF8 = std::get<u8string_view>(tempPropValue);
            break;
        case PropertyType::CharacterView:
            tmpCharView = std::get<CharacterView>(tempPropValue);
            break;
        case PropertyType::Color:
            tmpAscii = color_names[static_cast<uint8>(std::get<Graphics::Color>(tempPropValue))];
            break;
        case PropertyType::Key:
            tmpAscii = KeyUtils::GetKeyName(std::get<Input::Key>(tempPropValue));
            break;
        case PropertyType::Size:
            tmpSize  = std::get<Graphics::Size>(tempPropValue);
            tmpAscii = tmpString.Format("%u x %u", tmpSize.Width, tmpSize.Height);
            break;
        }

        if (w > 0)
        {
            params.Width = (uint32) w;

            switch (prop.type)
            {
            case PropertyType::Boolean:
                if (std::get<bool>(tempPropValue))
                    renderer.WriteSpecialCharacter(params.X, y, SpecialChars::CheckMark, Colors.Item.Checked);
                else
                    renderer.WriteCharacter(params.X, y, 'x', Colors.Item.Unchecked);
                params.X += 2;
                if (w > 2)
                {
                    params.Width -= 2;
                    renderer.WriteText(tmpAscii, params);
                }
                break;
            case PropertyType::UInt8:
            case PropertyType::UInt16:
            case PropertyType::UInt32:
            case PropertyType::UInt64:
            case PropertyType::Int8:
            case PropertyType::Int16:
            case PropertyType::Int32:
            case PropertyType::Int64:
            case PropertyType::Float:
            case PropertyType::Double:
            case PropertyType::Ascii:
            case PropertyType::Key:
            case PropertyType::Size:
                // value is already converted to ascii string --> printed
                renderer.WriteText(tmpAscii, params);
                break;
            case PropertyType::Unicode:
                renderer.WriteText(tmpUnicode, params);
                break;
            case PropertyType::UTF8:
                renderer.WriteText(tmpUTF8, params);
                break;
            case PropertyType::CharacterView:
                renderer.WriteText(tmpCharView, params);
                break;
            case PropertyType::Color:
                renderer.WriteSpecialCharacter(
                      params.X,
                      y,
                      SpecialChars::BlockCentered,
                      ColorPair{ std::get<Graphics::Color>(tempPropValue), Color::Transparent });
                params.X += 2;
                if (w > 2)
                {
                    params.Width -= 2;
                    renderer.WriteText(tmpAscii, params);
                }
                break;
            default:
                renderer.WriteText("<Internal error - fail to process item type>", params);
                break;
            }
        }
    }
    else
    {
        if (w > 0)
        {
            params.Width = (uint32) w;
            params.Color = Colors.Item.Error;
            renderer.WriteText("<Unable to read item value>", params);
        }
    }
}
void PropertyListContext::Paint(Graphics::Renderer& renderer)
{
    uint32 value;
    int32 y     = 1;
    int32 max_y = this->Layout.Height;
    auto c      = this->Cfg->PropertList.Border;

    if (this->propertyNameWidth == 0)
        this->SetPropertyNameWidth(this->Layout.Width * 4 / 10, false);

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
        this->Colors.Item.Checked       = c;
        this->Colors.Item.Unchecked     = c;
        this->Colors.Item.Error         = c;
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
void PropertyListContext::SetPropertyNameWidth(int32 value, bool adjustPercentage)
{
    if (this->hasBorder)
        value = std::min<>(this->Layout.Width - 8, value);
    else
        value = std::min<>(this->Layout.Width - 6, value);
    this->propertyNameWidth = std::max<>(2, value);
    if (adjustPercentage)
        this->propetyNamePercentage = (float) this->propertyNameWidth / (float) (this->Layout.Width);
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
    case Key::Left:
        SetPropertyNameWidth(this->propertyNameWidth - 1, true);
        return true;
    case Key::Right:
        SetPropertyNameWidth(this->propertyNameWidth + 1, true);
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
        // clear categories filtered items;
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

PropertyList::PropertyList(string_view layout, Reference<PropertiesInterface> obj, PropertyListFlags flags)
    : Control(new PropertyListContext(), "", layout, false)
{
    auto* Members = (PropertyListContext*) this->Context;
    Members->properties.reserve(64);
    Members->categories.reserve(8);
    Members->showCategories        = true;
    Members->hasBorder             = (flags & PropertyListFlags::Border) != PropertyListFlags::None;
    Members->propertyNameWidth     = 0;
    Members->startView             = 0;
    Members->currentPos            = 0;
    Members->Flags                 = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | (uint32) flags;
    Members->Layout.MinWidth       = 10; // 3 spaces+2chars(name)+1char(bar)+2chars(value)+2chars(border)
    Members->Layout.MinHeight      = 4;
    Members->propetyNamePercentage = 0.4f; // 40% of width is the property name

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
            if ((pi.type == PropertyType::Flags) || (pi.type == PropertyType::List))
            {
                LocalUnicodeStringBuilder<1024> tempValues(e.values);
                // we need to process list of flags/values
                ListItemsParser parser(tempValues.GetString(), tempValues.GetString() + tempValues.Len());
                if (parser.Create(pi.listValues) == false)
                {
                    // clear all data
                    pi.listValues.clear();
                }
            }
            auto it = s.find(e.category);
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

void PropertyList::OnAfterResize(int newWidth, int)
{
    auto* Members = (PropertyListContext*) this->Context;
    if (Members->propertyNameWidth == 0)
        Members->SetPropertyNameWidth((int32) (newWidth * Members->propetyNamePercentage), false);
    else
        Members->SetPropertyNameWidth((int32) (newWidth * Members->propetyNamePercentage), false);
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