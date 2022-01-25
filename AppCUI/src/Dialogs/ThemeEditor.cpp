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

        r.FillRect(0, 1, 20, 9, ' ', obj.Menu.Text.Normal);
        r.DrawRect(0, 1, 20, 9, obj.Menu.Text.Normal, LineType::Single);
        // item Save
        r.WriteSingleLineText(2, 2, "Save", obj.Menu.Text.Normal);
        r.WriteCharacter(2, 2, 'S', obj.Menu.HotKey.Normal);
        r.WriteSingleLineText(18, 2, "Ctrl+S", obj.Menu.ShortCut.Normal, TextAlignament::Right);
        // item Open
        r.WriteSingleLineText(2, 3, "Open", obj.Menu.Text.Inactive);
        r.WriteCharacter(2, 3, 'O', obj.Menu.HotKey.Inactive);
        r.WriteSingleLineText(18, 3, "Ctrl+O", obj.Menu.ShortCut.Inactive, TextAlignament::Right);
        // line
        r.DrawHorizontalLine(1, 4, 19, obj.Menu.Text.Normal, true);
        // options
        r.WriteSingleLineText(2, 5, "  Option 1", obj.Menu.Text.Normal);
        r.WriteSingleLineText(2, 6, "  Option 2", obj.Menu.Text.Normal);
        r.WriteSpecialCharacter(2, 5, SpecialChars::CheckMark, obj.Menu.Symbol.Normal);
        // line
        r.DrawHorizontalLine(1, 7, 19, obj.Menu.Text.Normal, true);
        // item close all
        r.WriteSingleLineText(1, 8, " Close         ", obj.Menu.Text.Hovered);
        r.WriteCharacter(2, 8, 'C', obj.Menu.HotKey.Hovered);
        r.WriteSingleLineText(19, 8, "F10 ", obj.Menu.ShortCut.Hovered, TextAlignament::Right);
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