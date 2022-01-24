#include "AppCUI.hpp"

using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;

namespace AppCUI::Dialogs
{
constexpr int BUTTON_CMD_CLOSE = 1;
enum class PropID : uint32
{
    DesktopChar,
    DesktopColor
};
class ConfigProperty : public PropertiesInterface
{
    AppCUI::Application::Config obj;

  public:
    ConfigProperty(const AppCUI::Application::Config& config) : obj(config)
    {
    }
    void PaintMenusAndCommandBar(Graphics::Renderer& r, Size sz)
    {
        r.FillHorizontalLine(0, 0, (int) sz.Width, ' ', obj.Menu.Text.Normal);
        r.WriteSingleLineText(1, 0, " File ", obj.Menu.Text.PressedOrSelected, obj.Menu.HotKey.PressedOrSelected, 1);
        r.WriteSingleLineText(7, 0, " View ", obj.Menu.Text.Normal, obj.Menu.HotKey.Normal, 8);
        r.WriteSingleLineText(14, 0, " Help ", obj.Menu.Text.Hovered, obj.Menu.HotKey.Hovered, 15);

        auto y = ((int) (sz.Height)) - 1;
        r.FillHorizontalLine(0, y, (int) sz.Width, ' ', obj.Menu.Text.Normal);
        r.WriteSingleLineText(0, y, " Alt+ ", obj.Menu.Text.Inactive);
        r.WriteSingleLineText(7, y, " F1 ", obj.Menu.ShortCut.PressedOrSelected);
        r.WriteSingleLineText(11, y, "Run ", obj.Menu.Text.PressedOrSelected);
        r.WriteSingleLineText(15, y, " F2 ", obj.Menu.ShortCut.Normal);
        r.WriteSingleLineText(19, y, "Quit ", obj.Menu.Text.Normal);
        r.WriteSingleLineText(24, y, " F3 ", obj.Menu.ShortCut.Hovered);
        r.WriteSingleLineText(28, y, "Reload ", obj.Menu.Text.Hovered);
    }
    void Paint(Graphics::Renderer& r, Size sz)
    {
        r.ClearWithSpecialChar(SpecialChars::Block50, obj.Symbol.Desktop);
        PaintMenusAndCommandBar(r, sz);
    }
    bool GetPropertyValue(uint32 propertyID, PropertyValue& value) override
    {
        switch (static_cast<PropID>(propertyID))
        {
        case PropID::DesktopChar:
            value = (char16)186;
            return true;
        case PropID::DesktopColor:
            value = obj.Symbol.Desktop;
            return true;
        }
        return false;
    }
    bool SetPropertyValue(uint32 propertyID, const PropertyValue& value, String& error) override
    {
        switch (static_cast<PropID>(propertyID))
        {
        case PropID::DesktopColor:
            obj.Symbol.Desktop = std::get<ColorPair>(value);
            return true;       
        }
        error.SetFormat("Invalid property id (%d)", propertyID);
        return false;
    }
    void SetCustomPropertyValue(uint32 propertyID) override
    {
    }
    bool IsPropertyValueReadOnly(uint32 propertyID) override
    {
        NOT_IMPLEMENTED(false);
    }
    const vector<Property> GetPropertiesList() override
    {
#define PT(t) static_cast<uint32>(t)
        return { { PT(PropID::DesktopChar), "Desktop", "Symbol", PropertyType::Char16 },
                 { PT(PropID::DesktopColor), "Desktop", "Color", PropertyType::ColorPair } };
#undef PT
    };
};
class PreviewControl : public UserControl
{
    Reference<ConfigProperty> config;

  public:
    PreviewControl() : UserControl("d:c")
    {
    }
    void Paint(Graphics::Renderer& r) override
    {
        Size sz;
        GetSize(sz);
        if (config.IsValid())
            config->Paint(r, sz);
    }
    void SetConfig(Reference<ConfigProperty> _config)
    {
        config = _config;
    }
};
class ThemeEditorDialog : public Window
{
    Reference<PropertyList> prop;
    ConfigProperty cfg;
    Reference<PreviewControl> pc;

  public:
    ThemeEditorDialog(const AppCUI::Application::Config& configObject)
        : Window("Theme editor", "d:c,w:80,h:24", WindowFlags::Sizeable), cfg(configObject)
    {
        auto sp = Factory::Splitter::Create(this, "l:0,t:0,b:3,r:0", true);
        sp->SetSecondPanelSize(30);        
        pc   = sp->CreateChildControl<PreviewControl>();
        pc->SetConfig(&cfg);
        prop = Factory::PropertyList::Create(sp, "d:c", &cfg, PropertyListFlags::None);
        Factory::Button::Create(this, "&Close", "r:1,b:0,w:12", BUTTON_CMD_CLOSE);
    }
    bool OnEvent(Reference<Control> control, Event eventType, int ID) override
    {
        if (Window::OnEvent(control, eventType, ID))
            return true;
        if (eventType == Event::ButtonClicked)
        {
            switch (ID)
            {
            case BUTTON_CMD_CLOSE:
                this->Exit(0);
                return true;
            }
        }
        return false;
    }
};

void ThemeEditor::Show()
{
    auto* cfg = Application::GetAppConfig();
    if (cfg)
    {
        ThemeEditorDialog dlg(*cfg);
        dlg.Show();
    }
}
} // namespace AppCUI::Dialogs