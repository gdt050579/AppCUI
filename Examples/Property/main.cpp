#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;

class MyUserControl : public UserControl, public PropertiesInterface
{
    int32 x, y;
    Size sz;
    char16 ch;
    ColorPair c;

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
    bool GetProperty(string_view category, string_view name) override
    {
        NOT_IMPLEMENTED(false);
    };
    bool SetProperty(string_view category, string_view name) override
    {
        NOT_IMPLEMENTED(false);
    };
    vector<Property> GetPropertiesList() override
    {
        return vector<Property>({
              { "Layout", "X", PropertyType::SignedInteger},
              { "Layout", "Y", PropertyType::SignedInteger },
              { "Layout", "Size", PropertyType::Size },
              { "Look & Fill", "Fore color", PropertyType::Color },
              { "Look & Fill", "Back color", PropertyType::Color },
              { "Look & Fill", "Character", PropertyType::String },
              { "General", "Name", PropertyType::String },
              { "General", "Version", PropertyType::String },
              { "General", "Draw border", PropertyType::Boolean },
              { "General", "Border Type", "SingleLine=1,DoubleLine=2,Block=3" },
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
