#include <cassert>

#include "Internal.hpp"

using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;

#ifdef MessageBox
#    undef MessageBox
#endif

namespace AppCUI::Dialogs
{
//#define SETUP_TO_CPP_MODE

#ifdef SETUP_TO_CPP_MODE
void PrintColorToString(Color& c, string_view name, AppCUI::Utils::String& output)
{
    output.Add("cfg.");
    output.Add(name);
    output.Add(" = Color::");
    output.Add(ColorUtils::GetColorName(c));
    output.Add(";\n");
}
void PrintColorToString(ColorPair& c, string_view name, AppCUI::Utils::String& output)
{
    output.Add("cfg.");
    output.Add(name);
    output.Add(" = {Color::");
    output.Add(ColorUtils::GetColorName(c.Foreground));
    output.Add(" , Color::");
    output.Add(ColorUtils::GetColorName(c.Background));
    output.Add("};\n");
}
void PrintColorToString(ObjectColorState& c, string_view name, AppCUI::Utils::String& output)
{
    output.Add("cfg.");
    output.Add(name);
    output.Add(".Set(");
    output.Add("{Color::");
    output.Add(ColorUtils::GetColorName(c.Focused.Foreground));
    output.Add(" , Color::");
    output.Add(ColorUtils::GetColorName(c.Focused.Background));
    output.Add("}, ");

    output.Add("{Color::");
    output.Add(ColorUtils::GetColorName(c.Normal.Foreground));
    output.Add(" , Color::");
    output.Add(ColorUtils::GetColorName(c.Normal.Background));
    output.Add("}, ");

    output.Add("{Color::");
    output.Add(ColorUtils::GetColorName(c.Inactive.Foreground));
    output.Add(" , Color::");
    output.Add(ColorUtils::GetColorName(c.Inactive.Background));
    output.Add("}, ");

    output.Add("{Color::");
    output.Add(ColorUtils::GetColorName(c.Hovered.Foreground));
    output.Add(" , Color::");
    output.Add(ColorUtils::GetColorName(c.Hovered.Background));
    output.Add("}, ");

    output.Add("{Color::");
    output.Add(ColorUtils::GetColorName(c.PressedOrSelected.Foreground));
    output.Add(" , Color::");
    output.Add(ColorUtils::GetColorName(c.PressedOrSelected.Background));
    output.Add("});\n");
}
#    define OUTPUT_CPP(name) PrintColorToString(cfg.name, #    name, output);
void CreateCPPCode(AppCUI::Application::Config& cfg, AppCUI::Utils::String& output)
{
    OUTPUT_CPP(SearchBar);
    OUTPUT_CPP(Border);
    OUTPUT_CPP(Lines);
    OUTPUT_CPP(Editor);
    OUTPUT_CPP(LineMarker);

    OUTPUT_CPP(Button.Text);
    OUTPUT_CPP(Button.HotKey);

    OUTPUT_CPP(Text.Normal);
    OUTPUT_CPP(Text.HotKey);
    OUTPUT_CPP(Text.Inactive);
    OUTPUT_CPP(Text.Error);
    OUTPUT_CPP(Text.Warning);
    OUTPUT_CPP(Text.Hovered);
    OUTPUT_CPP(Text.Focused);
    OUTPUT_CPP(Text.Highlighted);
    OUTPUT_CPP(Text.Emphasized1);
    OUTPUT_CPP(Text.Emphasized2);

    OUTPUT_CPP(Symbol.Inactive);
    OUTPUT_CPP(Symbol.Hovered);
    OUTPUT_CPP(Symbol.Pressed);
    OUTPUT_CPP(Symbol.Checked);
    OUTPUT_CPP(Symbol.Unchecked);
    OUTPUT_CPP(Symbol.Unknown);
    OUTPUT_CPP(Symbol.Desktop);
    OUTPUT_CPP(Symbol.Arrows);
    OUTPUT_CPP(Symbol.Close);
    OUTPUT_CPP(Symbol.Maximized);
    OUTPUT_CPP(Symbol.Resize);

    OUTPUT_CPP(Cursor.Normal);
    OUTPUT_CPP(Cursor.Inactive);
    OUTPUT_CPP(Cursor.OverInactiveItem);
    OUTPUT_CPP(Cursor.OverSelection);

    OUTPUT_CPP(Selection.Editor);
    OUTPUT_CPP(Selection.LineMarker);
    OUTPUT_CPP(Selection.Text);
    OUTPUT_CPP(Selection.SearchMarker);
    OUTPUT_CPP(Selection.SimilarText);

    OUTPUT_CPP(ProgressStatus.Empty);
    OUTPUT_CPP(ProgressStatus.Full);

    OUTPUT_CPP(Menu.Text);
    OUTPUT_CPP(Menu.HotKey);
    OUTPUT_CPP(Menu.ShortCut);
    OUTPUT_CPP(Menu.Symbol);
    OUTPUT_CPP(ParentMenu.Text);
    OUTPUT_CPP(ParentMenu.HotKey);
    OUTPUT_CPP(ParentMenu.ShortCut);
    OUTPUT_CPP(ParentMenu.Symbol);

    OUTPUT_CPP(Header.Text);
    OUTPUT_CPP(Header.HotKey);
    OUTPUT_CPP(Header.Symbol);

    OUTPUT_CPP(ScrollBar.Bar);
    OUTPUT_CPP(ScrollBar.Arrows);
    OUTPUT_CPP(ScrollBar.Position);

    OUTPUT_CPP(ToolTip.Text);
    OUTPUT_CPP(ToolTip.Arrow);

    OUTPUT_CPP(Tab.Text);
    OUTPUT_CPP(Tab.HotKey);
    OUTPUT_CPP(Tab.ListText);
    OUTPUT_CPP(Tab.ListHotKey);

    OUTPUT_CPP(Window.Background.Normal);
    OUTPUT_CPP(Window.Background.Inactive);
    OUTPUT_CPP(Window.Background.Error);
    OUTPUT_CPP(Window.Background.Warning);
    OUTPUT_CPP(Window.Background.Info);
}
#endif

constexpr int BUTTON_CMD_CLOSE = 1;
constexpr int BUTTON_CMD_APPLY = 2;
constexpr int BUTTON_CMD_SAVE  = 3;
constexpr int BUTTON_CMD_LOAD  = 4;

enum class PreviewWindowID : uint32
{
    Normal = 0,
    Inactive,
    Error,
    Warning,
    Notification
};
using CatID = uint32;
enum class CatType : uint32
{
    None = 0,
    Desktop,
    Menu,
    ParentMenu,
    Window,
    ToolTip,
    ProgressBar,
    Button,
    Text,
    ScrollBars,
    Symbols,
    SearchBar,
    Headers,
    Cursor,
    Editor,
    BorderAndLines,
    Tabs,
    TabsList,

    Count // must be the last one
};
constexpr string_view catNames[static_cast<uint32>(CatType::Count)] = {
    "",        "Desktop",        "Menu",        "Menu (parent)", "Window",    "ToolTip", "Progress Bar",
    "Buttons", "Text",           "Scroll bars", "Symbols",       "SearchBar", "Headers", "Cursor",
    "Editor",  "Border & Lines", "Tabs",        "Tabs (lists)"
};

using PropID = uint32;
enum class PropType : uint32
{
    None,

    // Desktop
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
    ParentMenuHotKeyNormal,
    ParentMenuHotKeyHovered,
    ParentMenuShortCutNormal,
    ParentMenuShortCutHovered,
    ParentMenuSymbolNormal,
    ParentMenuSymbolHovered,
    ParentMenuInactive,

    // Windows
    WindowNormal,
    WindowInactive,
    WindowError,
    WindowInfo,
    WindowWarning,

    // ToolTip
    ToolTipText,
    ToolTipArrow,

    // Progress BAr
    ProgressBarEmpty,
    ProgressBarFull,

    // Button
    ButtonTextFocused,
    ButtonTextNormal,
    ButtonTextHovered,
    ButtonTextSelected,
    ButtonTextInactive,
    ButtonHotKeyFocused,
    ButtonHotKeyNormal,
    ButtonHotKeyHovered,
    ButtonHotKeySelected,
    ButtonHotKeyInactive,
    ButtonShadow,

    // Text
    TextNormal,
    TextHotKey,
    TextInactive,
    TextError,
    TextWarning,
    TextFocused,
    TextHovered,
    TextHighlighted,
    TextEmphasized1,
    TextEmphasized2,

    // ScrollBars
    ScrollBarButtonNormal,
    ScrollBarButtonHovered,
    ScrollBarButtonPressed,
    ScrollBarButtonInactive,
    ScrollBarNormal,
    ScrollBarHovered,
    ScrollBarPressed,
    ScrollBarInactive,
    ScrollBarPositionNormal,
    ScrollBarPositionHovered,
    ScrollBarPositionPressed,

    // Symbols
    SymbolInactive,
    SymbolHovered,
    SymbolPressed,
    SymbolChecked,
    SymbolUnchecked,
    SymbolUnknown,
    SymbolArrows,
    SymbolClose,
    SymbolMaximized,
    SymbolResize,

    // SearchBar
    SearchBarNormal,
    SearchBarHovered,
    SearchBarFocused,
    SearchBarInactive,

    // Header
    HeaderTextNormal,
    HeaderTextFocused,
    HeaderTextHovered,
    HeaderTextSelected,
    HeaderTextInactive,
    HeaderHotKeyNormal,
    HeaderHotKeyFocused,
    HeaderHotKeyHovered,
    HeaderHotKeySelected,
    HeaderHotKeyInactive,
    HeaderSymbolNormal,
    HeaderSymbolFocused,
    HeaderSymbolHovered,
    HeaderSymbolSelected,
    HeaderSymbolInactive,

    // Cursor
    CursorNormal,
    CursorInactive,
    CursorOverInactiveItem,
    CursorOverSelection,

    // Editor
    EditorBackground,
    EditorNormal,
    EditorInactive,
    EditorFocus,
    EditorHovered,
    EditorSelection,
    EditorLineMarkerNormal,
    EditorLineMarkerFocused,
    EditorLineMarkerInactive,
    EditorLineMarkerHovered,

    // Border & Lines
    BorderNormal,
    BorderFocused,
    BorderInactive,
    BorderHovered,
    BorderPressed,
    LineNormal,
    LineFocused,
    LineInactive,
    LineHovered,
    LinePressed,

    // Tabs
    TabsTextFocused,
    TabsTextNormal,
    TabsTextHovered,
    TabsTextInactive,
    TabsTextSelected,
    TabsHotKeyFocused,
    TabsHotKeyNormal,
    TabsHotKeyHovered,
    TabsHotKeyInactive,
    TabsHotKeySelected,

    // TabsList
    TabsListTextFocused,
    TabsListTextNormal,
    TabsListTextHovered,
    TabsListTextInactive,
    TabsListTextSelected,
    TabsListHotKeyFocused,
    TabsListHotKeyNormal,
    TabsListHotKeyHovered,
    TabsListHotKeyInactive,
    TabsListHotKeySelected,

    Count
};

struct ConfigCustomEntry
{
    std::string_view category;
    std::string colorName;
    PropertyType propertyType;
    CustomColor *color;
    std::optional<uint8> stateIndex;
};

struct StringHash
{
    using is_transparent = void;
    std::size_t operator()(std::string_view sv) const noexcept
    {
        return std::hash<std::string_view>{}(sv);
    }
};

struct StringEqual
{
    using is_transparent = void;
    bool operator()(std::string_view lhs, std::string_view rhs) const noexcept
    {
        return lhs == rhs;
    }
};

struct CategoryRefWithName
{
    Reference<Application::Config::CategoryColorsData> categoryRef;
    std::string_view categoryName;
};

class ConfigProperty : public PropertiesInterface
{
    // <propID, <category, color_name, value>>

    AppCUI::Application::Config obj;
    CatID catID;
    PropID propID;
    PreviewWindowID windowID;
    CatID catIDCount;
    std::unordered_map<std::string, CatID, StringHash, StringEqual> categoriesName;
    std::unordered_map<CatID, CategoryRefWithName> categoriesData;
    std::map<PropID, ConfigCustomEntry> customProperties;

  public:
    ConfigProperty(const AppCUI::Application::Config& config)
        : obj(config), catID(static_cast<CatID>(CatType::None)), propID(static_cast<PropID>(PropType::None)),
          windowID(PreviewWindowID::Normal)
    {
        ReInitializeFields();
    }

    void ReInitializeFields()
    {
        categoriesName.clear();
        categoriesData.clear();
        catIDCount = static_cast<CatID>(CatType::Count) + static_cast<uint32>(obj.CustomColors.size());

        CatID currentCategoryID = static_cast<CatID>(CatType::None);
        for (; currentCategoryID < static_cast<CatID>(CatType::Count); currentCategoryID++)
            categoriesName[std::string(catNames[static_cast<uint32>(currentCategoryID)])] = currentCategoryID;

        PropID currentPropId = static_cast<PropID>(PropType::Count);
        for (auto& [categoryName, categoryColors] : obj.CustomColors)
        {
            for (auto& [colorName, color] : categoryColors.data)
            {
                if (color.IsColorState())
                {
                    static_assert(
                          OBJECT_COLOR_STATE_COUNT == 5, "If OBJECT_COLOR_STATE_COUNT changes, update the code below!");
                    LocalString<128> buffer;
                    for (uint32 state = 0; state < OBJECT_COLOR_STATE_COUNT; ++state)
                    {
                        buffer.SetFormat("%s (%s)", colorName.c_str(), OBJECT_COLOR_STATE_NAMES[state].data());
                        ConfigCustomEntry entry           = { .category     = categoryName,
                                                              .colorName    = buffer.GetText(),
                                                              .propertyType = PropertyType::ColorPair,
                                                              .color        = &color,
                                                              .stateIndex   = state };
                        customProperties[currentPropId++] = std::move(entry);
                    }
                    continue;
                }

                if (color.IsColorPair())
                {
                    ConfigCustomEntry entry           = { .category     = categoryName,
                                                          .colorName    = colorName,
                                                          .propertyType = PropertyType::ColorPair,
                                                          .color        = &color,
                                                          .stateIndex   = std::nullopt };
                    customProperties[currentPropId++] = std::move(entry);
                    continue;
                }
                assert(false && "Only ColorPair and ColorState are supported for custom colors!");
            }
            categoriesData[currentCategoryID] = { &categoryColors, categoryName };
            categoriesName[categoryName]      = currentCategoryID++;
        }
    }

    inline AppCUI::Application::Config& GetConfig()
    {
        return obj;
    }
    void SetCategoryAndProperty(string_view name, PropID pID)
    {
        propID = pID;
        auto nameIt = categoriesName.find(name);
        if (nameIt != categoriesName.end())
        {
            catID =  nameIt->second;
            return;
        }
        catID = static_cast<uint32>(CatType::None);
    }
    void SetPreviewWindowID(PreviewWindowID id)
    {
        windowID = id;
    }
    void DrawWindow(
          Graphics::Renderer& r,
          int left,
          int top,
          int right,
          int bottom,
          string_view title,
          Color backColor,
          bool focused = true)
    {
        r.FillRect(left, top, right, bottom, ' ', { Color::Black, backColor });
        if (focused)
            r.DrawRect(left, top, right, bottom, obj.Border.Focused, LineType::Double);
        else
            r.DrawRect(left, top, right, bottom, obj.Border.Normal, LineType::Single);

        if ((int) title.size() < ((right - left) - 4))
        {
            r.WriteSingleLineText(
                  (left + right) >> 1,
                  top,
                  title,
                  focused ? obj.Text.Focused : obj.Text.Normal,
                  TextAlignament::Center);
        }
    }
    void DrawPreviewWindow(Graphics::Renderer& r, int left, int top, int right, int bottom, string_view title)
    {
        switch (this->windowID)
        {
        case PreviewWindowID::Normal:
            DrawWindow(r, left, top, right, bottom, title, obj.Window.Background.Normal);
            break;
        case PreviewWindowID::Error:
            DrawWindow(r, left, top, right, bottom, title, obj.Window.Background.Error);
            break;
        case PreviewWindowID::Notification:
            DrawWindow(r, left, top, right, bottom, title, obj.Window.Background.Info);
            break;
        case PreviewWindowID::Warning:
            DrawWindow(r, left, top, right, bottom, title, obj.Window.Background.Warning);
            break;
        case PreviewWindowID::Inactive:
            DrawWindow(r, left, top, right, bottom, title, obj.Window.Background.Inactive, false);
            break;
        }
    }
    void PaintDesktop(Graphics::Renderer& r)
    {
        r.ClearWithSpecialChar(SpecialChars::Block50, obj.Symbol.Desktop);
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
    void PaintParentMenusAndCommandBar(Graphics::Renderer& r, Size /*sz*/)
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
    void PaintWindow(Graphics::Renderer& r, Size sz)
    {
        if ((sz.Width < 4) || (sz.Height < 4))
            return;
        switch (propID)
        {
        case static_cast<uint32>(PropType::WindowError):
            DrawWindow(r, 2, 1, sz.Width - 3, sz.Height - 2, " Error ", obj.Window.Background.Error);
            break;
        case static_cast<uint32>(PropType::WindowInfo):
            DrawWindow(r, 2, 1, sz.Width - 3, sz.Height - 2, " Info ", obj.Window.Background.Info);
            break;
        case static_cast<uint32>(PropType::WindowWarning):
            DrawWindow(r, 2, 1, sz.Width - 3, sz.Height - 2, " Warning ", obj.Window.Background.Warning);
            break;
        case static_cast<uint32>(PropType::WindowInactive):
            DrawWindow(r, 2, 1, sz.Width - 3, sz.Height - 2, " Title ", obj.Window.Background.Inactive, false);
            break;
        default:
            DrawWindow(r, 2, 1, sz.Width - 3, sz.Height - 2, " Title ", obj.Window.Background.Normal);
            break;
        }
    }
    void PaintToolTip(Graphics::Renderer& r, Size sz)
    {
        DrawPreviewWindow(r, 2, 3, sz.Width - 3, sz.Height - 3, " Title ");
        r.WriteSingleLineText(sz.Width / 2, 1, " Tool tip text ", obj.ToolTip.Text, TextAlignament::Center);
        r.WriteSpecialCharacter(sz.Width / 2, 2, SpecialChars::ArrowDown, obj.ToolTip.Arrow);
    }
    void PaintProgressBar(Graphics::Renderer& r, Size sz)
    {
        DrawWindow(r, 2, sz.Height / 2 - 2, sz.Width - 3, sz.Height / 2 + 2, " Status ", obj.Window.Background.Normal);
        r.FillHorizontalLine(4, sz.Height / 2, sz.Width - 5, ' ', obj.ProgressStatus.Empty);
        r.FillHorizontalLine(4, sz.Height / 2, sz.Width / 2, ' ', obj.ProgressStatus.Full);
    }
    void PaintOneButton(Graphics::Renderer& r, int x, int y, string_view text, ControlState state, bool hasShadow)
    {
        auto txCol = obj.Button.Text.GetColor(state);
        auto hkCol = obj.Button.HotKey.GetColor(state);
        r.WriteSingleLineText(x, y, text, txCol, hkCol, 2, TextAlignament::Left);
        if (hasShadow)
        {
            r.FillHorizontalLineWithSpecialChar(
                  x + 1, y + 1, x + (int) text.length(), SpecialChars::BlockUpperHalf, obj.Button.ShadowColor);
            r.WriteSpecialCharacter(x + (int) text.length(), y, SpecialChars::BlockLowerHalf, obj.Button.ShadowColor);
        }
    }
    void PaintButtons(Graphics::Renderer& r, Size sz)
    {
        int x, y;

        x = (int) (sz.Width / 2) - 14;
        y = (int) (sz.Height / 2);
        DrawPreviewWindow(r, x - 2, y - 4, x + 27, y + 4, " Buttons ");

        PaintOneButton(r, x, y - 2, "  Focused  ", ControlState::Focused, true);
        PaintOneButton(r, x, y, "  Regular  ", ControlState::Normal, true);
        PaintOneButton(r, x, y + 2, "  Hovered  ", ControlState::Hovered, true);
        PaintOneButton(r, x + 14, y, "  Inactive  ", ControlState::Inactive, true);
        PaintOneButton(r, x + 14, y + 2, "  Pressed  ", ControlState::PressedOrSelected, false);
    }
    void PaintTexts(Graphics::Renderer& r, Size sz)
    {
        DrawPreviewWindow(r, 2, 3, sz.Width - 3, sz.Height - 3, " Texts ");
        r.WriteSingleLineText(4, 4, "Regular text", obj.Text.Normal, obj.Text.HotKey, 0);
        r.WriteSingleLineText(4, 5, "Inactive text", obj.Text.Inactive);
        r.WriteSingleLineText(4, 6, "Hovered text", obj.Text.Hovered);
        r.WriteSingleLineText(4, 7, "Focused text", obj.Text.Focused);

        r.WriteSingleLineText(20, 4, "Error messages", obj.Text.Error);
        r.WriteSingleLineText(20, 5, "Warning messages", obj.Text.Warning);
        r.WriteSingleLineText(20, 6, "Highlighted text", obj.Text.Highlighted);
        r.WriteSingleLineText(20, 7, "Emphasized text (1)", obj.Text.Emphasized1);
        r.WriteSingleLineText(20, 8, "Emphasized text (2)", obj.Text.Emphasized2);
    }
    void PaintSymbols(Graphics::Renderer& r, Size sz)
    {
        DrawPreviewWindow(r, 2, 3, sz.Width - 3, sz.Height - 3, " Symbols ");
        r.WriteSingleLineText(3, 3, "[ ]", obj.Border.Focused);
        r.WriteSpecialCharacter(4, 3, SpecialChars::ArrowUp, obj.Symbol.Maximized);
        r.WriteSingleLineText(sz.Width - 6, 3, "[ ]", obj.Border.Focused);
        r.WriteCharacter(sz.Width - 5, 3, 'x', obj.Symbol.Close);
        r.WriteSpecialCharacter(
              sz.Width - 3, sz.Height - 3, SpecialChars::BoxBottomRightCornerSingleLine, obj.Symbol.Resize);

        r.WriteSingleLineText(6, 5, "Check symbol", obj.Text.Normal);
        r.WriteSpecialCharacter(4, 5, SpecialChars::CheckMark, obj.Symbol.Checked);
        r.WriteSingleLineText(6, 6, "Un-Check symbol", obj.Text.Normal);
        r.WriteCharacter(4, 6, 'x', obj.Symbol.Unchecked);
        r.WriteSingleLineText(6, 7, "Unknown symbol", obj.Text.Normal);
        r.WriteCharacter(4, 7, '?', obj.Symbol.Unknown);
        r.WriteSingleLineText(6, 8, "Inactive", obj.Text.Inactive);
        r.WriteSpecialCharacter(4, 8, SpecialChars::CheckMark, obj.Symbol.Inactive);

        const auto cy = sz.Height - 5;
        r.DrawHorizontalLine(3, cy, sz.Width - 4, obj.Lines.Normal);
        r.WriteSpecialCharacter(5, cy, SpecialChars::TriangleUp, obj.Symbol.Arrows);
        r.WriteSpecialCharacter(7, cy, SpecialChars::TriangleUp, obj.Symbol.Hovered);
        r.WriteSpecialCharacter(9, cy, SpecialChars::TriangleDown, obj.Symbol.Pressed);
    }
    void PaintScrollBars(Graphics::Renderer& r, Size /*sz*/)
    {
        DrawPreviewWindow(r, 2, 3, 16, 10, " Scroll ");
        r.FillHorizontalLineWithSpecialChar(4, 10, 14, SpecialChars::Block25, obj.ScrollBar.Bar.Normal);
        r.WriteSpecialCharacter(4, 10, SpecialChars::TriangleLeft, obj.ScrollBar.Arrows.Normal);
        r.WriteSpecialCharacter(14, 10, SpecialChars::TriangleRight, obj.ScrollBar.Arrows.Normal);
        r.WriteSpecialCharacter(10, 10, SpecialChars::BlockCentered, obj.ScrollBar.Position.Normal);

        r.FillVerticalLineWithSpecialChar(16, 4, 9, SpecialChars::Block25, obj.ScrollBar.Bar.Normal);
        r.WriteSpecialCharacter(16, 4, SpecialChars::TriangleUp, obj.ScrollBar.Arrows.Inactive);
        r.WriteSpecialCharacter(16, 9, SpecialChars::TriangleDown, obj.ScrollBar.Arrows.Hovered);
        r.WriteSpecialCharacter(16, 5, SpecialChars::BlockCentered, obj.ScrollBar.Position.Normal);

        DrawPreviewWindow(r, 18, 3, 32, 10, " Scroll ");
        r.FillHorizontalLineWithSpecialChar(20, 10, 30, SpecialChars::Block25, obj.ScrollBar.Bar.Normal);
        r.WriteSpecialCharacter(20, 10, SpecialChars::TriangleLeft, obj.ScrollBar.Arrows.Normal);
        r.WriteSpecialCharacter(30, 10, SpecialChars::TriangleRight, obj.ScrollBar.Arrows.Normal);
        r.WriteSpecialCharacter(24, 10, SpecialChars::BlockCentered, obj.ScrollBar.Position.Hovered);
        r.FillVerticalLineWithSpecialChar(32, 4, 9, SpecialChars::Block25, obj.ScrollBar.Bar.Inactive);
        r.WriteSpecialCharacter(32, 4, SpecialChars::TriangleUp, obj.ScrollBar.Arrows.Inactive);
        r.WriteSpecialCharacter(32, 9, SpecialChars::TriangleDown, obj.ScrollBar.Arrows.Inactive);
    }

    void PaintSearchBar(Graphics::Renderer& r, Size sz)
    {
        if (sz.Width < 6)
            return;
        const auto cx = sz.Width / 2;
        const auto cy = sz.Height / 2;
        const auto w  = cx > 10 ? cx - 10 : 0;
        DrawPreviewWindow(r, 2, 1, cx - 1, cy - 1, " Search ");
        r.FillHorizontalLine(4, cy - 1, cx - 3, ' ', obj.SearchBar.Focused);
        r.WriteSingleLineText(5, cy - 1, w, "Focused", obj.SearchBar.Focused);

        DrawPreviewWindow(r, cx + 2, 1, sz.Width - 3, cy - 1, " Search ");
        r.FillHorizontalLine(cx + 4, cy - 1, sz.Width - 5, ' ', obj.SearchBar.Normal);
        r.WriteSingleLineText(cx + 6, cy - 1, w, "Regular", obj.SearchBar.Normal);

        DrawPreviewWindow(r, 2, cy + 1, cx - 1, sz.Height - 2, " Search ");
        r.FillHorizontalLine(4, sz.Height - 2, cx - 3, ' ', obj.SearchBar.Hovered);
        r.WriteSingleLineText(5, sz.Height - 2, w, "Hovered", obj.SearchBar.Hovered);

        DrawPreviewWindow(r, cx + 2, cy + 1, sz.Width - 3, sz.Height - 2, " Search ");
        r.FillHorizontalLine(cx + 4, sz.Height - 2, sz.Width - 5, ' ', obj.SearchBar.Inactive);
        r.WriteSingleLineText(cx + 6, sz.Height - 2, w, "Inactive", obj.SearchBar.Inactive);
    }

    void PaintHeaders(Graphics::Renderer& r, Size sz)
    {
        DrawPreviewWindow(r, 2, 1, sz.Width - 3, sz.Height - 2, " Headers ");
        r.SetClipMargins(3, 2, 3, 2);
        r.FillHorizontalLine(3, 2, sz.Width - 4, ' ', obj.Header.Text.Focused);
        r.WriteSingleLineText(
              3, 2, " Selected  ", obj.Header.Text.PressedOrSelected, obj.Header.HotKey.PressedOrSelected, 1);
        r.WriteSpecialCharacter(13, 2, SpecialChars::TriangleDown, obj.Header.Symbol.PressedOrSelected);
        r.DrawVerticalLine(14, 2, 5, obj.Lines.Focused);
        r.WriteSingleLineText(15, 2, " Column  ", obj.Header.Text.Focused, obj.Header.HotKey.Focused, 1);
        r.WriteSpecialCharacter(23, 2, SpecialChars::TriangleDown, obj.Header.Symbol.Focused);
        r.DrawVerticalLine(24, 2, 5, obj.Lines.Focused);
        r.WriteSingleLineText(25, 2, " Hovered  ", obj.Header.Text.Hovered, obj.Header.HotKey.Hovered, 1);
        r.WriteSpecialCharacter(35, 2, SpecialChars::TriangleDown, obj.Header.Symbol.Hovered);
        r.DrawVerticalLine(36, 2, 5, obj.Lines.Focused);

        r.FillHorizontalLine(3, 7, sz.Width - 4, ' ', obj.Header.Text.Normal);
        r.WriteSingleLineText(3, 7, " Column  ", obj.Header.Text.Normal, obj.Header.HotKey.Normal, 1);
        r.WriteSpecialCharacter(13, 7, SpecialChars::TriangleDown, obj.Header.Symbol.Normal);
        r.DrawVerticalLine(14, 7, 9, obj.Lines.Normal);
        r.WriteSingleLineText(15, 7, " Column  ", obj.Header.Text.Normal, obj.Header.HotKey.Normal, 1);
        r.WriteSpecialCharacter(23, 7, SpecialChars::TriangleDown, obj.Header.Symbol.Normal);
        r.DrawVerticalLine(24, 7, 9, obj.Lines.Normal);

        r.FillHorizontalLine(3, 11, sz.Width - 4, ' ', obj.Header.Text.Inactive);
        r.WriteSingleLineText(3, 11, " Column  ", obj.Header.Text.Inactive, obj.Header.HotKey.Inactive, 1);
        r.WriteSpecialCharacter(13, 11, SpecialChars::TriangleDown, obj.Header.Symbol.Inactive);
        r.DrawVerticalLine(14, 11, 13, obj.Lines.Inactive);
        r.WriteSingleLineText(15, 11, " Column  ", obj.Header.Text.Inactive, obj.Header.HotKey.Inactive, 1);
        r.WriteSpecialCharacter(23, 11, SpecialChars::TriangleDown, obj.Header.Symbol.Inactive);
        r.DrawVerticalLine(24, 11, 13, obj.Lines.Inactive);

        r.ResetClip();
    }

    void PaintCursors(Graphics::Renderer& r, Size sz)
    {
        DrawPreviewWindow(r, 2, 1, sz.Width - 3, sz.Height - 2, " Headers ");
        r.SetClipMargins(3, 2, 3, 2);
        r.FillHorizontalLine(3, 2, sz.Width - 4, ' ', obj.Header.Text.Focused);
        r.WriteSingleLineText(3, 2, " Column               Column         Column   ", obj.Header.Text.Focused);
        r.WriteSingleLineText(3, 3, " Regular cursor         30             Red ", obj.Text.Normal);
        r.WriteSingleLineText(3, 4, " Normal item            20            Blue ", obj.Text.Normal);
        r.WriteSingleLineText(3, 5, " Inactive cursor        15           Green ", obj.Text.Normal);
        r.WriteSingleLineText(3, 6, " Normal item            20            Blue ", obj.Text.Normal);
        r.WriteSingleLineText(3, 7, " Over Inactive item     25            Blue ", obj.Text.Normal);
        r.WriteSingleLineText(3, 8, " Over selection         10           Black ", obj.Text.Normal);
        r.WriteSingleLineText(3, 9, " Normal item            20            Blue ", obj.Text.Normal);
        r.DrawVerticalLine(23, 2, sz.Height - 3, obj.Lines.Focused);
        r.DrawVerticalLine(38, 2, sz.Height - 3, obj.Lines.Focused);
        r.DrawVerticalLine(50, 2, sz.Height - 3, obj.Lines.Focused);
        r.FillHorizontalLine(3, 3, sz.Width - 4, -1, obj.Cursor.Normal);
        r.FillHorizontalLine(3, 5, sz.Width - 4, -1, obj.Cursor.Inactive);
        r.FillHorizontalLine(3, 7, sz.Width - 4, -1, obj.Cursor.OverInactiveItem);
        r.FillHorizontalLine(3, 8, sz.Width - 4, -1, obj.Cursor.OverSelection);
        r.ResetClip();
    }

    void PaintEditors(Graphics::Renderer& r, Size sz)
    {
        DrawPreviewWindow(r, 2, 1, sz.Width - 3, sz.Height - 2, " Headers ");
        r.SetClipMargins(3, 2, 3, 2);
        r.FillHorizontalLine(4, 3, sz.Width - 5, ' ', obj.Editor.Normal);
        r.WriteSingleLineText(8, 3, "Normal/regular text", obj.Editor.Normal);
        r.WriteSingleLineText(4, 3, "  7", obj.LineMarker.Normal);
        r.FillHorizontalLine(4, 5, sz.Width - 5, ' ', obj.Editor.Focused);
        r.WriteSingleLineText(5, 5, "Focused text", obj.Editor.Focused);
        r.FillHorizontalLine(4, 7, sz.Width - 5, ' ', obj.Editor.Hovered);
        r.WriteSingleLineText(8, 7, "Hovered text", obj.Editor.Hovered);
        r.WriteSingleLineText(4, 7, "  5", obj.LineMarker.Hovered);
        r.FillHorizontalLine(4, 9, sz.Width - 5, ' ', obj.Editor.Inactive);
        r.WriteSingleLineText(8, 9, "Inactive text", obj.Editor.Inactive);
        r.WriteSingleLineText(4, 9, "  4", obj.LineMarker.Inactive);
        r.FillRect(4, 11, sz.Width - 5, 13, ' ', obj.Editor.Focused);
        r.WriteSingleLineText(8, 11, "Some text", obj.Editor.Focused);
        r.WriteSingleLineText(8, 12, "in multiline", obj.Editor.Focused);
        r.WriteSingleLineText(8, 13, "mode", obj.Editor.Focused);
        r.WriteSingleLineText(11, 12, "multi", obj.Selection.Editor);
        r.WriteSingleLineText(4, 11, "  1", obj.LineMarker.Focused);
        r.WriteSingleLineText(4, 12, "  2", obj.LineMarker.Focused);
        r.WriteSingleLineText(4, 13, "  3", obj.LineMarker.Focused);
        r.ResetClip();
    }
    void PaintBordersAndLines(Graphics::Renderer& r, Size sz)
    {
        DrawPreviewWindow(r, 2, 1, sz.Width - 3, sz.Height - 2, " Borders & Lines ");
        r.SetClipMargins(3, 2, 3, 2);

        const auto wx = std::max<uint32>(9U, (sz.Width - 10) / 3);
        r.DrawRectSize(4 + (wx + 1) * 0, 3, wx, 3, obj.Border.Normal, LineType::Single);
        r.DrawRectSize(4 + (wx + 1) * 1, 3, wx, 3, obj.Border.Focused, LineType::Single);
        r.DrawRectSize(4 + (wx + 1) * 2, 3, wx, 3, obj.Border.Inactive, LineType::Single);
        r.DrawRectSize(4 + (wx + 1) * 0, 6, wx, 3, obj.Border.Hovered, LineType::Single);
        r.DrawRectSize(4 + (wx + 1) * 1, 6, wx, 3, obj.Border.PressedOrSelected, LineType::Single);

        r.WriteSingleLineText(5 + (wx + 1) * 0, 4, "Normal", obj.Text.Normal);
        r.WriteSingleLineText(5 + (wx + 1) * 1, 4, "Focused", obj.Text.Normal);
        r.WriteSingleLineText(5 + (wx + 1) * 2, 4, "Inactive", obj.Text.Normal);
        r.WriteSingleLineText(5 + (wx + 1) * 0, 7, "Hovered", obj.Text.Normal);
        r.WriteSingleLineText(5 + (wx + 1) * 1, 7, "Pressed", obj.Text.Normal);

        r.DrawRectSize(4 + (wx + 1) * 0, 9, wx, 3, obj.Border.Normal, LineType::Single);
        r.DrawRectSize(4 + (wx + 1) * 1, 9, wx, 3, obj.Border.Focused, LineType::Single);
        r.DrawRectSize(4 + (wx + 1) * 2, 9, wx, 3, obj.Border.Inactive, LineType::Single);
        r.DrawRectSize(4 + (wx + 1) * 0, 12, wx, 3, obj.Border.Hovered, LineType::Single);
        r.DrawRectSize(4 + (wx + 1) * 1, 12, wx, 3, obj.Border.PressedOrSelected, LineType::Single);

        r.DrawHorizontalLine(5 + (wx + 1) * 0, 10, 2 + wx + (wx + 1) * 0, obj.Lines.Normal);
        r.DrawHorizontalLine(5 + (wx + 1) * 1, 10, 2 + wx + (wx + 1) * 1, obj.Lines.Focused);
        r.DrawHorizontalLine(5 + (wx + 1) * 2, 10, 2 + wx + (wx + 1) * 2, obj.Lines.Inactive);
        r.DrawHorizontalLine(5 + (wx + 1) * 0, 13, 2 + wx + (wx + 1) * 0, obj.Lines.Hovered);
        r.DrawHorizontalLine(5 + (wx + 1) * 1, 13, 2 + wx + (wx + 1) * 1, obj.Lines.PressedOrSelected);

        r.ResetClip();
    }
    void PaintTabs(Graphics::Renderer& r, Size sz)
    {
        const auto w = sz.Width - 8;
        DrawPreviewWindow(r, 2, 1, sz.Width - 3, sz.Height - 2, " Tabs ");
        r.SetClipMargins(3, 2, 3, 2);

        r.FillRectSize(4, 4, w, 2, ' ', obj.Tab.Text.PressedOrSelected);
        r.WriteSingleLineText(5, 3, " Tab 1 ", obj.Tab.Text.PressedOrSelected, obj.Tab.HotKey.PressedOrSelected, 1);
        r.WriteSingleLineText(13, 3, " Tab 2 ", obj.Tab.Text.Focused, obj.Tab.HotKey.Focused, 5);
        r.WriteSingleLineText(21, 3, " Tab 3 ", obj.Tab.Text.Hovered, obj.Tab.HotKey.Hovered, 5);
        r.WriteSingleLineText(29, 3, " Tab 4 ", obj.Tab.Text.Focused, obj.Tab.HotKey.Focused, 5);
        r.WriteSingleLineText(5, 5, "Focused", obj.Text.Normal);

        r.FillRectSize(4, 8, w, 2, ' ', obj.Tab.Text.PressedOrSelected);
        r.WriteSingleLineText(5, 7, " Tab 1 ", obj.Tab.Text.PressedOrSelected, obj.Tab.HotKey.PressedOrSelected, 1);
        r.WriteSingleLineText(13, 7, " Tab 2 ", obj.Tab.Text.Normal, obj.Tab.HotKey.Normal, 5);
        r.WriteSingleLineText(21, 7, " Tab 3 ", obj.Tab.Text.Normal, obj.Tab.HotKey.Normal, 5);
        r.WriteSingleLineText(29, 7, " Tab 4 ", obj.Tab.Text.Normal, obj.Tab.HotKey.Normal, 5);
        r.WriteSingleLineText(5, 9, "Normal (not focused)", obj.Text.Normal);

        r.FillRectSize(4, 12, w, 2, ' ', obj.Tab.Text.Inactive);
        r.WriteSingleLineText(5, 11, " Tab 1 ", obj.Tab.Text.Inactive, obj.Tab.HotKey.Inactive, 1);
        r.WriteSingleLineText(13, 11, " Tab 2 ", obj.Tab.Text.Inactive, obj.Tab.HotKey.Inactive, 5);
        r.WriteSingleLineText(21, 11, " Tab 3 ", obj.Tab.Text.Inactive, obj.Tab.HotKey.Inactive, 5);
        r.WriteSingleLineText(29, 11, " Tab 4 ", obj.Tab.Text.Inactive, obj.Tab.HotKey.Inactive, 5);
        r.WriteSingleLineText(5, 13, "Inactive", obj.Text.Inactive);

        r.ResetClip();
    }
    void PaintTabsList(Graphics::Renderer& r, Size sz)
    {
        const auto w = 12;
        const auto h = sz.Height - 6;
        const auto y = sz.Height - 5;
        DrawPreviewWindow(r, 2, 1, sz.Width - 3, sz.Height - 2, " Tabs ");
        r.SetClipMargins(3, 2, 3, 2);

        r.FillRectSize(4, 4, w, h, ' ', obj.Tab.Text.PressedOrSelected);
        r.WriteSingleLineText(
              4, 3, " Tab 1      ", obj.Tab.ListText.PressedOrSelected, obj.Tab.ListHotKey.PressedOrSelected, 1);
        r.WriteSingleLineText(4, y + 0, " Tab 2      ", obj.Tab.ListText.Focused, obj.Tab.ListHotKey.Focused, 5);
        r.WriteSingleLineText(4, y + 1, " Tab 3      ", obj.Tab.ListText.Hovered, obj.Tab.ListHotKey.Hovered, 5);
        r.WriteSingleLineText(4, y + 2, " Tab 4      ", obj.Tab.ListText.Focused, obj.Tab.ListHotKey.Focused, 5);
        r.WriteSingleLineText(5, 5, "Focused", obj.Text.Normal);

        r.FillRectSize(5 + (w * 1), 4, w, h, ' ', obj.Tab.Text.PressedOrSelected);
        r.WriteSingleLineText(
              5 + (w * 1),
              3,
              " Tab 1      ",
              obj.Tab.ListText.PressedOrSelected,
              obj.Tab.ListHotKey.PressedOrSelected,
              1);
        r.WriteSingleLineText(
              5 + (w * 1), y + 0, " Tab 2      ", obj.Tab.ListText.Normal, obj.Tab.ListHotKey.Normal, 5);
        r.WriteSingleLineText(
              5 + (w * 1), y + 1, " Tab 3      ", obj.Tab.ListText.Normal, obj.Tab.ListHotKey.Normal, 5);
        r.WriteSingleLineText(
              5 + (w * 1), y + 2, " Tab 4      ", obj.Tab.ListText.Normal, obj.Tab.ListHotKey.Normal, 5);
        r.WriteSingleLineText(6 + (w * 1), 5, "Normal", obj.Text.Normal);

        r.FillRectSize(5 + (w * 2), 4, w, h, ' ', obj.Tab.Text.Inactive);
        r.WriteSingleLineText(
              5 + (w * 2), 3, " Tab 1        ", obj.Tab.ListText.Inactive, obj.Tab.ListHotKey.Inactive, 1);
        r.WriteSingleLineText(
              5 + (w * 2), y + 0, " Tab 2      ", obj.Tab.ListText.Inactive, obj.Tab.ListHotKey.Inactive, 5);
        r.WriteSingleLineText(
              5 + (w * 2), y + 1, " Tab 3      ", obj.Tab.ListText.Inactive, obj.Tab.ListHotKey.Inactive, 5);
        r.WriteSingleLineText(
              5 + (w * 2), y + 2, " Tab 4      ", obj.Tab.ListText.Inactive, obj.Tab.ListHotKey.Inactive, 5);
        r.WriteSingleLineText(6 + (w * 2), 5, "Inactive", obj.Text.Inactive);

        r.ResetClip();
    }
    void Paint(Graphics::Renderer& r, Size sz)
    {
        if (catID < static_cast<CatID>(CatType::Count))
        {
            switch (catID)
            {
            case static_cast<CatID>(CatType::None):
                r.Clear();
                break;
            case static_cast<CatID>(CatType::Desktop):
                PaintDesktop(r);
                break;
            case static_cast<CatID>(CatType::Menu):
                PaintDesktop(r);
                PaintMenusAndCommandBar(r, sz);
                break;
            case static_cast<CatID>(CatType::ParentMenu):
                PaintDesktop(r);
                PaintParentMenusAndCommandBar(r, sz);
                break;
            case static_cast<CatID>(CatType::Window):
                PaintDesktop(r);
                PaintWindow(r, sz);
                break;
            case static_cast<CatID>(CatType::ToolTip):
                PaintDesktop(r);
                PaintToolTip(r, sz);
                break;
            case static_cast<CatID>(CatType::ProgressBar):
                PaintDesktop(r);
                PaintProgressBar(r, sz);
                break;
            case static_cast<CatID>(CatType::Button):
                PaintDesktop(r);
                PaintButtons(r, sz);
                break;
            case static_cast<CatID>(CatType::Text):
                PaintDesktop(r);
                PaintTexts(r, sz);
                break;
            case static_cast<CatID>(CatType::ScrollBars):
                PaintDesktop(r);
                PaintScrollBars(r, sz);
                break;
            case static_cast<CatID>(CatType::Symbols):
                PaintDesktop(r);
                PaintSymbols(r, sz);
                break;
            case static_cast<CatID>(CatType::SearchBar):
                PaintDesktop(r);
                PaintSearchBar(r, sz);
                break;
            case static_cast<CatID>(CatType::Headers):
                PaintDesktop(r);
                PaintHeaders(r, sz);
                break;
            case static_cast<CatID>(CatType::Cursor):
                PaintDesktop(r);
                PaintCursors(r, sz);
                break;
            case static_cast<CatID>(CatType::Editor):
                PaintDesktop(r);
                PaintEditors(r, sz);
                break;
            case static_cast<CatID>(CatType::BorderAndLines):
                PaintDesktop(r);
                PaintBordersAndLines(r, sz);
                break;
            case static_cast<CatID>(CatType::Tabs):
                PaintDesktop(r);
                PaintTabs(r, sz);
                break;
            case static_cast<CatID>(CatType::TabsList):
                PaintDesktop(r);
                PaintTabsList(r, sz);
                break;
            }

            return;
        }

        auto it = categoriesData.find(catID);
        if (it != categoriesData.end())
        {
            auto& data = it->second;
            if (auto owner = data.categoryRef->previewInterface)
            {
                PaintDesktop(r);

                DrawPreviewWindow(r, 2, 1, sz.Width - 3, sz.Height - 2, data.categoryName);
                r.SetClipMargins(3, 2, 3, 2);

                auto newSize = Size{ sz.Width - 6, sz.Height - 4 };

                owner->OnPreviewWindowDraw(data.categoryName, r, 4, 3, newSize, data.categoryRef->data);
                return;
            }
        }
    }
    bool GetPropertyValue(uint32 propertyID, PropertyValue& value) override
    {
        if (propertyID < static_cast<PropID>(PropType::Count))
        {
            switch (static_cast<PropID>(propertyID))
            {
            case static_cast<PropID>(PropType::DesktopChar):
                value = (char16) 186;
                return true;
            case static_cast<PropID>(PropType::DesktopColor):
                value = obj.Symbol.Desktop;
                return true;

            // Menus
            case static_cast<PropID>(PropType::MenuTextNormal):
                value = obj.Menu.Text.Normal;
                return true;
            case static_cast<PropID>(PropType::MenuTextHovered):
                value = obj.Menu.Text.Hovered;
                return true;
            case static_cast<PropID>(PropType::MenuTextSelected):
                value = obj.Menu.Text.PressedOrSelected;
                return true;
            case static_cast<PropID>(PropType::MenuHotKeyNormal):
                value = obj.Menu.HotKey.Normal;
                return true;
            case static_cast<PropID>(PropType::MenuHotKeyHovered):
                value = obj.Menu.HotKey.Hovered;
                return true;
            case static_cast<PropID>(PropType::MenuHotKeySelected):
                value = obj.Menu.HotKey.PressedOrSelected;
                return true;
            case static_cast<PropID>(PropType::MenuShortCutNormal):
                value = obj.Menu.ShortCut.Normal;
                return true;
            case static_cast<PropID>(PropType::MenuShortCutHovered):
                value = obj.Menu.ShortCut.Hovered;
                return true;
            case static_cast<PropID>(PropType::MenuShortCutSelected):
                value = obj.Menu.ShortCut.PressedOrSelected;
                return true;
            case static_cast<PropID>(PropType::MenuInactive):
                value = obj.Menu.Text.Inactive.Foreground;
                return true;
            case static_cast<PropID>(PropType::MenuSymbolNormal):
                value = obj.Menu.Symbol.Normal.Foreground;
                return true;
            case static_cast<PropID>(PropType::MenuSymbolHovered):
                value = obj.Menu.Symbol.Hovered.Foreground;
                return true;
            case static_cast<PropID>(PropType::MenuSymbolSelected):
                value = obj.Menu.Symbol.PressedOrSelected.Foreground;
                return true;

            // Parent Menus
            case static_cast<PropID>(PropType::ParentMenuTextNormal):
                value = obj.ParentMenu.Text.Normal;
                return true;
            case static_cast<PropID>(PropType::ParentMenuTextHovered):
                value = obj.ParentMenu.Text.Hovered;
                return true;
            case static_cast<PropID>(PropType::ParentMenuHotKeyNormal):
                value = obj.ParentMenu.HotKey.Normal;
                return true;
            case static_cast<PropID>(PropType::ParentMenuHotKeyHovered):
                value = obj.ParentMenu.HotKey.Hovered;
                return true;
            case static_cast<PropID>(PropType::ParentMenuShortCutNormal):
                value = obj.ParentMenu.ShortCut.Normal;
                return true;
            case static_cast<PropID>(PropType::ParentMenuShortCutHovered):
                value = obj.ParentMenu.ShortCut.Hovered;
                return true;
            case static_cast<PropID>(PropType::ParentMenuInactive):
                value = obj.ParentMenu.Text.Inactive.Foreground;
                return true;
            case static_cast<PropID>(PropType::ParentMenuSymbolNormal):
                value = obj.ParentMenu.Symbol.Normal.Foreground;
                return true;
            case static_cast<PropID>(PropType::ParentMenuSymbolHovered):
                value = obj.ParentMenu.Symbol.Hovered.Foreground;
                return true;

            // Window
            case static_cast<PropID>(PropType::WindowNormal):
                value = obj.Window.Background.Normal;
                return true;
            case static_cast<PropID>(PropType::WindowInactive):
                value = obj.Window.Background.Inactive;
                return true;
            case static_cast<PropID>(PropType::WindowError):
                value = obj.Window.Background.Error;
                return true;
            case static_cast<PropID>(PropType::WindowInfo):
                value = obj.Window.Background.Info;
                return true;
            case static_cast<PropID>(PropType::WindowWarning):
                value = obj.Window.Background.Warning;
                return true;

            // Tooltip
            case static_cast<PropID>(PropType::ToolTipText):
                value = obj.ToolTip.Text;
                return true;
            case static_cast<PropID>(PropType::ToolTipArrow):
                value = obj.ToolTip.Arrow;
                return true;

            // ProgressBar
            case static_cast<PropID>(PropType::ProgressBarEmpty):
                value = obj.ProgressStatus.Empty.Background;
                return true;
            case static_cast<PropID>(PropType::ProgressBarFull):
                value = obj.ProgressStatus.Full.Background;
                return true;

            // Buttons
            case static_cast<PropID>(PropType::ButtonTextFocused):
                value = obj.Button.Text.Focused;
                return true;
            case static_cast<PropID>(PropType::ButtonTextNormal):
                value = obj.Button.Text.Normal;
                return true;
            case static_cast<PropID>(PropType::ButtonTextHovered):
                value = obj.Button.Text.Hovered;
                return true;
            case static_cast<PropID>(PropType::ButtonTextInactive):
                value = obj.Button.Text.Inactive;
                return true;
            case static_cast<PropID>(PropType::ButtonTextSelected):
                value = obj.Button.Text.PressedOrSelected;
                return true;
            case static_cast<PropID>(PropType::ButtonHotKeyFocused):
                value = obj.Button.HotKey.Focused;
                return true;
            case static_cast<PropID>(PropType::ButtonHotKeyNormal):
                value = obj.Button.HotKey.Normal;
                return true;
            case static_cast<PropID>(PropType::ButtonHotKeyHovered):
                value = obj.Button.HotKey.Hovered;
                return true;
            case static_cast<PropID>(PropType::ButtonHotKeyInactive):
                value = obj.Button.HotKey.Inactive;
                return true;
            case static_cast<PropID>(PropType::ButtonHotKeySelected):
                value = obj.Button.HotKey.PressedOrSelected;
                return true;
            case static_cast<PropID>(PropType::ButtonShadow):
                value = obj.Button.ShadowColor.Foreground;
                return true;

            // Text
            case static_cast<PropID>(PropType::TextNormal):
                value = obj.Text.Normal.Foreground;
                return true;
            case static_cast<PropID>(PropType::TextHotKey):
                value = obj.Text.HotKey.Foreground;
                return true;
            case static_cast<PropID>(PropType::TextInactive):
                value = obj.Text.Inactive.Foreground;
                return true;
            case static_cast<PropID>(PropType::TextError):
                value = obj.Text.Error.Foreground;
                return true;
            case static_cast<PropID>(PropType::TextWarning):
                value = obj.Text.Warning.Foreground;
                return true;
            case static_cast<PropID>(PropType::TextFocused):
                value = obj.Text.Focused.Foreground;
                return true;
            case static_cast<PropID>(PropType::TextHovered):
                value = obj.Text.Hovered.Foreground;
                return true;
            case static_cast<PropID>(PropType::TextHighlighted):
                value = obj.Text.Highlighted.Foreground;
                return true;
            case static_cast<PropID>(PropType::TextEmphasized1):
                value = obj.Text.Emphasized1.Foreground;
                return true;
            case static_cast<PropID>(PropType::TextEmphasized2):
                value = obj.Text.Emphasized2.Foreground;
                return true;

            // Scroll bars
            case static_cast<PropID>(PropType::ScrollBarButtonNormal):
                value = obj.ScrollBar.Arrows.Normal;
                return true;
            case static_cast<PropID>(PropType::ScrollBarButtonHovered):
                value = obj.ScrollBar.Arrows.Hovered;
                return true;
            case static_cast<PropID>(PropType::ScrollBarButtonPressed):
                value = obj.ScrollBar.Arrows.PressedOrSelected;
                return true;
            case static_cast<PropID>(PropType::ScrollBarButtonInactive):
                value = obj.ScrollBar.Arrows.Inactive;
                return true;
            case static_cast<PropID>(PropType::ScrollBarNormal):
                value = obj.ScrollBar.Bar.Normal;
                return true;
            case static_cast<PropID>(PropType::ScrollBarHovered):
                value = obj.ScrollBar.Bar.Hovered;
                return true;
            case static_cast<PropID>(PropType::ScrollBarPressed):
                value = obj.ScrollBar.Bar.PressedOrSelected;
                return true;
            case static_cast<PropID>(PropType::ScrollBarInactive):
                value = obj.ScrollBar.Bar.Inactive;
                return true;
            case static_cast<PropID>(PropType::ScrollBarPositionNormal):
                value = obj.ScrollBar.Position.Normal;
                return true;
            case static_cast<PropID>(PropType::ScrollBarPositionHovered):
                value = obj.ScrollBar.Position.Hovered;
                return true;
            case static_cast<PropID>(PropType::ScrollBarPositionPressed):
                value = obj.ScrollBar.Position.PressedOrSelected;
                return true;

            // Symbols
            case static_cast<PropID>(PropType::SymbolInactive):
                value = obj.Symbol.Inactive.Foreground;
                return true;
            case static_cast<PropID>(PropType::SymbolHovered):
                value = obj.Symbol.Hovered;
                return true;
            case static_cast<PropID>(PropType::SymbolPressed):
                value = obj.Symbol.Hovered;
                return true;
            case static_cast<PropID>(PropType::SymbolChecked):
                value = obj.Symbol.Checked.Foreground;
                return true;
            case static_cast<PropID>(PropType::SymbolUnchecked):
                value = obj.Symbol.Unchecked.Foreground;
                return true;
            case static_cast<PropID>(PropType::SymbolUnknown):
                value = obj.Symbol.Unknown.Foreground;
                return true;
            case static_cast<PropID>(PropType::SymbolArrows):
                value = obj.Symbol.Arrows.Foreground;
                return true;
            case static_cast<PropID>(PropType::SymbolClose):
                value = obj.Symbol.Close.Foreground;
                return true;
            case static_cast<PropID>(PropType::SymbolMaximized):
                value = obj.Symbol.Maximized.Foreground;
                return true;
            case static_cast<PropID>(PropType::SymbolResize):
                value = obj.Symbol.Resize.Foreground;
                return true;

            // SearchBar
            case static_cast<PropID>(PropType::SearchBarNormal):
                value = obj.SearchBar.Normal;
                return true;
            case static_cast<PropID>(PropType::SearchBarFocused):
                value = obj.SearchBar.Focused;
                return true;
            case static_cast<PropID>(PropType::SearchBarHovered):
                value = obj.SearchBar.Hovered;
                return true;
            case static_cast<PropID>(PropType::SearchBarInactive):
                value = obj.SearchBar.Inactive;
                return true;

            // Headers
            case static_cast<PropID>(PropType::HeaderTextNormal):
                value = obj.Header.Text.Normal;
                return true;
            case static_cast<PropID>(PropType::HeaderTextFocused):
                value = obj.Header.Text.Focused;
                return true;
            case static_cast<PropID>(PropType::HeaderTextInactive):
                value = obj.Header.Text.Inactive;
                return true;
            case static_cast<PropID>(PropType::HeaderTextHovered):
                value = obj.Header.Text.Hovered;
                return true;
            case static_cast<PropID>(PropType::HeaderTextSelected):
                value = obj.Header.Text.PressedOrSelected;
                return true;
            case static_cast<PropID>(PropType::HeaderHotKeyNormal):
                value = obj.Header.HotKey.Normal;
                return true;
            case static_cast<PropID>(PropType::HeaderHotKeyFocused):
                value = obj.Header.HotKey.Focused;
                return true;
            case static_cast<PropID>(PropType::HeaderHotKeyInactive):
                value = obj.Header.HotKey.Inactive;
                return true;
            case static_cast<PropID>(PropType::HeaderHotKeyHovered):
                value = obj.Header.HotKey.Hovered;
                return true;
            case static_cast<PropID>(PropType::HeaderHotKeySelected):
                value = obj.Header.HotKey.PressedOrSelected;
                return true;
            case static_cast<PropID>(PropType::HeaderSymbolNormal):
                value = obj.Header.Symbol.Normal;
                return true;
            case static_cast<PropID>(PropType::HeaderSymbolFocused):
                value = obj.Header.Symbol.Focused;
                return true;
            case static_cast<PropID>(PropType::HeaderSymbolInactive):
                value = obj.Header.Symbol.Inactive;
                return true;
            case static_cast<PropID>(PropType::HeaderSymbolHovered):
                value = obj.Header.Symbol.Hovered;
                return true;
            case static_cast<PropID>(PropType::HeaderSymbolSelected):
                value = obj.Header.Symbol.PressedOrSelected;
                return true;

            // Cursor
            case static_cast<PropID>(PropType::CursorNormal):
                value = obj.Cursor.Normal;
                return true;
            case static_cast<PropID>(PropType::CursorInactive):
                value = obj.Cursor.Inactive;
                return true;
            case static_cast<PropID>(PropType::CursorOverInactiveItem):
                value = obj.Cursor.OverInactiveItem;
                return true;
            case static_cast<PropID>(PropType::CursorOverSelection):
                value = obj.Cursor.OverSelection;
                return true;

            // Editor
            case static_cast<PropID>(PropType::EditorBackground):
                value = obj.Editor.Normal.Background;
                return true;
            case static_cast<PropID>(PropType::EditorNormal):
                value = obj.Editor.Normal.Foreground;
                return true;
            case static_cast<PropID>(PropType::EditorFocus):
                value = obj.Editor.Focused.Foreground;
                return true;
            case static_cast<PropID>(PropType::EditorInactive):
                value = obj.Editor.Inactive.Foreground;
                return true;
            case static_cast<PropID>(PropType::EditorHovered):
                value = obj.Editor.Hovered.Foreground;
                return true;
            case static_cast<PropID>(PropType::EditorSelection):
                value = obj.Selection.Editor;
                return true;
            case static_cast<PropID>(PropType::EditorLineMarkerNormal):
                value = obj.LineMarker.Normal;
                return true;
            case static_cast<PropID>(PropType::EditorLineMarkerFocused):
                value = obj.LineMarker.Focused;
                return true;
            case static_cast<PropID>(PropType::EditorLineMarkerInactive):
                value = obj.LineMarker.Inactive;
                return true;
            case static_cast<PropID>(PropType::EditorLineMarkerHovered):
                value = obj.LineMarker.Hovered;
                return true;

            // Border & Lines
            case static_cast<PropID>(PropType::BorderNormal):
                value = obj.Border.Normal.Foreground;
                return true;
            case static_cast<PropID>(PropType::BorderFocused):
                value = obj.Border.Focused.Foreground;
                return true;
            case static_cast<PropID>(PropType::BorderInactive):
                value = obj.Border.Inactive.Foreground;
                return true;
            case static_cast<PropID>(PropType::BorderHovered):
                value = obj.Border.Hovered.Foreground;
                return true;
            case static_cast<PropID>(PropType::BorderPressed):
                value = obj.Border.PressedOrSelected;
                return true;
            case static_cast<PropID>(PropType::LineNormal):
                value = obj.Lines.Normal.Foreground;
                return true;
            case static_cast<PropID>(PropType::LineFocused):
                value = obj.Lines.Focused.Foreground;
                return true;
            case static_cast<PropID>(PropType::LineInactive):
                value = obj.Lines.Inactive.Foreground;
                return true;
            case static_cast<PropID>(PropType::LineHovered):
                value = obj.Lines.Hovered;
                return true;
            case static_cast<PropID>(PropType::LinePressed):
                value = obj.Lines.PressedOrSelected;
                return true;

            // Tabs
            case static_cast<PropID>(PropType::TabsTextNormal):
                value = obj.Tab.Text.Normal;
                return true;
            case static_cast<PropID>(PropType::TabsTextFocused):
                value = obj.Tab.Text.Focused;
                return true;
            case static_cast<PropID>(PropType::TabsTextInactive):
                value = obj.Tab.Text.Inactive;
                return true;
            case static_cast<PropID>(PropType::TabsTextHovered):
                value = obj.Tab.Text.Hovered;
                return true;
            case static_cast<PropID>(PropType::TabsTextSelected):
                value = obj.Tab.Text.PressedOrSelected;
                return true;
            case static_cast<PropID>(PropType::TabsHotKeyNormal):
                value = obj.Tab.HotKey.Normal;
                return true;
            case static_cast<PropID>(PropType::TabsHotKeyFocused):
                value = obj.Tab.HotKey.Focused;
                return true;
            case static_cast<PropID>(PropType::TabsHotKeyInactive):
                value = obj.Tab.HotKey.Inactive;
                return true;
            case static_cast<PropID>(PropType::TabsHotKeyHovered):
                value = obj.Tab.HotKey.Hovered;
                return true;
            case static_cast<PropID>(PropType::TabsHotKeySelected):
                value = obj.Tab.HotKey.PressedOrSelected;
                return true;

            // Tabs list
            case static_cast<PropID>(PropType::TabsListTextNormal):
                value = obj.Tab.ListText.Normal;
                return true;
            case static_cast<PropID>(PropType::TabsListTextFocused):
                value = obj.Tab.ListText.Focused;
                return true;
            case static_cast<PropID>(PropType::TabsListTextInactive):
                value = obj.Tab.ListText.Inactive;
                return true;
            case static_cast<PropID>(PropType::TabsListTextHovered):
                value = obj.Tab.ListText.Hovered;
                return true;
            case static_cast<PropID>(PropType::TabsListTextSelected):
                value = obj.Tab.ListText.PressedOrSelected;
                return true;
            case static_cast<PropID>(PropType::TabsListHotKeyNormal):
                value = obj.Tab.ListHotKey.Normal;
                return true;
            case static_cast<PropID>(PropType::TabsListHotKeyFocused):
                value = obj.Tab.ListHotKey.Focused;
                return true;
            case static_cast<PropID>(PropType::TabsListHotKeyInactive):
                value = obj.Tab.ListHotKey.Inactive;
                return true;
            case static_cast<PropID>(PropType::TabsListHotKeyHovered):
                value = obj.Tab.ListHotKey.Hovered;
                return true;
            case static_cast<PropID>(PropType::TabsListHotKeySelected):
                value = obj.Tab.ListHotKey.PressedOrSelected;
                return true;
            }

            return false; // Unhandled property
        }
        if (GetCustomColorByPropertyId(propertyID, &value))
            return true;
        return false;
    }

    bool SetPropertyValue(uint32 propertyID, const PropertyValue& value, String& error) override
    {
        if (propertyID < static_cast<PropID>(PropType::Count))
        {
            switch (static_cast<PropID>(propertyID))
            {
            case static_cast<PropID>(PropType::DesktopColor):
                obj.Symbol.Desktop = std::get<ColorPair>(value);
                return true;

            case static_cast<PropID>(PropType::MenuTextNormal):
                obj.Menu.Text.Normal              = std::get<ColorPair>(value);
                obj.Menu.Symbol.Normal.Background = obj.Menu.Text.Normal.Background;
                obj.Menu.Text.Inactive.Background = obj.Menu.Text.Normal.Background;
                return true;
            case static_cast<PropID>(PropType::MenuTextHovered):
                obj.Menu.Text.Hovered              = std::get<ColorPair>(value);
                obj.Menu.Symbol.Hovered.Background = obj.Menu.Text.Hovered.Background;
                obj.Menu.Symbol.Hovered.Background = obj.Menu.Text.Hovered.Background;
                return true;
            case static_cast<PropID>(PropType::MenuTextSelected):
                obj.Menu.Text.PressedOrSelected              = std::get<ColorPair>(value);
                obj.Menu.Symbol.PressedOrSelected.Background = obj.Menu.Text.PressedOrSelected.Background;
                obj.Menu.Symbol.PressedOrSelected.Background = obj.Menu.Text.PressedOrSelected.Background;
                return true;
            case static_cast<PropID>(PropType::MenuHotKeyNormal):
                obj.Menu.HotKey.Normal              = std::get<ColorPair>(value);
                obj.Menu.HotKey.Inactive.Background = obj.Menu.HotKey.Normal.Background;
                return true;
            case static_cast<PropID>(PropType::MenuHotKeyHovered):
                obj.Menu.HotKey.Hovered = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::MenuHotKeySelected):
                obj.Menu.HotKey.PressedOrSelected = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::MenuShortCutNormal):
                obj.Menu.ShortCut.Normal              = std::get<ColorPair>(value);
                obj.Menu.ShortCut.Inactive.Background = obj.Menu.ShortCut.Normal.Background;
                return true;
            case static_cast<PropID>(PropType::MenuShortCutHovered):
                obj.Menu.ShortCut.Hovered = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::MenuShortCutSelected):
                obj.Menu.ShortCut.PressedOrSelected = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::MenuSymbolNormal):
                obj.Menu.Symbol.Normal.Foreground = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::MenuSymbolHovered):
                obj.Menu.Symbol.Hovered.Foreground = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::MenuSymbolSelected):
                obj.Menu.Symbol.PressedOrSelected.Foreground = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::MenuInactive):
                obj.Menu.Text.Inactive.Foreground     = std::get<Color>(value);
                obj.Menu.HotKey.Inactive.Foreground   = std::get<Color>(value);
                obj.Menu.ShortCut.Inactive.Foreground = std::get<Color>(value);
                obj.Menu.Symbol.Inactive.Foreground   = std::get<Color>(value);
                return true;

            case static_cast<PropID>(PropType::ParentMenuTextNormal):
                obj.ParentMenu.Text.Normal              = std::get<ColorPair>(value);
                obj.ParentMenu.Symbol.Normal.Background = obj.ParentMenu.Text.Normal.Background;
                obj.ParentMenu.Text.Inactive.Background = obj.ParentMenu.Text.Normal.Background;
                return true;
            case static_cast<PropID>(PropType::ParentMenuTextHovered):
                obj.ParentMenu.Text.Hovered              = std::get<ColorPair>(value);
                obj.ParentMenu.Symbol.Hovered.Background = obj.ParentMenu.Text.Hovered.Background;
                obj.ParentMenu.Symbol.Hovered.Background = obj.ParentMenu.Text.Hovered.Background;
                return true;
            case static_cast<PropID>(PropType::ParentMenuHotKeyNormal):
                obj.ParentMenu.HotKey.Normal              = std::get<ColorPair>(value);
                obj.ParentMenu.HotKey.Inactive.Background = obj.ParentMenu.HotKey.Normal.Background;
                return true;
            case static_cast<PropID>(PropType::ParentMenuHotKeyHovered):
                obj.ParentMenu.HotKey.Hovered = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::ParentMenuShortCutNormal):
                obj.ParentMenu.ShortCut.Normal              = std::get<ColorPair>(value);
                obj.ParentMenu.ShortCut.Inactive.Background = obj.ParentMenu.ShortCut.Normal.Background;
                return true;
            case static_cast<PropID>(PropType::ParentMenuShortCutHovered):
                obj.ParentMenu.ShortCut.Hovered = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::ParentMenuSymbolNormal):
                obj.ParentMenu.Symbol.Normal.Foreground = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::ParentMenuSymbolHovered):
                obj.ParentMenu.Symbol.Hovered.Foreground = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::ParentMenuInactive):
                obj.ParentMenu.Text.Inactive.Foreground     = std::get<Color>(value);
                obj.ParentMenu.HotKey.Inactive.Foreground   = std::get<Color>(value);
                obj.ParentMenu.ShortCut.Inactive.Foreground = std::get<Color>(value);
                obj.ParentMenu.Symbol.Inactive.Foreground   = std::get<Color>(value);
                return true;

            case static_cast<PropID>(PropType::WindowNormal):
                obj.Window.Background.Normal = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::WindowInactive):
                obj.Window.Background.Inactive = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::WindowError):
                obj.Window.Background.Error = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::WindowWarning):
                obj.Window.Background.Warning = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::WindowInfo):
                obj.Window.Background.Info = std::get<Color>(value);
                return true;

            case static_cast<PropID>(PropType::ToolTipText):
                obj.ToolTip.Text = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::ToolTipArrow):
                obj.ToolTip.Arrow = std::get<ColorPair>(value);
                return true;

            case static_cast<PropID>(PropType::ProgressBarEmpty):
                obj.ProgressStatus.Empty.Background = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::ProgressBarFull):
                obj.ProgressStatus.Full.Background = std::get<Color>(value);
                return true;

            case static_cast<PropID>(PropType::ButtonTextFocused):
                obj.Button.Text.Focused = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::ButtonTextNormal):
                obj.Button.Text.Normal = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::ButtonTextHovered):
                obj.Button.Text.Hovered = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::ButtonTextInactive):
                obj.Button.Text.Inactive = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::ButtonTextSelected):
                obj.Button.Text.PressedOrSelected = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::ButtonHotKeyFocused):
                obj.Button.HotKey.Focused = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::ButtonHotKeyNormal):
                obj.Button.HotKey.Normal = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::ButtonHotKeyHovered):
                obj.Button.HotKey.Hovered = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::ButtonHotKeyInactive):
                obj.Button.HotKey.Inactive = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::ButtonHotKeySelected):
                obj.Button.HotKey.PressedOrSelected = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::ButtonShadow):
                obj.Button.ShadowColor.Foreground = std::get<Color>(value);
                return true;

            // Text
            case static_cast<PropID>(PropType::TextNormal):
                obj.Text.Normal.Foreground = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::TextHotKey):
                obj.Text.HotKey.Foreground = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::TextInactive):
                obj.Text.Inactive.Foreground = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::TextError):
                obj.Text.Error.Foreground = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::TextWarning):
                obj.Text.Warning.Foreground = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::TextFocused):
                obj.Text.Focused.Foreground = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::TextHovered):
                obj.Text.Hovered.Foreground = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::TextHighlighted):
                obj.Text.Highlighted.Foreground = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::TextEmphasized1):
                obj.Text.Emphasized1.Foreground = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::TextEmphasized2):
                obj.Text.Emphasized2.Foreground = std::get<Color>(value);
                return true;

            case static_cast<PropID>(PropType::ScrollBarButtonNormal):
                obj.ScrollBar.Arrows.Normal = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::ScrollBarButtonHovered):
                obj.ScrollBar.Arrows.Hovered = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::ScrollBarButtonPressed):
                obj.ScrollBar.Arrows.PressedOrSelected = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::ScrollBarButtonInactive):
                obj.ScrollBar.Arrows.Inactive = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::ScrollBarNormal):
                obj.ScrollBar.Bar.Normal = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::ScrollBarHovered):
                obj.ScrollBar.Bar.Hovered = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::ScrollBarPressed):
                obj.ScrollBar.Bar.PressedOrSelected = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::ScrollBarInactive):
                obj.ScrollBar.Bar.Inactive = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::ScrollBarPositionNormal):
                obj.ScrollBar.Position.Normal = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::ScrollBarPositionHovered):
                obj.ScrollBar.Position.Hovered = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::ScrollBarPositionPressed):
                obj.ScrollBar.Position.PressedOrSelected = std::get<ColorPair>(value);
                return true;

            case static_cast<PropID>(PropType::SymbolInactive):
                obj.Symbol.Inactive.Foreground = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::SymbolHovered):
                obj.Symbol.Hovered = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::SymbolPressed):
                obj.Symbol.Pressed = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::SymbolChecked):
                obj.Symbol.Checked.Foreground = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::SymbolUnchecked):
                obj.Symbol.Unchecked.Foreground = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::SymbolUnknown):
                obj.Symbol.Unknown.Foreground = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::SymbolArrows):
                obj.Symbol.Arrows.Foreground = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::SymbolClose):
                obj.Symbol.Close.Foreground = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::SymbolMaximized):
                obj.Symbol.Maximized.Foreground = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::SymbolResize):
                obj.Symbol.Resize.Foreground = std::get<Color>(value);
                return true;

            case static_cast<PropID>(PropType::SearchBarNormal):
                obj.SearchBar.Normal = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::SearchBarFocused):
                obj.SearchBar.Focused = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::SearchBarHovered):
                obj.SearchBar.Hovered = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::SearchBarInactive):
                obj.SearchBar.Inactive = std::get<ColorPair>(value);
                return true;

            case static_cast<PropID>(PropType::HeaderTextNormal):
                obj.Header.Text.Normal = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::HeaderTextFocused):
                obj.Header.Text.Focused = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::HeaderTextInactive):
                obj.Header.Text.Inactive = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::HeaderTextHovered):
                obj.Header.Text.Hovered = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::HeaderTextSelected):
                obj.Header.Text.PressedOrSelected = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::HeaderHotKeyNormal):
                obj.Header.HotKey.Normal = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::HeaderHotKeyFocused):
                obj.Header.HotKey.Focused = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::HeaderHotKeyInactive):
                obj.Header.HotKey.Inactive = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::HeaderHotKeyHovered):
                obj.Header.HotKey.Hovered = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::HeaderHotKeySelected):
                obj.Header.HotKey.PressedOrSelected = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::HeaderSymbolNormal):
                obj.Header.Symbol.Normal = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::HeaderSymbolFocused):
                obj.Header.Symbol.Focused = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::HeaderSymbolInactive):
                obj.Header.Symbol.Inactive = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::HeaderSymbolHovered):
                obj.Header.Symbol.Hovered = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::HeaderSymbolSelected):
                obj.Header.Symbol.PressedOrSelected = std::get<ColorPair>(value);
                return true;

            // Cursor
            case static_cast<PropID>(PropType::CursorNormal):
                obj.Cursor.Normal = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::CursorInactive):
                obj.Cursor.Inactive = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::CursorOverInactiveItem):
                obj.Cursor.OverInactiveItem = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::CursorOverSelection):
                obj.Cursor.OverSelection = std::get<ColorPair>(value);
                return true;

            // Editor
            case static_cast<PropID>(PropType::EditorBackground):
                obj.Editor.Normal.Background  = std::get<Color>(value);
                obj.Editor.Focused.Background = std::get<Color>(value);
                obj.Editor.Hovered.Background = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::EditorNormal):
                obj.Editor.Normal.Foreground = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::EditorFocus):
                obj.Editor.Focused.Foreground = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::EditorInactive):
                obj.Editor.Inactive.Foreground = std::get<Color>(value);
                obj.Editor.Inactive.Background = Color::Transparent;
                return true;
            case static_cast<PropID>(PropType::EditorHovered):
                obj.Editor.Hovered.Foreground = std::get<Color>(value);
                return true;
            case static_cast<PropID>(PropType::EditorSelection):
                obj.Selection.Editor = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::EditorLineMarkerNormal):
                obj.LineMarker.Normal = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::EditorLineMarkerFocused):
                obj.LineMarker.Focused = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::EditorLineMarkerInactive):
                obj.LineMarker.Inactive = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::EditorLineMarkerHovered):
                obj.LineMarker.Hovered = std::get<ColorPair>(value);
                return true;

            case static_cast<PropID>(PropType::BorderNormal):
                obj.Border.Normal = { std::get<Color>(value), Color::Transparent };
                return true;
            case static_cast<PropID>(PropType::BorderFocused):
                obj.Border.Focused = { std::get<Color>(value), Color::Transparent };
                return true;
            case static_cast<PropID>(PropType::BorderInactive):
                obj.Border.Inactive = { std::get<Color>(value), Color::Transparent };
                return true;
            case static_cast<PropID>(PropType::BorderHovered):
                obj.Border.Hovered = { std::get<Color>(value), Color::Transparent };
                return true;
            case static_cast<PropID>(PropType::BorderPressed):
                obj.Border.PressedOrSelected = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::LineNormal):
                obj.Lines.Normal = { std::get<Color>(value), Color::Transparent };
                return true;
            case static_cast<PropID>(PropType::LineFocused):
                obj.Lines.Focused = { std::get<Color>(value), Color::Transparent };
                return true;
            case static_cast<PropID>(PropType::LineInactive):
                obj.Lines.Inactive = { std::get<Color>(value), Color::Transparent };
                return true;
            case static_cast<PropID>(PropType::LineHovered):
                obj.Lines.Hovered = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::LinePressed):
                obj.Lines.PressedOrSelected = std::get<ColorPair>(value);
                return true;

            case static_cast<PropID>(PropType::TabsTextNormal):
                obj.Tab.Text.Normal = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::TabsTextFocused):
                obj.Tab.Text.Focused = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::TabsTextInactive):
                obj.Tab.Text.Inactive = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::TabsTextHovered):
                obj.Tab.Text.Hovered = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::TabsTextSelected):
                obj.Tab.Text.PressedOrSelected = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::TabsHotKeyNormal):
                obj.Tab.HotKey.Normal = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::TabsHotKeyFocused):
                obj.Tab.HotKey.Focused = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::TabsHotKeyInactive):
                obj.Tab.HotKey.Inactive = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::TabsHotKeyHovered):
                obj.Tab.HotKey.Hovered = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::TabsHotKeySelected):
                obj.Tab.HotKey.PressedOrSelected = std::get<ColorPair>(value);
                return true;

            case static_cast<PropID>(PropType::TabsListTextNormal):
                obj.Tab.ListText.Normal = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::TabsListTextFocused):
                obj.Tab.ListText.Focused = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::TabsListTextInactive):
                obj.Tab.ListText.Inactive = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::TabsListTextHovered):
                obj.Tab.ListText.Hovered = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::TabsListTextSelected):
                obj.Tab.ListText.PressedOrSelected = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::TabsListHotKeyNormal):
                obj.Tab.ListHotKey.Normal = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::TabsListHotKeyFocused):
                obj.Tab.ListHotKey.Focused = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::TabsListHotKeyInactive):
                obj.Tab.ListHotKey.Inactive = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::TabsListHotKeyHovered):
                obj.Tab.ListHotKey.Hovered = std::get<ColorPair>(value);
                return true;
            case static_cast<PropID>(PropType::TabsListHotKeySelected):
                obj.Tab.ListHotKey.PressedOrSelected = std::get<ColorPair>(value);
                return true;
            }
            error.SetFormat("Invalid default property id (%d)", propertyID);
            return false;
        }

        if (GetCustomColorByPropertyId(propertyID, nullptr, &value))
            return true;

        return false;
    }

    bool GetCustomColorByPropertyId(
          PropID propertyID, PropertyValue* valueToObtain = nullptr, const PropertyValue* valueToSet = nullptr)
    {
        if (propertyID < static_cast<PropID>(PropType::Count))
            return false;
        auto it = customProperties.find(propertyID);
        if (it == customProperties.end())
            return false;

        auto& propData = it->second;
        if (propData.color->IsColorState())
        {
            auto colorState = propData.color->TryGetColorState();
            assert(colorState != nullptr);
            assert(propData.stateIndex.has_value() && propData.stateIndex.value() < OBJECT_COLOR_STATE_COUNT);
            if (!propData.stateIndex.has_value()) // should not happen but return on release and abort on debug
                return false;
            auto& specifiedColor = colorState->StatesList[propData.stateIndex.value()];
            if (valueToObtain)
            {
                *valueToObtain = specifiedColor;
            }
            if (valueToSet)
            {
                auto newColorState                                    = *colorState;
                newColorState.StatesList[propData.stateIndex.value()] = std::get<ColorPair>(*valueToSet);
                *it->second.color                                     = CustomColor(newColorState);
            }
            return true;
        }
        if (propData.color->IsColorPair())
        {
            auto colorPair = propData.color->TryGetColorPair();
            assert(colorPair != nullptr);
            if (valueToObtain)
                *valueToObtain = *colorPair;
            if (valueToSet)
                *it->second.color = CustomColor(std::get<ColorPair>(*valueToSet));
            return true;
        }
        assert(false && "Only ColorPair and ColorState are supported for custom colors!");
        return false;
    }

    void SetCustomPropertyValue(uint32 /*propertyID*/) override
    {
    }
    bool IsPropertyValueReadOnly(uint32 /*propertyID*/) override
    {
        NOT_IMPLEMENTED(false);
    }
    const vector<Property> GetPropertiesList() override
    {
#define PT(t)  static_cast<uint32>(t)
#define CAT(t) catNames[static_cast<uint32>(t)]
        vector<Property> properties = {
            { PT(PropType::DesktopChar), CAT(CatType::Desktop), "Symbol", PropertyType::Char16 },
            { PT(PropType::DesktopColor), CAT(CatType::Desktop), "Color", PropertyType::ColorPair },
            // Menus
            { PT(PropType::MenuTextNormal), CAT(CatType::Menu), "Text (normal)", PropertyType::ColorPair },
            { PT(PropType::MenuTextHovered), CAT(CatType::Menu), "Text (hovered)", PropertyType::ColorPair },
            { PT(PropType::MenuTextSelected), CAT(CatType::Menu), "Text (selected)", PropertyType::ColorPair },
            { PT(PropType::MenuHotKeyNormal), CAT(CatType::Menu), "HotKey (normal)", PropertyType::ColorPair },
            { PT(PropType::MenuHotKeyHovered), CAT(CatType::Menu), "HotKey (hovered)", PropertyType::ColorPair },
            { PT(PropType::MenuHotKeySelected), CAT(CatType::Menu), "HotKey (selected)", PropertyType::ColorPair },
            { PT(PropType::MenuShortCutNormal), CAT(CatType::Menu), "ShortCut (normal)", PropertyType::ColorPair },
            { PT(PropType::MenuShortCutHovered), CAT(CatType::Menu), "ShortCut (hovered)", PropertyType::ColorPair },
            { PT(PropType::MenuShortCutSelected), CAT(CatType::Menu), "ShortCut (selected)", PropertyType::ColorPair },
            { PT(PropType::MenuInactive), CAT(CatType::Menu), "Inactive", PropertyType::Color },
            { PT(PropType::MenuSymbolNormal), CAT(CatType::Menu), "Symbols (normal)", PropertyType::Color },
            { PT(PropType::MenuSymbolSelected), CAT(CatType::Menu), "Symbols (selected)", PropertyType::Color },
            { PT(PropType::MenuSymbolHovered), CAT(CatType::Menu), "Symbols (hovered)", PropertyType::Color },
            // parent menu
            { PT(PropType::ParentMenuTextNormal), CAT(CatType::ParentMenu), "Text (normal)", PropertyType::ColorPair },
            { PT(PropType::ParentMenuTextHovered), CAT(CatType::ParentMenu), "Text (hovered)", PropertyType::ColorPair },
            { PT(PropType::ParentMenuHotKeyNormal), CAT(CatType::ParentMenu), "HotKey (normal)", PropertyType::ColorPair },
            { PT(PropType::ParentMenuHotKeyHovered),
              CAT(CatType::ParentMenu),
              "HotKey (hovered)",
              PropertyType::ColorPair },
            { PT(PropType::ParentMenuShortCutNormal),
              CAT(CatType::ParentMenu),
              "ShortCut (normal)",
              PropertyType::ColorPair },
            { PT(PropType::ParentMenuShortCutHovered),
              CAT(CatType::ParentMenu),
              "ShortCut (hovered)",
              PropertyType::ColorPair },
            { PT(PropType::ParentMenuInactive), CAT(CatType::ParentMenu), "Inactive", PropertyType::Color },
            { PT(PropType::ParentMenuSymbolNormal), CAT(CatType::ParentMenu), "Symbols (normal)", PropertyType::Color },
            { PT(PropType::ParentMenuSymbolHovered), CAT(CatType::ParentMenu), "Symbols (hovered)", PropertyType::Color },
            // Window
            { PT(PropType::WindowNormal), CAT(CatType::Window), "Regular", PropertyType::Color },
            { PT(PropType::WindowInactive), CAT(CatType::Window), "Inactive", PropertyType::Color },
            { PT(PropType::WindowError), CAT(CatType::Window), "Error", PropertyType::Color },
            { PT(PropType::WindowInfo), CAT(CatType::Window), "Notification", PropertyType::Color },
            { PT(PropType::WindowWarning), CAT(CatType::Window), "Warning", PropertyType::Color },
            // ToolTip
            { PT(PropType::ToolTipText), CAT(CatType::ToolTip), "Text", PropertyType::ColorPair },
            { PT(PropType::ToolTipArrow), CAT(CatType::ToolTip), "Arrow", PropertyType::ColorPair },
            // Progress Bar
            { PT(PropType::ProgressBarEmpty), CAT(CatType::ProgressBar), "Empty", PropertyType::Color },
            { PT(PropType::ProgressBarFull), CAT(CatType::ProgressBar), "Full", PropertyType::Color },
            // Buttons
            { PT(PropType::ButtonTextFocused), CAT(CatType::Button), "Text (focused)", PropertyType::ColorPair },
            { PT(PropType::ButtonTextNormal), CAT(CatType::Button), "Text (normal)", PropertyType::ColorPair },
            { PT(PropType::ButtonTextHovered), CAT(CatType::Button), "Text (hovered)", PropertyType::ColorPair },
            { PT(PropType::ButtonTextSelected), CAT(CatType::Button), "Text (pressed)", PropertyType::ColorPair },
            { PT(PropType::ButtonTextInactive), CAT(CatType::Button), "Text (inactive)", PropertyType::ColorPair },
            { PT(PropType::ButtonHotKeyFocused), CAT(CatType::Button), "HotKey (focused)", PropertyType::ColorPair },
            { PT(PropType::ButtonHotKeyNormal), CAT(CatType::Button), "HotKey (normal)", PropertyType::ColorPair },
            { PT(PropType::ButtonHotKeyHovered), CAT(CatType::Button), "HotKey (hovered)", PropertyType::ColorPair },
            { PT(PropType::ButtonHotKeySelected), CAT(CatType::Button), "HotKey (pressed)", PropertyType::ColorPair },
            { PT(PropType::ButtonHotKeyInactive), CAT(CatType::Button), "HotKey (inactive)", PropertyType::ColorPair },
            { PT(PropType::ButtonShadow), CAT(CatType::Button), "Shaddow", PropertyType::Color },
            // Text
            { PT(PropType::TextNormal), CAT(CatType::Text), "Regular", PropertyType::Color },
            { PT(PropType::TextHotKey), CAT(CatType::Text), "Hot Key", PropertyType::Color },
            { PT(PropType::TextInactive), CAT(CatType::Text), "Inactive", PropertyType::Color },
            { PT(PropType::TextError), CAT(CatType::Text), "Error", PropertyType::Color },
            { PT(PropType::TextWarning), CAT(CatType::Text), "Warning", PropertyType::Color },
            { PT(PropType::TextFocused), CAT(CatType::Text), "Focused", PropertyType::Color },
            { PT(PropType::TextHovered), CAT(CatType::Text), "HOvered", PropertyType::Color },
            { PT(PropType::TextHighlighted), CAT(CatType::Text), "Highlighted", PropertyType::Color },
            { PT(PropType::TextEmphasized1), CAT(CatType::Text), "Emphasized (1)", PropertyType::Color },
            { PT(PropType::TextEmphasized2), CAT(CatType::Text), "Emphasized (2)", PropertyType::Color },
            // Scroll Bar
            { PT(PropType::ScrollBarButtonNormal), CAT(CatType::ScrollBars), "Buttons (normal)", PropertyType::ColorPair },
            { PT(PropType::ScrollBarButtonHovered),
              CAT(CatType::ScrollBars),
              "Buttons (hovered)",
              PropertyType::ColorPair },
            { PT(PropType::ScrollBarButtonPressed),
              CAT(CatType::ScrollBars),
              "Buttons (pressed)",
              PropertyType::ColorPair },
            { PT(PropType::ScrollBarButtonInactive),
              CAT(CatType::ScrollBars),
              "Buttons (Inactive)",
              PropertyType::ColorPair },
            { PT(PropType::ScrollBarNormal), CAT(CatType::ScrollBars), "Bar (normal)", PropertyType::ColorPair },
            { PT(PropType::ScrollBarHovered), CAT(CatType::ScrollBars), "Bar (hovered)", PropertyType::ColorPair },
            { PT(PropType::ScrollBarPressed), CAT(CatType::ScrollBars), "Bar (pressed)", PropertyType::ColorPair },
            { PT(PropType::ScrollBarInactive), CAT(CatType::ScrollBars), "Bar (Inactive)", PropertyType::ColorPair },
            { PT(PropType::ScrollBarPositionNormal),
              CAT(CatType::ScrollBars),
              "Position (normal)",
              PropertyType::ColorPair },
            { PT(PropType::ScrollBarPositionHovered),
              CAT(CatType::ScrollBars),
              "Position (hovered)",
              PropertyType::ColorPair },
            { PT(PropType::ScrollBarPositionPressed),
              CAT(CatType::ScrollBars),
              "Position (pressed)",
              PropertyType::ColorPair },
            // symbols
            { PT(PropType::SymbolInactive), CAT(CatType::Symbols), "Inactive", PropertyType::Color },
            { PT(PropType::SymbolHovered), CAT(CatType::Symbols), "Hovered", PropertyType::ColorPair },
            { PT(PropType::SymbolPressed), CAT(CatType::Symbols), "Pressed", PropertyType::ColorPair },
            { PT(PropType::SymbolChecked), CAT(CatType::Symbols), "Check", PropertyType::Color },
            { PT(PropType::SymbolUnchecked), CAT(CatType::Symbols), "Uncheck", PropertyType::Color },
            { PT(PropType::SymbolUnknown), CAT(CatType::Symbols), "Unknown", PropertyType::Color },
            { PT(PropType::SymbolArrows), CAT(CatType::Symbols), "Arrows", PropertyType::Color },
            { PT(PropType::SymbolClose), CAT(CatType::Symbols), "Windows close", PropertyType::Color },
            { PT(PropType::SymbolMaximized), CAT(CatType::Symbols), "Window maximize", PropertyType::Color },
            { PT(PropType::SymbolResize), CAT(CatType::Symbols), "Window resize", PropertyType::Color },

            // search bar
            { PT(PropType::SearchBarNormal), CAT(CatType::SearchBar), "Regular", PropertyType::ColorPair },
            { PT(PropType::SearchBarFocused), CAT(CatType::SearchBar), "Focused", PropertyType::ColorPair },
            { PT(PropType::SearchBarHovered), CAT(CatType::SearchBar), "Hovered", PropertyType::ColorPair },
            { PT(PropType::SearchBarInactive), CAT(CatType::SearchBar), "Inactive", PropertyType::ColorPair },

            // Header
            { PT(PropType::HeaderTextNormal), CAT(CatType::Headers), "Text (regular)", PropertyType::ColorPair },
            { PT(PropType::HeaderTextFocused), CAT(CatType::Headers), "Text (focused)", PropertyType::ColorPair },
            { PT(PropType::HeaderTextInactive), CAT(CatType::Headers), "Text (inactive)", PropertyType::ColorPair },
            { PT(PropType::HeaderTextHovered), CAT(CatType::Headers), "Text (hovered)", PropertyType::ColorPair },
            { PT(PropType::HeaderTextSelected), CAT(CatType::Headers), "Text (pressed)", PropertyType::ColorPair },
            { PT(PropType::HeaderHotKeyNormal), CAT(CatType::Headers), "HotKey (regular)", PropertyType::ColorPair },
            { PT(PropType::HeaderHotKeyFocused), CAT(CatType::Headers), "HotKey (focused)", PropertyType::ColorPair },
            { PT(PropType::HeaderHotKeyInactive), CAT(CatType::Headers), "HotKey (inactive)", PropertyType::ColorPair },
            { PT(PropType::HeaderHotKeyHovered), CAT(CatType::Headers), "HotKey (hovered)", PropertyType::ColorPair },
            { PT(PropType::HeaderHotKeySelected), CAT(CatType::Headers), "HotKey (pressed)", PropertyType::ColorPair },
            { PT(PropType::HeaderSymbolNormal), CAT(CatType::Headers), "Symbol (regular)", PropertyType::ColorPair },
            { PT(PropType::HeaderSymbolFocused), CAT(CatType::Headers), "Symbol (focused)", PropertyType::ColorPair },
            { PT(PropType::HeaderSymbolInactive), CAT(CatType::Headers), "Symbol (inactive)", PropertyType::ColorPair },
            { PT(PropType::HeaderSymbolHovered), CAT(CatType::Headers), "Symbol (hovered)", PropertyType::ColorPair },
            { PT(PropType::HeaderSymbolSelected), CAT(CatType::Headers), "Symbol (pressed)", PropertyType::ColorPair },

            // Cursor
            { PT(PropType::CursorNormal), CAT(CatType::Cursor), "Normal", PropertyType::ColorPair },
            { PT(PropType::CursorInactive), CAT(CatType::Cursor), "Inactive", PropertyType::ColorPair },
            { PT(PropType::CursorOverInactiveItem), CAT(CatType::Cursor), "Over inactive item", PropertyType::ColorPair },
            { PT(PropType::CursorOverSelection), CAT(CatType::Cursor), "Over selection", PropertyType::ColorPair },

            // Editor
            { PT(PropType::EditorBackground), CAT(CatType::Editor), "Background", PropertyType::Color },
            { PT(PropType::EditorNormal), CAT(CatType::Editor), "Text (normal)", PropertyType::Color },
            { PT(PropType::EditorInactive), CAT(CatType::Editor), "Text (inactive)", PropertyType::Color },
            { PT(PropType::EditorFocus), CAT(CatType::Editor), "Text (focused)", PropertyType::Color },
            { PT(PropType::EditorHovered), CAT(CatType::Editor), "Text (selected)", PropertyType::Color },
            { PT(PropType::EditorSelection), CAT(CatType::Editor), "Selection", PropertyType::ColorPair },
            { PT(PropType::EditorLineMarkerNormal), CAT(CatType::Editor), "Line (normal)", PropertyType::ColorPair },
            { PT(PropType::EditorLineMarkerFocused), CAT(CatType::Editor), "Line (focused)", PropertyType::ColorPair },
            { PT(PropType::EditorLineMarkerInactive), CAT(CatType::Editor), "Line (inactive)", PropertyType::ColorPair },
            { PT(PropType::EditorLineMarkerHovered), CAT(CatType::Editor), "Line (hovered)", PropertyType::ColorPair },

            // Border & Lines
            { PT(PropType::BorderNormal), CAT(CatType::BorderAndLines), "Border (normal)", PropertyType::Color },
            { PT(PropType::BorderFocused), CAT(CatType::BorderAndLines), "Border (focused)", PropertyType::Color },
            { PT(PropType::BorderInactive), CAT(CatType::BorderAndLines), "Border (inactive)", PropertyType::Color },
            { PT(PropType::BorderHovered), CAT(CatType::BorderAndLines), "Border (hovered)", PropertyType::Color },
            { PT(PropType::BorderPressed), CAT(CatType::BorderAndLines), "Border (pressed)", PropertyType::ColorPair },
            { PT(PropType::LineNormal), CAT(CatType::BorderAndLines), "Line (normal)", PropertyType::Color },
            { PT(PropType::LineFocused), CAT(CatType::BorderAndLines), "Line (focused)", PropertyType::Color },
            { PT(PropType::LineInactive), CAT(CatType::BorderAndLines), "Line (inactive)", PropertyType::Color },
            { PT(PropType::LineHovered), CAT(CatType::BorderAndLines), "Line (hovered)", PropertyType::ColorPair },
            { PT(PropType::LinePressed), CAT(CatType::BorderAndLines), "Line (pressed)", PropertyType::ColorPair },

            // Tabs
            { PT(PropType::TabsTextNormal), CAT(CatType::Tabs), "Text (normal)", PropertyType::ColorPair },
            { PT(PropType::TabsTextFocused), CAT(CatType::Tabs), "Text (focused)", PropertyType::ColorPair },
            { PT(PropType::TabsTextInactive), CAT(CatType::Tabs), "Text (inactive)", PropertyType::ColorPair },
            { PT(PropType::TabsTextHovered), CAT(CatType::Tabs), "Text (hovered)", PropertyType::ColorPair },
            { PT(PropType::TabsTextSelected), CAT(CatType::Tabs), "Text (selected)", PropertyType::ColorPair },
            { PT(PropType::TabsHotKeyNormal), CAT(CatType::Tabs), "HotKey (normal)", PropertyType::ColorPair },
            { PT(PropType::TabsHotKeyFocused), CAT(CatType::Tabs), "HotKey (focused)", PropertyType::ColorPair },
            { PT(PropType::TabsHotKeyInactive), CAT(CatType::Tabs), "HotKey (inactive)", PropertyType::ColorPair },
            { PT(PropType::TabsHotKeyHovered), CAT(CatType::Tabs), "HotKey (hovered)", PropertyType::ColorPair },
            { PT(PropType::TabsHotKeySelected), CAT(CatType::Tabs), "HotKey (selected)", PropertyType::ColorPair },

            // Tabs (lists)
            { PT(PropType::TabsListTextNormal), CAT(CatType::TabsList), "Text (normal)", PropertyType::ColorPair },
            { PT(PropType::TabsListTextFocused), CAT(CatType::TabsList), "Text (focused)", PropertyType::ColorPair },
            { PT(PropType::TabsListTextInactive), CAT(CatType::TabsList), "Text (inactive)", PropertyType::ColorPair },
            { PT(PropType::TabsListTextHovered), CAT(CatType::TabsList), "Text (hovered)", PropertyType::ColorPair },
            { PT(PropType::TabsListTextSelected), CAT(CatType::TabsList), "Text (selected)", PropertyType::ColorPair },
            { PT(PropType::TabsListHotKeyNormal), CAT(CatType::TabsList), "HotKey (normal)", PropertyType::ColorPair },
            { PT(PropType::TabsListHotKeyFocused), CAT(CatType::TabsList), "HotKey (focused)", PropertyType::ColorPair },
            { PT(PropType::TabsListHotKeyInactive), CAT(CatType::TabsList), "HotKey (inactive)", PropertyType::ColorPair },
            { PT(PropType::TabsListHotKeyHovered), CAT(CatType::TabsList), "HotKey (hovered)", PropertyType::ColorPair },
            { PT(PropType::TabsListHotKeySelected), CAT(CatType::TabsList), "HotKey (selected)", PropertyType::ColorPair },
        };
#undef PT
#undef CAT
        properties.reserve(properties.size() + customProperties.size());

        for (const auto& [propId, propData] : customProperties)
        {
            properties.emplace_back(propId, propData.category, propData.colorName, PropertyType::ColorPair);
        }
        return properties;
    }
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
        Size sz = GetSize();
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
    Reference<ComboBox> previewWindow;
    Reference<ComboBox> themeMode;

  public:
    ThemeEditorDialog(const AppCUI::Application::Config& configObject)
        : Window("Theme editor", "d:c,w:80,h:25", WindowFlags::Sizeable), cfg(configObject)
    {
        auto sp = Factory::Splitter::Create(this, "l:1,t:3,b:3,r:0", SplitterFlags::Vertical);
        sp->SetSecondPanelSize(30);
        pc = sp->CreateChildControl<PreviewControl>();
        pc->SetConfig(&cfg);
        prop = Factory::PropertyList::Create(sp, "d:c", &cfg, PropertyListFlags::None);
        UpdateCategoryAndProperty();
        Factory::Button::Create(this, "&Close", "r:1,b:0,w:12", BUTTON_CMD_CLOSE);
        Factory::Button::Create(this, "&Apply", "r:14,b:0,w:12", BUTTON_CMD_APPLY);
        Factory::Button::Create(this, "&Save...", "r:27,b:0,w:12", BUTTON_CMD_SAVE);
        Factory::Button::Create(this, "&Load...", "r:40,b:0,w:12", BUTTON_CMD_LOAD);
        Factory::Label::Create(this, "Preview &Window", "x:1,y:1,w:15");
        previewWindow = Factory::ComboBox::Create(this, "x:17,y:1,w:18", "Normal,Inactive,Error,Warning,Notification");
        previewWindow->SetCurentItemIndex(0);
        previewWindow->SetHotKey('W');
        Factory::Label::Create(this, "&Theme mode", "x:40,y:1,w:11");
        themeMode = Factory::ComboBox::Create(this, "l:53,r:1,t:1");
        themeMode->AddItem("Default", static_cast<uint64>(Application::ThemeType::Default));
        themeMode->AddItem("Dark", static_cast<uint64>(Application::ThemeType::Dark));
        themeMode->AddItem("Light", static_cast<uint64>(Application::ThemeType::Light));
        themeMode->SetCurentItemIndex(0);
        themeMode->SetHotKey('T');
        cfg.SetPreviewWindowID(PreviewWindowID::Normal);

        switch (cfg.GetConfig().Theme)
        {
        case Application::ThemeType::Default:
            themeMode->SetCurentItemIndex(0);
            break;
        case Application::ThemeType::Dark:
            themeMode->SetCurentItemIndex(1);
            break;
        case Application::ThemeType::Light:
            themeMode->SetCurentItemIndex(2);
            break;
        default:
            break;
        }
    }
    void UpdateCategoryAndProperty()
    {
        auto propID = prop->GetCurrentItemID();
        if (propID.has_value())
        {
            cfg.SetCategoryAndProperty(prop->GetCurrentItemCategory(), static_cast<PropID>(propID.value()));
        }
        else
        {
            cfg.SetCategoryAndProperty(prop->GetCurrentItemCategory(), static_cast<uint32>(PropType::None));
        }
    }

    void SaveTheme()
    {
        auto [themeFolder, fileName] = GetThemesFolderAndCurrentFileName();
        auto res = FileDialog::ShowSaveFileWindow(fileName, "Theme:theme", themeFolder);
        if (res.has_value())
        {
            auto file = res.value();
            if (!file.has_extension())
                file += ".theme";
            if (Internal::Config::Save(this->cfg.GetConfig(), file) == false)
            {
                MessageBox::ShowError("Save", "Fail to save theme file !");
            }
            else
            {
                MessageBox::ShowNotification("Save", "Theme file saved !");
            }
        }
    }
    void LoadTheme()
    {
        auto [themeFolder, fileName] = GetThemesFolderAndCurrentFileName();
        auto res = FileDialog::ShowOpenFileWindow(fileName, "Theme:theme", themeFolder);
        if (res.has_value())
        {
            if (Internal::Config::Load(this->cfg.GetConfig(), res.value()) == false)
            {
                MessageBox::ShowError("Load", "Fail to load theme from file !");
            }
        }
        cfg.ReInitializeFields();
    }

    void OnApplyTheme()
    {
        (*Application::GetAppConfig()) = cfg.GetConfig();
        Application::GetApplication()->TriggerThemeChange();
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
                this->Exit(Dialogs::Result::Cancel);
                return true;
            case BUTTON_CMD_APPLY:
                OnApplyTheme();
                this->Exit(Dialogs::Result::Ok);
                return true;
            case BUTTON_CMD_SAVE:
                SaveTheme();
                return true;
            case BUTTON_CMD_LOAD:
                LoadTheme();
                return true;
            }
        }
        if (eventType == Event::PropertyItemChanged)
        {
            UpdateCategoryAndProperty();
            return true;
        }
        if (eventType == Event::ComboBoxSelectedItemChanged)
        {
            if (control == previewWindow)
            {
                cfg.SetPreviewWindowID(static_cast<PreviewWindowID>(previewWindow->GetCurrentItemIndex()));
                return true;
            }
            if (control == themeMode)
            {
                const auto themeID = static_cast<AppCUI::Application::ThemeType>(themeMode->GetCurrentItemUserData(0));
                Internal::Config::SetTheme(cfg.GetConfig(), themeID);
                return true;
            }
        }
        return false;
    }
    bool OnKeyEvent(Input::Key keyCode, char16 charCode) override
    {
        if (Window::OnKeyEvent(keyCode, charCode))
            return true;
#ifdef SETUP_TO_CPP_MODE
        if (keyCode == Input::Key::F1)
        {
            AppCUI::Utils::String temp;
            temp.Create(8192);
            CreateCPPCode(cfg.GetConfig(), temp);
            AppCUI::OS::Clipboard::SetText(temp);
            return true;
        }
#endif
        return false;
    }
private:

    std::tuple<std::filesystem::path, std::string> GetThemesFolderAndCurrentFileName()
    {
        auto themeFolder = cfg.GetConfig().ThemesFolder;
        if (themeFolder.empty())
            themeFolder = OS::GetCurrentApplicationPath().parent_path();
        auto themeName = themeMode->GetCurrentItemText();
        themeName.Add(".theme");
        std::string fileName;
        themeName.ToString(fileName);

        return { themeFolder, fileName };
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

bool ThemeEditor::RegisterCustomColors(
      std::string category_name,
      Application::Config::CustomColorNameStorage colors,
      OnThemePreviewWindowDrawInterface* previewInterface)
{
    auto app = Application::GetApplication();
    CHECK(app, false, "Application has not been initialized !");
    CHECK(app->Inited, false, "Application has not been correctly initialized !");

    if (category_name.find(' ') != std::string::npos)
    {
        MessageBox::ShowError("Error", "You cannot have spaces inside the color category name!");
        return false;
    }

    if (colors.empty())
    {
        RETURNERROR(false, "No colors specified for category '%s'!", category_name.c_str());
    }

    for (const auto& [colorName, color]: colors)
    {
        if (colorName.find(' ') != std::string::npos)
        {
            MessageBox::ShowError("Error", "You cannot have spaces inside the color name!");
            return false;
        }
    }

    if (catNames->find(category_name) != std::string::npos)
    {
        RETURNERROR(false, "Category '%s' is already registered in default category names!", category_name.c_str());
    }

    auto& config = app->config;
    auto& colorsCategory = config.CustomColors[category_name];
    if (colorsCategory.previewInterface)
    {
        RETURNERROR(false, "Category '%s' is already registered in custom config colors !", category_name.c_str());
    }

    for (auto& [colorName, color] : colors)
    {
        if (colorsCategory.data.contains(colorName))
            continue;
        colorsCategory.data[colorName] = color;
    }
    colorsCategory.previewInterface = previewInterface;
    return true;
}

void ThemeEditor::RemovePreviewDrawListener(OnThemePreviewWindowDrawInterface* previewInterface)
{
    auto app = Application::GetApplication();
    if (!app || !app->Inited)
        return;

    auto& config = app->config;
    for (auto& [catName, catData] : config.CustomColors)
    {
        if (catData.previewInterface == previewInterface)
        {
            catData.previewInterface = nullptr;
            // no break in case the same listener is registered for multiple categories
        }
    }

    //app->RemoveListener(previewInterface);
}

bool ThemeEditor::RegisterOnThemeChangeCallback(OnThemeChangedInterface* listener)
{
    if (!listener)
        return false;
    auto app = Application::GetApplication();
    CHECK(app, false, "Application has not been initialized !");
    CHECK(app->Inited, false, "Application has not been correctly initialized !");
    return app->RegisterThemeChangeListener(listener);

}

void ThemeEditor::RemoveOnThemeChangeCallback(OnThemeChangedInterface* listener)
{
    if (!listener)
        return;
    auto app = Application::GetApplication();
    if (!app || !app->Inited)
        return;
    app->RemoveThemeChangeListener(listener);
}

} // namespace AppCUI::Dialogs