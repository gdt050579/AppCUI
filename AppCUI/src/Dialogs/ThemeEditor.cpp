#include "AppCUI.hpp"

using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;

namespace AppCUI::Dialogs
{
constexpr int BUTTON_CMD_CLOSE = 1;

enum class CatID : uint32
{
    None = 0,
    Desktop,
    Menu,
    ParentMenu,

    Count // must be the last one
};
constexpr string_view catNames[static_cast<uint32>(CatID::Count)] = { "", "Desktop", "Menu", "Menu (parent)" };

enum class PropID : uint32
{
    DesktopChar,
    DesktopColor,

    // menus
    MenuTextNormal,
    MenuTextHovered,
    MenuTextSelected,
    MenuHotKeyNormal,
    MenuHotKeyHovered,
    MenuHotKeySelected,
    MenuShortCutNormal,
    MenuShortCutHovered,
    MenuShortCutSelected,
    MenuSymbolNormal,
    MenuSymbolHovered,
    MenuSymbolSelected,
    MenuInactive,

    // parent menu
    ParentMenuTextNormal,
    ParentMenuTextHovered,
    ParentMenuTextSelected,
    ParentMenuHotKeyNormal,
    ParentMenuHotKeyHovered,
    ParentMenuHotKeySelected,
    ParentMenuShortCutNormal,
    ParentMenuShortCutHovered,
    ParentMenuShortCutSelected,
    ParentMenuSymbolNormal,
    ParentMenuSymbolHovered,
    ParentMenuSymbolSelected,
    ParentMenuInactive,

};
class ConfigProperty : public PropertiesInterface
{
    AppCUI::Application::Config obj;
    CatID catID;

  public:
    ConfigProperty(const AppCUI::Application::Config& config) : obj(config), catID(CatID::None)
    {
    }
    void SetCategory(string_view name)
    {
        for (auto i = 0U; i < static_cast<uint32>(CatID::Count); i++)
        {
            if (catNames[i] == name)
            {
                catID = static_cast<CatID>(i);
                return;
            }
        }
        catID = CatID::None;
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
    void PaintParentMenusAndCommandBar(Graphics::Renderer& r, Size sz)
    {
        r.FillRect(2, 1, 22, 9, ' ', obj.ParentMenu.Text.Normal);
        r.DrawRect(2, 1, 22, 9, obj.ParentMenu.Text.Normal, LineType::Single);
        // item Save
        r.WriteSingleLineText(4, 2, "Save", obj.ParentMenu.Text.Normal);
        r.WriteCharacter(4, 2, 'S', obj.ParentMenu.HotKey.Normal);
        r.WriteSingleLineText(20, 2, "Ctrl+S", obj.ParentMenu.ShortCut.Normal, TextAlignament::Right);
        // item Open
        r.WriteSingleLineText(4, 3, "Open", obj.ParentMenu.Text.Inactive);
        r.WriteCharacter(4, 3, 'O', obj.ParentMenu.HotKey.Inactive);
        r.WriteSingleLineText(20, 3, "Ctrl+O", obj.ParentMenu.ShortCut.Inactive, TextAlignament::Right);
        // line
        r.DrawHorizontalLine(3, 4, 19, obj.ParentMenu.Text.Normal, true);
        // options
        r.WriteSingleLineText(4, 5, "  Option 1", obj.ParentMenu.Text.Normal);
        r.WriteSingleLineText(4, 6, "  Option 2", obj.ParentMenu.Text.Normal);
        r.WriteSpecialCharacter(4, 5, SpecialChars::CheckMark, obj.ParentMenu.Symbol.Normal);
        // line
        r.DrawHorizontalLine(3, 7, 19, obj.ParentMenu.Text.Normal, true);
        // item close all
        r.WriteSingleLineText(3, 8, " Copy              ", obj.ParentMenu.Text.Hovered);
        r.WriteCharacter(4, 8, 'C', obj.ParentMenu.HotKey.Hovered);
        r.WriteSpecialCharacter(20, 8, SpecialChars::TriangleRight, obj.ParentMenu.Text.Hovered);
        // draw a child menu
        r.FillRect(21, 7, 35, 12, ' ', obj.Menu.Text.Normal);
        r.DrawRect(21, 7, 35, 12, obj.Menu.Text.Normal, LineType::Single);
        r.WriteSingleLineText(22, 8, " Slot 1", obj.Menu.Text.Normal);
        r.WriteSingleLineText(22, 9, " Slot 2", obj.Menu.Text.Normal);
        r.WriteSingleLineText(22, 10, " Slot 3      ", obj.Menu.Text.Hovered);
        r.WriteSingleLineText(22, 11, " Slot 4", obj.Menu.Text.Inactive);
        r.FillHorizontalLineSize(27, 7, 3, ' ', obj.Menu.Text.Normal);
        r.WriteSpecialCharacter(28, 7, SpecialChars::TriangleUp, obj.Menu.Text.Inactive);
        r.FillHorizontalLineSize(27, 12, 3, ' ', obj.Menu.Text.Normal);
        r.WriteSpecialCharacter(28, 12, SpecialChars::TriangleDown, obj.Menu.Text.Normal);
    }
    void Paint(Graphics::Renderer& r, Size sz)
    {
        switch (catID)
        {
        case CatID::None:
            r.Clear();
            break;
        case CatID::Desktop:
            r.ClearWithSpecialChar(SpecialChars::Block50, obj.Symbol.Desktop);
            break;
        case CatID::Menu:
            r.ClearWithSpecialChar(SpecialChars::Block50, obj.Symbol.Desktop);
            PaintMenusAndCommandBar(r, sz);
            break;
        case CatID::ParentMenu:
            r.ClearWithSpecialChar(SpecialChars::Block50, obj.Symbol.Desktop);
            PaintParentMenusAndCommandBar(r, sz);
            break;
        }
    }
    bool GetPropertyValue(uint32 propertyID, PropertyValue& value) override
    {
        switch (static_cast<PropID>(propertyID))
        {
        case PropID::DesktopChar:
            value = (char16) 186;
            return true;
        case PropID::DesktopColor:
            value = obj.Symbol.Desktop;
            return true;

        // Menus
        case PropID::MenuTextNormal:
            value = obj.Menu.Text.Normal;
            return true;
        case PropID::MenuTextHovered:
            value = obj.Menu.Text.Hovered;
            return true;
        case PropID::MenuTextSelected:
            value = obj.Menu.Text.PressedOrSelected;
            return true;
        case PropID::MenuHotKeyNormal:
            value = obj.Menu.HotKey.Normal;
            return true;
        case PropID::MenuHotKeyHovered:
            value = obj.Menu.HotKey.Hovered;
            return true;
        case PropID::MenuHotKeySelected:
            value = obj.Menu.HotKey.PressedOrSelected;
            return true;
        case PropID::MenuShortCutNormal:
            value = obj.Menu.ShortCut.Normal;
            return true;
        case PropID::MenuShortCutHovered:
            value = obj.Menu.ShortCut.Hovered;
            return true;
        case PropID::MenuShortCutSelected:
            value = obj.Menu.ShortCut.PressedOrSelected;
            return true;
        case PropID::MenuInactive:
            value = obj.Menu.Text.Inactive.Foreground;
            return true;
        case PropID::MenuSymbolNormal:
            value = obj.Menu.Symbol.Normal.Foreground;
            return true;
        case PropID::MenuSymbolHovered:
            value = obj.Menu.Symbol.Hovered.Foreground;
            return true;
        case PropID::MenuSymbolSelected:
            value = obj.Menu.Symbol.PressedOrSelected.Foreground;
            return true;

        // Parent Menus
        case PropID::ParentMenuTextNormal:
            value = obj.ParentMenu.Text.Normal;
            return true;
        case PropID::ParentMenuTextHovered:
            value = obj.ParentMenu.Text.Hovered;
            return true;
        case PropID::ParentMenuTextSelected:
            value = obj.ParentMenu.Text.PressedOrSelected;
            return true;
        case PropID::ParentMenuHotKeyNormal:
            value = obj.ParentMenu.HotKey.Normal;
            return true;
        case PropID::ParentMenuHotKeyHovered:
            value = obj.ParentMenu.HotKey.Hovered;
            return true;
        case PropID::ParentMenuHotKeySelected:
            value = obj.ParentMenu.HotKey.PressedOrSelected;
            return true;
        case PropID::ParentMenuShortCutNormal:
            value = obj.ParentMenu.ShortCut.Normal;
            return true;
        case PropID::ParentMenuShortCutHovered:
            value = obj.ParentMenu.ShortCut.Hovered;
            return true;
        case PropID::ParentMenuShortCutSelected:
            value = obj.ParentMenu.ShortCut.PressedOrSelected;
            return true;
        case PropID::ParentMenuInactive:
            value = obj.ParentMenu.Text.Inactive.Foreground;
            return true;
        case PropID::ParentMenuSymbolNormal:
            value = obj.ParentMenu.Symbol.Normal.Foreground;
            return true;
        case PropID::ParentMenuSymbolHovered:
            value = obj.ParentMenu.Symbol.Hovered.Foreground;
            return true;
        case PropID::ParentMenuSymbolSelected:
            value = obj.ParentMenu.Symbol.PressedOrSelected.Foreground;
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

        case PropID::MenuTextNormal:
            obj.Menu.Text.Normal              = std::get<ColorPair>(value);
            obj.Menu.Symbol.Normal.Background = obj.Menu.Text.Normal.Background;
            obj.Menu.Text.Inactive.Background = obj.Menu.Text.Normal.Background;
            return true;
        case PropID::MenuTextHovered:
            obj.Menu.Text.Hovered              = std::get<ColorPair>(value);
            obj.Menu.Symbol.Hovered.Background = obj.Menu.Text.Hovered.Background;
            obj.Menu.Symbol.Hovered.Background = obj.Menu.Text.Hovered.Background;
            return true;
        case PropID::MenuTextSelected:
            obj.Menu.Text.PressedOrSelected              = std::get<ColorPair>(value);
            obj.Menu.Symbol.PressedOrSelected.Background = obj.Menu.Text.PressedOrSelected.Background;
            obj.Menu.Symbol.PressedOrSelected.Background = obj.Menu.Text.PressedOrSelected.Background;
            return true;
        case PropID::MenuHotKeyNormal:
            obj.Menu.HotKey.Normal              = std::get<ColorPair>(value);
            obj.Menu.HotKey.Inactive.Background = obj.Menu.HotKey.Normal.Background;
            return true;
        case PropID::MenuHotKeyHovered:
            obj.Menu.HotKey.Hovered = std::get<ColorPair>(value);
            return true;
        case PropID::MenuHotKeySelected:
            obj.Menu.HotKey.PressedOrSelected = std::get<ColorPair>(value);
            return true;
        case PropID::MenuShortCutNormal:
            obj.Menu.ShortCut.Normal              = std::get<ColorPair>(value);
            obj.Menu.ShortCut.Inactive.Background = obj.Menu.ShortCut.Normal.Background;
            return true;
        case PropID::MenuShortCutHovered:
            obj.Menu.ShortCut.Hovered = std::get<ColorPair>(value);
            return true;
        case PropID::MenuShortCutSelected:
            obj.Menu.ShortCut.PressedOrSelected = std::get<ColorPair>(value);
            return true;
        case PropID::MenuSymbolNormal:
            obj.Menu.Symbol.Normal.Foreground = std::get<Color>(value);
            return true;
        case PropID::MenuSymbolHovered:
            obj.Menu.Symbol.Hovered.Foreground = std::get<Color>(value);
            return true;
        case PropID::MenuSymbolSelected:
            obj.Menu.Symbol.PressedOrSelected.Foreground = std::get<Color>(value);
            return true;
        case PropID::MenuInactive:
            obj.Menu.Text.Inactive.Foreground     = std::get<Color>(value);
            obj.Menu.HotKey.Inactive.Foreground   = std::get<Color>(value);
            obj.Menu.ShortCut.Inactive.Foreground = std::get<Color>(value);
            obj.Menu.Symbol.Inactive.Foreground   = std::get<Color>(value);
            return true;

        case PropID::ParentMenuTextNormal:
            obj.ParentMenu.Text.Normal              = std::get<ColorPair>(value);
            obj.ParentMenu.Symbol.Normal.Background = obj.ParentMenu.Text.Normal.Background;
            obj.ParentMenu.Text.Inactive.Background = obj.ParentMenu.Text.Normal.Background;
            return true;
        case PropID::ParentMenuTextHovered:
            obj.ParentMenu.Text.Hovered              = std::get<ColorPair>(value);
            obj.ParentMenu.Symbol.Hovered.Background = obj.ParentMenu.Text.Hovered.Background;
            obj.ParentMenu.Symbol.Hovered.Background = obj.ParentMenu.Text.Hovered.Background;
            return true;
        case PropID::ParentMenuTextSelected:
            obj.ParentMenu.Text.PressedOrSelected              = std::get<ColorPair>(value);
            obj.ParentMenu.Symbol.PressedOrSelected.Background = obj.ParentMenu.Text.PressedOrSelected.Background;
            obj.ParentMenu.Symbol.PressedOrSelected.Background = obj.ParentMenu.Text.PressedOrSelected.Background;
            return true;
        case PropID::ParentMenuHotKeyNormal:
            obj.ParentMenu.HotKey.Normal              = std::get<ColorPair>(value);
            obj.ParentMenu.HotKey.Inactive.Background = obj.ParentMenu.HotKey.Normal.Background;
            return true;
        case PropID::ParentMenuHotKeyHovered:
            obj.ParentMenu.HotKey.Hovered = std::get<ColorPair>(value);
            return true;
        case PropID::ParentMenuHotKeySelected:
            obj.ParentMenu.HotKey.PressedOrSelected = std::get<ColorPair>(value);
            return true;
        case PropID::ParentMenuShortCutNormal:
            obj.ParentMenu.ShortCut.Normal              = std::get<ColorPair>(value);
            obj.ParentMenu.ShortCut.Inactive.Background = obj.ParentMenu.ShortCut.Normal.Background;
            return true;
        case PropID::ParentMenuShortCutHovered:
            obj.ParentMenu.ShortCut.Hovered = std::get<ColorPair>(value);
            return true;
        case PropID::ParentMenuShortCutSelected:
            obj.ParentMenu.ShortCut.PressedOrSelected = std::get<ColorPair>(value);
            return true;
        case PropID::ParentMenuSymbolNormal:
            obj.ParentMenu.Symbol.Normal.Foreground = std::get<Color>(value);
            return true;
        case PropID::ParentMenuSymbolHovered:
            obj.ParentMenu.Symbol.Hovered.Foreground = std::get<Color>(value);
            return true;
        case PropID::ParentMenuSymbolSelected:
            obj.ParentMenu.Symbol.PressedOrSelected.Foreground = std::get<Color>(value);
            return true;
        case PropID::ParentMenuInactive:
            obj.ParentMenu.Text.Inactive.Foreground     = std::get<Color>(value);
            obj.ParentMenu.HotKey.Inactive.Foreground   = std::get<Color>(value);
            obj.ParentMenu.ShortCut.Inactive.Foreground = std::get<Color>(value);
            obj.ParentMenu.Symbol.Inactive.Foreground   = std::get<Color>(value);
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
#define PT(t)  static_cast<uint32>(t)
#define CAT(t) catNames[static_cast<uint32>(t)]
        return {
            { PT(PropID::DesktopChar), CAT(CatID::Desktop), "Symbol", PropertyType::Char16 },
            { PT(PropID::DesktopColor), CAT(CatID::Desktop), "Color", PropertyType::ColorPair },
            // Menus
            { PT(PropID::MenuTextNormal), CAT(CatID::Menu), "Text (normal)", PropertyType::ColorPair },
            { PT(PropID::MenuTextHovered), CAT(CatID::Menu), "Text (hovered)", PropertyType::ColorPair },
            { PT(PropID::MenuTextSelected), CAT(CatID::Menu), "Text (selected)", PropertyType::ColorPair },
            { PT(PropID::MenuHotKeyNormal), CAT(CatID::Menu), "HotKey (normal)", PropertyType::ColorPair },
            { PT(PropID::MenuHotKeyHovered), CAT(CatID::Menu), "HotKey (hovered)", PropertyType::ColorPair },
            { PT(PropID::MenuHotKeySelected), CAT(CatID::Menu), "HotKey (selected)", PropertyType::ColorPair },
            { PT(PropID::MenuShortCutNormal), CAT(CatID::Menu), "ShortCut (normal)", PropertyType::ColorPair },
            { PT(PropID::MenuShortCutHovered), CAT(CatID::Menu), "ShortCut (hovered)", PropertyType::ColorPair },
            { PT(PropID::MenuShortCutSelected), CAT(CatID::Menu), "ShortCut (selected)", PropertyType::ColorPair },
            { PT(PropID::MenuInactive), CAT(CatID::Menu), "Inactive", PropertyType::Color },
            { PT(PropID::MenuSymbolNormal), CAT(CatID::Menu), "Symbols (normal)", PropertyType::Color },
            { PT(PropID::MenuSymbolSelected), CAT(CatID::Menu), "Symbols (selected)", PropertyType::Color },
            { PT(PropID::MenuSymbolHovered), CAT(CatID::Menu), "Symbols (hovered)", PropertyType::Color },
            // parent menu
            { PT(PropID::ParentMenuTextNormal), CAT(CatID::ParentMenu), "Text (normal)", PropertyType::ColorPair },
            { PT(PropID::ParentMenuTextHovered), CAT(CatID::ParentMenu), "Text (hovered)", PropertyType::ColorPair },
            { PT(PropID::ParentMenuTextSelected), CAT(CatID::ParentMenu), "Text (selected)", PropertyType::ColorPair },
            { PT(PropID::ParentMenuHotKeyNormal), CAT(CatID::ParentMenu), "HotKey (normal)", PropertyType::ColorPair },
            { PT(PropID::ParentMenuHotKeyHovered), CAT(CatID::ParentMenu), "HotKey (hovered)", PropertyType::ColorPair },
            { PT(PropID::ParentMenuHotKeySelected), CAT(CatID::ParentMenu), "HotKey (selected)", PropertyType::ColorPair },
            { PT(PropID::ParentMenuShortCutNormal), CAT(CatID::ParentMenu), "ShortCut (normal)", PropertyType::ColorPair },
            { PT(PropID::ParentMenuShortCutHovered), CAT(CatID::ParentMenu), "ShortCut (hovered)", PropertyType::ColorPair },
            { PT(PropID::ParentMenuShortCutSelected), CAT(CatID::ParentMenu), "ShortCut (selected)", PropertyType::ColorPair },
            { PT(PropID::ParentMenuInactive), CAT(CatID::ParentMenu), "Inactive", PropertyType::Color },
            { PT(PropID::ParentMenuSymbolNormal), CAT(CatID::ParentMenu), "Symbols (normal)", PropertyType::Color },
            { PT(PropID::ParentMenuSymbolSelected), CAT(CatID::ParentMenu), "Symbols (selected)", PropertyType::Color },
            { PT(PropID::ParentMenuSymbolHovered), CAT(CatID::ParentMenu), "Symbols (hovered)", PropertyType::Color },

        };
#undef PT
#undef CAT
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
        pc = sp->CreateChildControl<PreviewControl>();
        pc->SetConfig(&cfg);
        prop = Factory::PropertyList::Create(sp, "d:c", &cfg, PropertyListFlags::None);
        cfg.SetCategory(prop->GetCurrentItemCategory());
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
        if (eventType == Event::PropertyItemChanged)
        {
            cfg.SetCategory(prop->GetCurrentItemCategory());
            return true;
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