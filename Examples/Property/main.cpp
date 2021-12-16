#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;

enum class MyControlProperty : uint32
{
    X,
    Y,
    Size,
    ForeColor,
    BackColor,
    Character,
    Name,
    Version,
    Border,
    BorderType,
};

class MyUserControl : public UserControl, public PropertiesInterface
{
    int32 x, y;
    Size sz;
    char16 ch;
    ColorPair c;
    bool hasBorder;

  public:
    MyUserControl() : UserControl("d:c")
    {
    }

    void Paint(Graphics::Renderer& renderer) override
    {
        renderer.Clear(' ', ColorPair{ Color::White, Color::Black });
        renderer.WriteSingleLineText(0, 0, "My user control", ColorPair{ Color::White, Color::Black });
    }
    // PropertiesInterface
    bool GetPropertyValue(uint32 id, PropertyValue& value) override
    {
        switch (static_cast<MyControlProperty>(id))
        {
        case MyControlProperty::X:
            value = this->x;
            return true;
        case MyControlProperty::Y:
            value = this->y;
            return true;
        case MyControlProperty::Size:
            value = this->sz;
            return true;
        case MyControlProperty::ForeColor:
            value = this->c.Foreground;
            return true;
        case MyControlProperty::BackColor:
            value = this->c.Background;
            return true;
        case MyControlProperty::Name:
            value = "My user control";
            return true;
        case MyControlProperty::Version:
            value = "1.2.3";
            return true;
        case MyControlProperty::Border:
            value = this->hasBorder;
            return true;
        case MyControlProperty::BorderType:
            value = 2;
            return true;
        }
        return false;
    };
    void SetPropertyValue(uint32 id, const PropertyValue& value, String& error) override
    {
        NOT_IMPLEMENTED(false);
    };
    vector<Property> GetPropertiesList() override
    {
        return vector<Property>({
              { (uint32) MyControlProperty::X, "Layout", "X", PropertyType::SignedInteger },
              { (uint32) MyControlProperty::Y, "Layout", "Y", PropertyType::SignedInteger },
              { (uint32) MyControlProperty::Size, "Layout", "Size", PropertyType::Size },
              { (uint32) MyControlProperty::ForeColor, "Look & Fill", "Fore color", PropertyType::Color },
              { (uint32) MyControlProperty::BackColor, "Look & Fill", "Back color", PropertyType::Color },
              { (uint32) MyControlProperty::Character, "Look & Fill", "Character", PropertyType::String },
              { (uint32) MyControlProperty::Name, "General", "Name", PropertyType::String },
              { (uint32) MyControlProperty::Version, "General", "Version", PropertyType::String },
              { (uint32) MyControlProperty::Border, "General", "Draw border", PropertyType::Boolean },
              { (uint32) MyControlProperty::BorderType, "General", "Border Type", "SingleLine=1,DoubleLine=2,Block=3" },
        });
    };
};
class PropertyWindowExmaple : public Window
{
  public:
    PropertyWindowExmaple() : Window("Example", "d:c,w:60,h:20", WindowFlags::Sizeable)
    {
        auto sp = Factory::Splitter::Create(this, "d:c", true);
        auto ct = sp->CreateChildControl<MyUserControl>();
        auto pl = sp->CreateChildControl<PropertyList>("d:c", ct.UpCast<PropertiesInterface>());
        sp->SetSecondPanelSize(30);
    }
};
int main()
{
    if (!Application::Init())
        return 1;
    Application::AddWindow(std::make_unique<PropertyWindowExmaple>());
    Application::Run();
    return 0;
}
