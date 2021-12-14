#include "ControlContext.hpp"

namespace AppCUI
{
constexpr uint32 CATEGORY_MASK = 0x80000000;
int32 SortWithCategories(int32 i1, int32 i2, void* context)
{
    auto* PC = reinterpret_cast<PropertyListContext*>(context);
    int32 result;
    const auto& p1 = PC->properties[i1];
    const auto& p2 = PC->properties[i2];
    result         = p1.category.compare(p2.category);
    if (result != 0)
        return result;
    return p1.name.compare(p2.name);
}
bool PropertyListContext::IsItemFiltered(const Property& p)
{
    NOT_IMPLEMENTED(false);
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
    this->items.Sort(SortWithCategories, this);
    // insert categories if case
    uint32 idx = 0;
    string_view last_cat;
    while (idx < this->items.Len())
    {
        if (this->properties[idx].category != last_cat)
        {
            last_cat = this->properties[idx].category;
            this->items.Insert(idx, idx | CATEGORY_MASK);
        }
        idx++;
    }
}

PropertyList::PropertyList(string_view layout, Reference<PropertiesInterface> obj)
    : Control(new PropertyListContext(), "", layout, false)
{
    SetObject(obj);
}
void PropertyList::SetObject(Reference<PropertiesInterface> obj)
{
    auto* Members   = (PropertyListContext*) this->Context;
    Members->object = obj;
    if (obj.IsValid())
    {
        Members->properties = std::move<>(obj->GetPropertiesList());
    }
    else
    {
        Members->properties.clear();
    }
    Members->items.Resize((uint32) Members->properties.size() * 2); // assume that each item has its own category
    Members->Refilter();
    ;
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