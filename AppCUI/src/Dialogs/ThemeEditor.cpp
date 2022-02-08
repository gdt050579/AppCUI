#include "AppCUI.hpp"

using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;

namespace AppCUI::Dialogs
{
constexpr int BUTTON_CMD_CLOSE = 1;

enum class PreviewWindowID : uint32
{
    Normal = 0,
    Inactive,
    Error,
    Warning,
    Notification
};
enum class CatID : uint32
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

    Count // must be the last one
};
constexpr string_view catNames[static_cast<uint32>(CatID::Count)] = {
    "",     "Desktop",     "Menu",    "Menu (parent)", "Window",  "ToolTip", "Progress Bar", "Buttons",
    "Text", "Scroll bars", "Symbols", "SearchBar",     "Headers", "Cursor",  "Editor",
};

enum class PropID : uint32
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
    ButtonTextNormal,
    ButtonTextHovered,
    ButtonTextSelected,
    ButtonTextInactive,
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
};
class ConfigProperty : public PropertiesInterface
{
    AppCUI::Application::Config obj;
    CatID catID;
    PropID propID;
    PreviewWindowID windowID;

  public:
    ConfigProperty(const AppCUI::Application::Config& config) : obj(config), catID(CatID::None), propID(PropID::None)
    {
    }
    void SetCategoryAndProperty(string_view name, PropID pID)
    {
        propID = pID;
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
    void PaintWindow(Graphics::Renderer& r, Size sz)
    {
        if ((sz.Width < 4) || (sz.Height < 4))
            return;
        switch (propID)
        {
        case PropID::WindowError:
            DrawWindow(r, 2, 1, sz.Width - 3, sz.Height - 2, " Error ", obj.Window.Background.Error);
            break;
        case PropID::WindowInfo:
            DrawWindow(r, 2, 1, sz.Width - 3, sz.Height - 2, " Info ", obj.Window.Background.Info);
            break;
        case PropID::WindowWarning:
            DrawWindow(r, 2, 1, sz.Width - 3, sz.Height - 2, " Warning ", obj.Window.Background.Warning);
            break;
        case PropID::WindowInactive:
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
        y = (int) (sz.Height / 2) - 3;
        DrawPreviewWindow(r, x - 2, y - 2, x + 27, y + 4, " Buttons ");
        PaintOneButton(r, x, y, "  Regular  ", ControlState::Normal, true);
        PaintOneButton(r, x, y + 2, "  Hovered  ", ControlState::Hovered, true);
        PaintOneButton(r, x + 14, y, "  Inactiv  ", ControlState::Inactive, true);
        PaintOneButton(r, x + 14, y + 2, "  Pressed  ", ControlState::PressedOrSelected, false);
    }
    void PaintTexts(Graphics::Renderer& r, Size sz)
    {
        DrawPreviewWindow(r, 2, 3, sz.Width - 3, sz.Height - 3, " Texts ");
        r.WriteSingleLineText(4, 4, "Regular text", obj.Text.Normal, obj.Text.HotKey, 0);
        r.WriteSingleLineText(4, 5, "Inactive text", obj.Text.Inactive);
        r.WriteSingleLineText(4, 6, "Hovered text", obj.Text.Hovered);
        r.WriteSingleLineText(4, 7, "Focused text", obj.Text.Hovered);

        r.WriteSingleLineText(20, 4, "Error messages", obj.Text.Error);
        r.WriteSingleLineText(20, 5, "Warning messages", obj.Text.Warning);
        r.WriteSingleLineText(20, 6, "Error messages", obj.Text.Error);
        r.WriteSingleLineText(20, 7, "Highlighted text", obj.Text.Highlighted);
        r.WriteSingleLineText(20, 8, "Emphasized text (1)", obj.Text.Emphasized1);
        r.WriteSingleLineText(20, 9, "Emphasized text (12)", obj.Text.Emphasized2);
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
    void PaintScrollBars(Graphics::Renderer& r, Size sz)
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
        r.WriteSingleLineText(5, 3, "Normal/regular text", obj.Editor.Normal);
        r.FillHorizontalLine(4, 5, sz.Width - 5, ' ', obj.Editor.Focused);
        r.WriteSingleLineText(5, 5, "Focused text", obj.Editor.Focused);
        r.FillHorizontalLine(4, 7, sz.Width - 5, ' ', obj.Editor.Hovered);
        r.WriteSingleLineText(5, 7, "Hovered text", obj.Editor.Hovered);
        r.FillHorizontalLine(4, 9, sz.Width - 5, ' ', obj.Editor.Inactive);
        r.WriteSingleLineText(5, 9, "Inactive text", obj.Editor.Inactive);
        r.FillRect(4, 11, sz.Width - 5, 13, ' ', obj.Editor.Focused);
        r.WriteSingleLineText(10, 11, "Some text", obj.Editor.Focused);
        r.WriteSingleLineText(10, 12, "in multiline", obj.Editor.Focused);
        r.WriteSingleLineText(10, 13, "mode", obj.Editor.Focused);
        r.WriteSingleLineText(13, 12, "multi", obj.Selection.Editor);

        r.ResetClip();
    }

    void Paint(Graphics::Renderer& r, Size sz)
    {
        switch (catID)
        {
        case CatID::None:
            r.Clear();
            break;
        case CatID::Desktop:
            PaintDesktop(r);
            break;
        case CatID::Menu:
            PaintDesktop(r);
            PaintMenusAndCommandBar(r, sz);
            break;
        case CatID::ParentMenu:
            PaintDesktop(r);
            PaintParentMenusAndCommandBar(r, sz);
            break;
        case CatID::Window:
            PaintDesktop(r);
            PaintWindow(r, sz);
            break;
        case CatID::ToolTip:
            PaintDesktop(r);
            PaintToolTip(r, sz);
            break;
        case CatID::ProgressBar:
            PaintDesktop(r);
            PaintProgressBar(r, sz);
            break;
        case CatID::Button:
            PaintDesktop(r);
            PaintButtons(r, sz);
            break;
        case CatID::Text:
            PaintDesktop(r);
            PaintTexts(r, sz);
            break;
        case CatID::ScrollBars:
            PaintDesktop(r);
            PaintScrollBars(r, sz);
            break;
        case CatID::Symbols:
            PaintDesktop(r);
            PaintSymbols(r, sz);
            break;
        case CatID::SearchBar:
            PaintDesktop(r);
            PaintSearchBar(r, sz);
            break;
        case CatID::Headers:
            PaintDesktop(r);
            PaintHeaders(r, sz);
            break;
        case CatID::Cursor:
            PaintDesktop(r);
            PaintCursors(r, sz);
            break;
        case CatID::Editor:
            PaintDesktop(r);
            PaintEditors(r, sz);
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
        case PropID::ParentMenuHotKeyNormal:
            value = obj.ParentMenu.HotKey.Normal;
            return true;
        case PropID::ParentMenuHotKeyHovered:
            value = obj.ParentMenu.HotKey.Hovered;
            return true;
        case PropID::ParentMenuShortCutNormal:
            value = obj.ParentMenu.ShortCut.Normal;
            return true;
        case PropID::ParentMenuShortCutHovered:
            value = obj.ParentMenu.ShortCut.Hovered;
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

        // Window
        case PropID::WindowNormal:
            value = obj.Window.Background.Normal;
            return true;
        case PropID::WindowInactive:
            value = obj.Window.Background.Inactive;
            return true;
        case PropID::WindowError:
            value = obj.Window.Background.Error;
            return true;
        case PropID::WindowInfo:
            value = obj.Window.Background.Info;
            return true;
        case PropID::WindowWarning:
            value = obj.Window.Background.Warning;
            return true;

        // Tooltip
        case PropID::ToolTipText:
            value = obj.ToolTip.Text;
            return true;
        case PropID::ToolTipArrow:
            value = obj.ToolTip.Arrow;
            return true;

        // ProgressBar
        case PropID::ProgressBarEmpty:
            value = obj.ProgressStatus.Empty.Background;
            return true;
        case PropID::ProgressBarFull:
            value = obj.ProgressStatus.Full.Background;
            return true;

        // Buttons
        case PropID::ButtonTextNormal:
            value = obj.Button.Text.Normal;
            return true;
        case PropID::ButtonTextHovered:
            value = obj.Button.Text.Hovered;
            return true;
        case PropID::ButtonTextInactive:
            value = obj.Button.Text.Inactive;
            return true;
        case PropID::ButtonTextSelected:
            value = obj.Button.Text.PressedOrSelected;
            return true;
        case PropID::ButtonHotKeyNormal:
            value = obj.Button.HotKey.Normal;
            return true;
        case PropID::ButtonHotKeyHovered:
            value = obj.Button.HotKey.Hovered;
            return true;
        case PropID::ButtonHotKeyInactive:
            value = obj.Button.HotKey.Inactive;
            return true;
        case PropID::ButtonHotKeySelected:
            value = obj.Button.HotKey.PressedOrSelected;
            return true;
        case PropID::ButtonShadow:
            value = obj.Button.ShadowColor.Foreground;
            return true;

        // Text
        case PropID::TextNormal:
            value = obj.Text.Normal.Foreground;
            return true;
        case PropID::TextHotKey:
            value = obj.Text.HotKey.Foreground;
            return true;
        case PropID::TextInactive:
            value = obj.Text.Inactive.Foreground;
            return true;
        case PropID::TextError:
            value = obj.Text.Error.Foreground;
            return true;
        case PropID::TextWarning:
            value = obj.Text.Warning.Foreground;
            return true;
        case PropID::TextFocused:
            value = obj.Text.Focused.Foreground;
            return true;
        case PropID::TextHovered:
            value = obj.Text.Hovered.Foreground;
            return true;
        case PropID::TextHighlighted:
            value = obj.Text.Highlighted.Foreground;
            return true;
        case PropID::TextEmphasized1:
            value = obj.Text.Emphasized1.Foreground;
            return true;
        case PropID::TextEmphasized2:
            value = obj.Text.Emphasized2.Foreground;
            return true;

        // Scroll bars
        case PropID::ScrollBarButtonNormal:
            value = obj.ScrollBar.Arrows.Normal;
            return true;
        case PropID::ScrollBarButtonHovered:
            value = obj.ScrollBar.Arrows.Hovered;
            return true;
        case PropID::ScrollBarButtonPressed:
            value = obj.ScrollBar.Arrows.PressedOrSelected;
            return true;
        case PropID::ScrollBarButtonInactive:
            value = obj.ScrollBar.Arrows.Inactive;
            return true;
        case PropID::ScrollBarNormal:
            value = obj.ScrollBar.Bar.Normal;
            return true;
        case PropID::ScrollBarHovered:
            value = obj.ScrollBar.Bar.Hovered;
            return true;
        case PropID::ScrollBarPressed:
            value = obj.ScrollBar.Bar.PressedOrSelected;
            return true;
        case PropID::ScrollBarInactive:
            value = obj.ScrollBar.Bar.Inactive;
            return true;
        case PropID::ScrollBarPositionNormal:
            value = obj.ScrollBar.Position.Normal;
            return true;
        case PropID::ScrollBarPositionHovered:
            value = obj.ScrollBar.Position.Hovered;
            return true;
        case PropID::ScrollBarPositionPressed:
            value = obj.ScrollBar.Position.PressedOrSelected;
            return true;

        // Symbols
        case PropID::SymbolInactive:
            value = obj.Symbol.Inactive.Foreground;
            return true;
        case PropID::SymbolHovered:
            value = obj.Symbol.Hovered;
            return true;
        case PropID::SymbolPressed:
            value = obj.Symbol.Hovered;
            return true;
        case PropID::SymbolChecked:
            value = obj.Symbol.Checked.Foreground;
            return true;
        case PropID::SymbolUnchecked:
            value = obj.Symbol.Unchecked.Foreground;
            return true;
        case PropID::SymbolUnknown:
            value = obj.Symbol.Unknown.Foreground;
            return true;
        case PropID::SymbolArrows:
            value = obj.Symbol.Arrows.Foreground;
            return true;
        case PropID::SymbolClose:
            value = obj.Symbol.Close.Foreground;
            return true;
        case PropID::SymbolMaximized:
            value = obj.Symbol.Maximized.Foreground;
            return true;
        case PropID::SymbolResize:
            value = obj.Symbol.Resize.Foreground;
            return true;

        // SearchBar
        case PropID::SearchBarNormal:
            value = obj.SearchBar.Normal;
            return true;
        case PropID::SearchBarFocused:
            value = obj.SearchBar.Focused;
            return true;
        case PropID::SearchBarHovered:
            value = obj.SearchBar.Hovered;
            return true;
        case PropID::SearchBarInactive:
            value = obj.SearchBar.Inactive;
            return true;

        // Headers
        case PropID::HeaderTextNormal:
            value = obj.Header.Text.Normal;
            return true;
        case PropID::HeaderTextFocused:
            value = obj.Header.Text.Focused;
            return true;
        case PropID::HeaderTextInactive:
            value = obj.Header.Text.Inactive;
            return true;
        case PropID::HeaderTextHovered:
            value = obj.Header.Text.Hovered;
            return true;
        case PropID::HeaderTextSelected:
            value = obj.Header.Text.PressedOrSelected;
            return true;
        case PropID::HeaderHotKeyNormal:
            value = obj.Header.HotKey.Normal;
            return true;
        case PropID::HeaderHotKeyFocused:
            value = obj.Header.HotKey.Focused;
            return true;
        case PropID::HeaderHotKeyInactive:
            value = obj.Header.HotKey.Inactive;
            return true;
        case PropID::HeaderHotKeyHovered:
            value = obj.Header.HotKey.Hovered;
            return true;
        case PropID::HeaderHotKeySelected:
            value = obj.Header.HotKey.PressedOrSelected;
            return true;
        case PropID::HeaderSymbolNormal:
            value = obj.Header.Symbol.Normal;
            return true;
        case PropID::HeaderSymbolFocused:
            value = obj.Header.Symbol.Focused;
            return true;
        case PropID::HeaderSymbolInactive:
            value = obj.Header.Symbol.Inactive;
            return true;
        case PropID::HeaderSymbolHovered:
            value = obj.Header.Symbol.Hovered;
            return true;
        case PropID::HeaderSymbolSelected:
            value = obj.Header.Symbol.PressedOrSelected;
            return true;

        // Cursor
        case PropID::CursorNormal:
            value = obj.Cursor.Normal;
            return true;
        case PropID::CursorInactive:
            value = obj.Cursor.Inactive;
            return true;
        case PropID::CursorOverInactiveItem:
            value = obj.Cursor.OverInactiveItem;
            return true;
        case PropID::CursorOverSelection:
            value = obj.Cursor.OverSelection;
            return true;

        // Editor
        case PropID::EditorBackground:
            value = obj.Editor.Normal.Background;
            return true;
        case PropID::EditorNormal:
            value = obj.Editor.Normal.Foreground;
            return true;
        case PropID::EditorFocus:
            value = obj.Editor.Focused.Foreground;
            return true;
        case PropID::EditorInactive:
            value = obj.Editor.Inactive.Foreground;
            return true;
        case PropID::EditorHovered:
            value = obj.Editor.Hovered.Foreground;
            return true;
        case PropID::EditorSelection:
            value = obj.Selection.Editor;
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
        case PropID::ParentMenuHotKeyNormal:
            obj.ParentMenu.HotKey.Normal              = std::get<ColorPair>(value);
            obj.ParentMenu.HotKey.Inactive.Background = obj.ParentMenu.HotKey.Normal.Background;
            return true;
        case PropID::ParentMenuHotKeyHovered:
            obj.ParentMenu.HotKey.Hovered = std::get<ColorPair>(value);
            return true;
        case PropID::ParentMenuShortCutNormal:
            obj.ParentMenu.ShortCut.Normal              = std::get<ColorPair>(value);
            obj.ParentMenu.ShortCut.Inactive.Background = obj.ParentMenu.ShortCut.Normal.Background;
            return true;
        case PropID::ParentMenuShortCutHovered:
            obj.ParentMenu.ShortCut.Hovered = std::get<ColorPair>(value);
            return true;
        case PropID::ParentMenuSymbolNormal:
            obj.ParentMenu.Symbol.Normal.Foreground = std::get<Color>(value);
            return true;
        case PropID::ParentMenuSymbolHovered:
            obj.ParentMenu.Symbol.Hovered.Foreground = std::get<Color>(value);
            return true;
        case PropID::ParentMenuInactive:
            obj.ParentMenu.Text.Inactive.Foreground     = std::get<Color>(value);
            obj.ParentMenu.HotKey.Inactive.Foreground   = std::get<Color>(value);
            obj.ParentMenu.ShortCut.Inactive.Foreground = std::get<Color>(value);
            obj.ParentMenu.Symbol.Inactive.Foreground   = std::get<Color>(value);
            return true;

        case PropID::WindowNormal:
            obj.Window.Background.Normal = std::get<Color>(value);
            return true;
        case PropID::WindowInactive:
            obj.Window.Background.Inactive = std::get<Color>(value);
            return true;
        case PropID::WindowError:
            obj.Window.Background.Error = std::get<Color>(value);
            return true;
        case PropID::WindowWarning:
            obj.Window.Background.Warning = std::get<Color>(value);
            return true;
        case PropID::WindowInfo:
            obj.Window.Background.Info = std::get<Color>(value);
            return true;

        case PropID::ToolTipText:
            obj.ToolTip.Text = std::get<ColorPair>(value);
            return true;
        case PropID::ToolTipArrow:
            obj.ToolTip.Arrow = std::get<ColorPair>(value);
            return true;

        case PropID::ProgressBarEmpty:
            obj.ProgressStatus.Empty.Background = std::get<Color>(value);
            return true;
        case PropID::ProgressBarFull:
            obj.ProgressStatus.Full.Background = std::get<Color>(value);
            return true;

        case PropID::ButtonTextNormal:
            obj.Button.Text.Normal = std::get<ColorPair>(value);
            return true;
        case PropID::ButtonTextHovered:
            obj.Button.Text.Hovered = std::get<ColorPair>(value);
            return true;
        case PropID::ButtonTextInactive:
            obj.Button.Text.Inactive = std::get<ColorPair>(value);
            return true;
        case PropID::ButtonTextSelected:
            obj.Button.Text.PressedOrSelected = std::get<ColorPair>(value);
            return true;
        case PropID::ButtonHotKeyNormal:
            obj.Button.HotKey.Normal = std::get<ColorPair>(value);
            return true;
        case PropID::ButtonHotKeyHovered:
            obj.Button.HotKey.Hovered = std::get<ColorPair>(value);
            return true;
        case PropID::ButtonHotKeyInactive:
            obj.Button.HotKey.Inactive = std::get<ColorPair>(value);
            return true;
        case PropID::ButtonHotKeySelected:
            obj.Button.HotKey.PressedOrSelected = std::get<ColorPair>(value);
            return true;
        case PropID::ButtonShadow:
            obj.Button.ShadowColor.Foreground = std::get<Color>(value);
            return true;

        // Text
        case PropID::TextNormal:
            obj.Text.Normal.Foreground = std::get<Color>(value);
            return true;
        case PropID::TextHotKey:
            obj.Text.HotKey.Foreground = std::get<Color>(value);
            return true;
        case PropID::TextInactive:
            obj.Text.Inactive.Foreground = std::get<Color>(value);
            return true;
        case PropID::TextError:
            obj.Text.Error.Foreground = std::get<Color>(value);
            return true;
        case PropID::TextWarning:
            obj.Text.Warning.Foreground = std::get<Color>(value);
            return true;
        case PropID::TextFocused:
            obj.Text.Focused.Foreground = std::get<Color>(value);
            return true;
        case PropID::TextHovered:
            obj.Text.Hovered.Foreground = std::get<Color>(value);
            return true;
        case PropID::TextHighlighted:
            obj.Text.Highlighted.Foreground = std::get<Color>(value);
            return true;
        case PropID::TextEmphasized1:
            obj.Text.Emphasized1.Foreground = std::get<Color>(value);
            return true;
        case PropID::TextEmphasized2:
            obj.Text.Emphasized2.Foreground = std::get<Color>(value);
            return true;

        case PropID::ScrollBarButtonNormal:
            obj.ScrollBar.Arrows.Normal = std::get<ColorPair>(value);
            return true;
        case PropID::ScrollBarButtonHovered:
            obj.ScrollBar.Arrows.Hovered = std::get<ColorPair>(value);
            return true;
        case PropID::ScrollBarButtonPressed:
            obj.ScrollBar.Arrows.PressedOrSelected = std::get<ColorPair>(value);
            return true;
        case PropID::ScrollBarButtonInactive:
            obj.ScrollBar.Arrows.Inactive = std::get<ColorPair>(value);
            return true;
        case PropID::ScrollBarNormal:
            obj.ScrollBar.Bar.Normal = std::get<ColorPair>(value);
            return true;
        case PropID::ScrollBarHovered:
            obj.ScrollBar.Bar.Hovered = std::get<ColorPair>(value);
            return true;
        case PropID::ScrollBarPressed:
            obj.ScrollBar.Bar.PressedOrSelected = std::get<ColorPair>(value);
            return true;
        case PropID::ScrollBarInactive:
            obj.ScrollBar.Bar.Inactive = std::get<ColorPair>(value);
            return true;
        case PropID::ScrollBarPositionNormal:
            obj.ScrollBar.Position.Normal = std::get<ColorPair>(value);
            return true;
        case PropID::ScrollBarPositionHovered:
            obj.ScrollBar.Position.Hovered = std::get<ColorPair>(value);
            return true;
        case PropID::ScrollBarPositionPressed:
            obj.ScrollBar.Position.PressedOrSelected = std::get<ColorPair>(value);
            return true;

        case PropID::SymbolInactive:
            obj.Symbol.Inactive.Foreground = std::get<Color>(value);
            return true;
        case PropID::SymbolHovered:
            obj.Symbol.Hovered = std::get<ColorPair>(value);
            return true;
        case PropID::SymbolPressed:
            obj.Symbol.Pressed = std::get<ColorPair>(value);
            return true;
        case PropID::SymbolChecked:
            obj.Symbol.Checked.Foreground = std::get<Color>(value);
            return true;
        case PropID::SymbolUnchecked:
            obj.Symbol.Unchecked.Foreground = std::get<Color>(value);
            return true;
        case PropID::SymbolUnknown:
            obj.Symbol.Unknown.Foreground = std::get<Color>(value);
            return true;
        case PropID::SymbolArrows:
            obj.Symbol.Arrows.Foreground = std::get<Color>(value);
            return true;
        case PropID::SymbolClose:
            obj.Symbol.Close.Foreground = std::get<Color>(value);
            return true;
        case PropID::SymbolMaximized:
            obj.Symbol.Maximized.Foreground = std::get<Color>(value);
            return true;
        case PropID::SymbolResize:
            obj.Symbol.Resize.Foreground = std::get<Color>(value);
            return true;

        case PropID::SearchBarNormal:
            obj.SearchBar.Normal = std::get<ColorPair>(value);
            return true;
        case PropID::SearchBarFocused:
            obj.SearchBar.Focused = std::get<ColorPair>(value);
            return true;
        case PropID::SearchBarHovered:
            obj.SearchBar.Hovered = std::get<ColorPair>(value);
            return true;
        case PropID::SearchBarInactive:
            obj.SearchBar.Inactive = std::get<ColorPair>(value);
            return true;

        case PropID::HeaderTextNormal:
            obj.Header.Text.Normal = std::get<ColorPair>(value);
            return true;
        case PropID::HeaderTextFocused:
            obj.Header.Text.Focused = std::get<ColorPair>(value);
            return true;
        case PropID::HeaderTextInactive:
            obj.Header.Text.Inactive = std::get<ColorPair>(value);
            return true;
        case PropID::HeaderTextHovered:
            obj.Header.Text.Hovered = std::get<ColorPair>(value);
            return true;
        case PropID::HeaderTextSelected:
            obj.Header.Text.PressedOrSelected = std::get<ColorPair>(value);
            return true;
        case PropID::HeaderHotKeyNormal:
            obj.Header.HotKey.Normal = std::get<ColorPair>(value);
            return true;
        case PropID::HeaderHotKeyFocused:
            obj.Header.HotKey.Focused = std::get<ColorPair>(value);
            return true;
        case PropID::HeaderHotKeyInactive:
            obj.Header.HotKey.Inactive = std::get<ColorPair>(value);
            return true;
        case PropID::HeaderHotKeyHovered:
            obj.Header.HotKey.Hovered = std::get<ColorPair>(value);
            return true;
        case PropID::HeaderHotKeySelected:
            obj.Header.HotKey.PressedOrSelected = std::get<ColorPair>(value);
            return true;
        case PropID::HeaderSymbolNormal:
            obj.Header.Symbol.Normal = std::get<ColorPair>(value);
            return true;
        case PropID::HeaderSymbolFocused:
            obj.Header.Symbol.Focused = std::get<ColorPair>(value);
            return true;
        case PropID::HeaderSymbolInactive:
            obj.Header.Symbol.Inactive = std::get<ColorPair>(value);
            return true;
        case PropID::HeaderSymbolHovered:
            obj.Header.Symbol.Hovered = std::get<ColorPair>(value);
            return true;
        case PropID::HeaderSymbolSelected:
            obj.Header.Symbol.PressedOrSelected = std::get<ColorPair>(value);
            return true;

        // Cursor
        case PropID::CursorNormal:
            obj.Cursor.Normal = std::get<ColorPair>(value);
            return true;
        case PropID::CursorInactive:
            obj.Cursor.Inactive = std::get<ColorPair>(value);
            return true;
        case PropID::CursorOverInactiveItem:
            obj.Cursor.OverInactiveItem = std::get<ColorPair>(value);
            return true;
        case PropID::CursorOverSelection:
            obj.Cursor.OverSelection = std::get<ColorPair>(value);
            return true;

        // Editor
        case PropID::EditorBackground:
            obj.Editor.Normal.Background  = std::get<Color>(value);
            obj.Editor.Focused.Background = std::get<Color>(value);
            obj.Editor.Hovered.Background = std::get<Color>(value);
            return true;
        case PropID::EditorNormal:
            obj.Editor.Normal.Foreground = std::get<Color>(value);
            return true;
        case PropID::EditorFocus:
            obj.Editor.Focused.Foreground = std::get<Color>(value);
            return true;
        case PropID::EditorInactive:
            obj.Editor.Inactive.Foreground = std::get<Color>(value);
            obj.Editor.Inactive.Background = Color::Transparent;
            return true;
        case PropID::EditorHovered:
            obj.Editor.Hovered.Foreground = std::get<Color>(value);
            return true;
        case PropID::EditorSelection:
            obj.Selection.Editor = std::get<ColorPair>(value);
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
            { PT(PropID::ParentMenuHotKeyNormal), CAT(CatID::ParentMenu), "HotKey (normal)", PropertyType::ColorPair },
            { PT(PropID::ParentMenuHotKeyHovered),
              CAT(CatID::ParentMenu),
              "HotKey (hovered)",
              PropertyType::ColorPair },
            { PT(PropID::ParentMenuShortCutNormal),
              CAT(CatID::ParentMenu),
              "ShortCut (normal)",
              PropertyType::ColorPair },
            { PT(PropID::ParentMenuShortCutHovered),
              CAT(CatID::ParentMenu),
              "ShortCut (hovered)",
              PropertyType::ColorPair },
            { PT(PropID::ParentMenuInactive), CAT(CatID::ParentMenu), "Inactive", PropertyType::Color },
            { PT(PropID::ParentMenuSymbolNormal), CAT(CatID::ParentMenu), "Symbols (normal)", PropertyType::Color },
            { PT(PropID::ParentMenuSymbolHovered), CAT(CatID::ParentMenu), "Symbols (hovered)", PropertyType::Color },
            // Window
            { PT(PropID::WindowNormal), CAT(CatID::Window), "Regular", PropertyType::Color },
            { PT(PropID::WindowInactive), CAT(CatID::Window), "Inactive", PropertyType::Color },
            { PT(PropID::WindowError), CAT(CatID::Window), "Error", PropertyType::Color },
            { PT(PropID::WindowInfo), CAT(CatID::Window), "Notification", PropertyType::Color },
            { PT(PropID::WindowWarning), CAT(CatID::Window), "Warning", PropertyType::Color },
            // ToolTip
            { PT(PropID::ToolTipText), CAT(CatID::ToolTip), "Text", PropertyType::ColorPair },
            { PT(PropID::ToolTipArrow), CAT(CatID::ToolTip), "Arrow", PropertyType::ColorPair },
            // Progress Bar
            { PT(PropID::ProgressBarEmpty), CAT(CatID::ProgressBar), "Empty", PropertyType::Color },
            { PT(PropID::ProgressBarFull), CAT(CatID::ProgressBar), "Full", PropertyType::Color },
            // Buttons
            { PT(PropID::ButtonTextNormal), CAT(CatID::Button), "Text (normal)", PropertyType::ColorPair },
            { PT(PropID::ButtonTextHovered), CAT(CatID::Button), "Text (hovered)", PropertyType::ColorPair },
            { PT(PropID::ButtonTextSelected), CAT(CatID::Button), "Text (pressed)", PropertyType::ColorPair },
            { PT(PropID::ButtonTextInactive), CAT(CatID::Button), "Text (inactive)", PropertyType::ColorPair },
            { PT(PropID::ButtonHotKeyNormal), CAT(CatID::Button), "HotKey (normal)", PropertyType::ColorPair },
            { PT(PropID::ButtonHotKeyHovered), CAT(CatID::Button), "HotKey (hovered)", PropertyType::ColorPair },
            { PT(PropID::ButtonHotKeySelected), CAT(CatID::Button), "HotKey (pressed)", PropertyType::ColorPair },
            { PT(PropID::ButtonHotKeyInactive), CAT(CatID::Button), "HotKey (inactive)", PropertyType::ColorPair },
            { PT(PropID::ButtonShadow), CAT(CatID::Button), "Shaddow", PropertyType::Color },
            // Text
            { PT(PropID::TextNormal), CAT(CatID::Text), "Regular", PropertyType::Color },
            { PT(PropID::TextHotKey), CAT(CatID::Text), "Hot Key", PropertyType::Color },
            { PT(PropID::TextInactive), CAT(CatID::Text), "Inactive", PropertyType::Color },
            { PT(PropID::TextError), CAT(CatID::Text), "Error", PropertyType::Color },
            { PT(PropID::TextWarning), CAT(CatID::Text), "Warning", PropertyType::Color },
            { PT(PropID::TextFocused), CAT(CatID::Text), "Focused", PropertyType::Color },
            { PT(PropID::TextHovered), CAT(CatID::Text), "HOvered", PropertyType::Color },
            { PT(PropID::TextHighlighted), CAT(CatID::Text), "Highlighted", PropertyType::Color },
            { PT(PropID::TextEmphasized1), CAT(CatID::Text), "Emphasized (1)", PropertyType::Color },
            { PT(PropID::TextEmphasized2), CAT(CatID::Text), "Emphasized (2)", PropertyType::Color },
            // Scroll Bar
            { PT(PropID::ScrollBarButtonNormal), CAT(CatID::ScrollBars), "Buttons (normal)", PropertyType::ColorPair },
            { PT(PropID::ScrollBarButtonHovered),
              CAT(CatID::ScrollBars),
              "Buttons (hovered)",
              PropertyType::ColorPair },
            { PT(PropID::ScrollBarButtonPressed),
              CAT(CatID::ScrollBars),
              "Buttons (pressed)",
              PropertyType::ColorPair },
            { PT(PropID::ScrollBarButtonInactive),
              CAT(CatID::ScrollBars),
              "Buttons (Inactive)",
              PropertyType::ColorPair },
            { PT(PropID::ScrollBarNormal), CAT(CatID::ScrollBars), "Bar (normal)", PropertyType::ColorPair },
            { PT(PropID::ScrollBarHovered), CAT(CatID::ScrollBars), "Bar (hovered)", PropertyType::ColorPair },
            { PT(PropID::ScrollBarPressed), CAT(CatID::ScrollBars), "Bar (pressed)", PropertyType::ColorPair },
            { PT(PropID::ScrollBarInactive), CAT(CatID::ScrollBars), "Bar (Inactive)", PropertyType::ColorPair },
            { PT(PropID::ScrollBarPositionNormal),
              CAT(CatID::ScrollBars),
              "Position (normal)",
              PropertyType::ColorPair },
            { PT(PropID::ScrollBarPositionHovered),
              CAT(CatID::ScrollBars),
              "Position (hovered)",
              PropertyType::ColorPair },
            { PT(PropID::ScrollBarPositionPressed),
              CAT(CatID::ScrollBars),
              "Position (pressed)",
              PropertyType::ColorPair },
            // symbols
            { PT(PropID::SymbolInactive), CAT(CatID::Symbols), "Inactive", PropertyType::Color },
            { PT(PropID::SymbolHovered), CAT(CatID::Symbols), "Hovered", PropertyType::ColorPair },
            { PT(PropID::SymbolPressed), CAT(CatID::Symbols), "Pressed", PropertyType::ColorPair },
            { PT(PropID::SymbolChecked), CAT(CatID::Symbols), "Check", PropertyType::Color },
            { PT(PropID::SymbolUnchecked), CAT(CatID::Symbols), "Uncheck", PropertyType::Color },
            { PT(PropID::SymbolUnknown), CAT(CatID::Symbols), "Unknown", PropertyType::Color },
            { PT(PropID::SymbolArrows), CAT(CatID::Symbols), "Arrows", PropertyType::Color },
            { PT(PropID::SymbolClose), CAT(CatID::Symbols), "Windows close", PropertyType::Color },
            { PT(PropID::SymbolMaximized), CAT(CatID::Symbols), "Window maximize", PropertyType::Color },
            { PT(PropID::SymbolResize), CAT(CatID::Symbols), "Window resize", PropertyType::Color },

            // search bar
            { PT(PropID::SearchBarNormal), CAT(CatID::SearchBar), "Regular", PropertyType::ColorPair },
            { PT(PropID::SearchBarFocused), CAT(CatID::SearchBar), "Focused", PropertyType::ColorPair },
            { PT(PropID::SearchBarHovered), CAT(CatID::SearchBar), "Hovered", PropertyType::ColorPair },
            { PT(PropID::SearchBarInactive), CAT(CatID::SearchBar), "Inactive", PropertyType::ColorPair },

            // Header
            { PT(PropID::HeaderTextNormal), CAT(CatID::Headers), "Text (regular)", PropertyType::ColorPair },
            { PT(PropID::HeaderTextFocused), CAT(CatID::Headers), "Text (focused)", PropertyType::ColorPair },
            { PT(PropID::HeaderTextInactive), CAT(CatID::Headers), "Text (inactive)", PropertyType::ColorPair },
            { PT(PropID::HeaderTextHovered), CAT(CatID::Headers), "Text (hovered)", PropertyType::ColorPair },
            { PT(PropID::HeaderTextSelected), CAT(CatID::Headers), "Text (pressed)", PropertyType::ColorPair },
            { PT(PropID::HeaderHotKeyNormal), CAT(CatID::Headers), "HotKey (regular)", PropertyType::ColorPair },
            { PT(PropID::HeaderHotKeyFocused), CAT(CatID::Headers), "HotKey (focused)", PropertyType::ColorPair },
            { PT(PropID::HeaderHotKeyInactive), CAT(CatID::Headers), "HotKey (inactive)", PropertyType::ColorPair },
            { PT(PropID::HeaderHotKeyHovered), CAT(CatID::Headers), "HotKey (hovered)", PropertyType::ColorPair },
            { PT(PropID::HeaderHotKeySelected), CAT(CatID::Headers), "HotKey (pressed)", PropertyType::ColorPair },
            { PT(PropID::HeaderSymbolNormal), CAT(CatID::Headers), "Symbol (regular)", PropertyType::ColorPair },
            { PT(PropID::HeaderSymbolFocused), CAT(CatID::Headers), "Symbol (focused)", PropertyType::ColorPair },
            { PT(PropID::HeaderSymbolInactive), CAT(CatID::Headers), "Symbol (inactive)", PropertyType::ColorPair },
            { PT(PropID::HeaderSymbolHovered), CAT(CatID::Headers), "Symbol (hovered)", PropertyType::ColorPair },
            { PT(PropID::HeaderSymbolSelected), CAT(CatID::Headers), "Symbol (pressed)", PropertyType::ColorPair },

            // Cursor
            { PT(PropID::CursorNormal), CAT(CatID::Cursor), "Normal", PropertyType::ColorPair },
            { PT(PropID::CursorInactive), CAT(CatID::Cursor), "Inactive", PropertyType::ColorPair },
            { PT(PropID::CursorOverInactiveItem), CAT(CatID::Cursor), "Over inactive item", PropertyType::ColorPair },
            { PT(PropID::CursorOverSelection), CAT(CatID::Cursor), "Over selection", PropertyType::ColorPair },

            // Editor
            { PT(PropID::EditorBackground), CAT(CatID::Editor), "Background", PropertyType::Color },
            { PT(PropID::EditorNormal), CAT(CatID::Editor), "Text (normal)", PropertyType::Color },
            { PT(PropID::EditorInactive), CAT(CatID::Editor), "Text (inactive)", PropertyType::Color },
            { PT(PropID::EditorFocus), CAT(CatID::Editor), "Text (focused)", PropertyType::Color },
            { PT(PropID::EditorHovered), CAT(CatID::Editor), "Text (selected)", PropertyType::Color },
            { PT(PropID::EditorSelection), CAT(CatID::Editor), "Selection", PropertyType::ColorPair },

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
    Reference<ComboBox> previewWindow;

  public:
    ThemeEditorDialog(const AppCUI::Application::Config& configObject)
        : Window("Theme editor", "d:c,w:80,h:24", WindowFlags::Sizeable), cfg(configObject)
    {
        auto sp = Factory::Splitter::Create(this, "l:1,t:3,b:3,r:0", true);
        sp->SetSecondPanelSize(30);
        pc = sp->CreateChildControl<PreviewControl>();
        pc->SetConfig(&cfg);
        prop = Factory::PropertyList::Create(sp, "d:c", &cfg, PropertyListFlags::None);
        UpdateCategoryAndProperty();
        Factory::Button::Create(this, "&Close", "r:1,b:0,w:12", BUTTON_CMD_CLOSE);
        Factory::Label::Create(this, "Preview &Window", "x:1,y:1,w:15");
        previewWindow = Factory::ComboBox::Create(this, "x:17,y:1,w:18", "Normal,Inactive,Error,Warning,Notification");
        previewWindow->SetCurentItemIndex(0);
        cfg.SetPreviewWindowID(PreviewWindowID::Normal);
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
            cfg.SetCategoryAndProperty(prop->GetCurrentItemCategory(), PropID::None);
        }
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