#include "ControlContext.hpp"
#include <set>

namespace AppCUI
{
constexpr uint32 CATEGORY_FLAG       = 0x80000000;
constexpr uint32 CATEGORY_INDEX_MASK = 0x7FFFFFFF;

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
    Members->showCategories = true;

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
            auto it  = s.find(e.category);
            if (it != s.cend())
            {
                pi.category = it->second;
                Members->categories[pi.category].totalItems++;
            }
            else
            {
                pi.category   = (uint32)Members->categories.size();
                s[e.category] = pi.category;
                // add categoy
                auto& cat = Members->categories.emplace_back();
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
}
bool PropertyList::OnKeyEvent(Input::Key keyCode, char16 UnicodeChar)
{
    NOT_IMPLEMENTED(false);
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