#include <cassert>

#include "Internal.hpp"

namespace AppCUI
{
using namespace Graphics;
using namespace Utils;
using namespace OS;
using namespace Application;

// ======================================== CONFIGS ====================================
void Config_SetDefaultTheme(Config& cfg)
{
    cfg.SearchBar.Set(
          { Color::White, Color::DarkRed },
          { Color::Silver, Color::DarkRed },
          { Color::Gray, Color::DarkRed },
          { Color::Yellow, Color::DarkRed });

    cfg.Border.Set(
          { Color::White, Color::Transparent },
          { Color::Silver, Color::Transparent },
          { Color::Gray, Color::Transparent },
          { Color::Yellow, Color::Transparent },
          { Color::Yellow, Color::Magenta });

    cfg.Lines.Set(
          { Color::DarkGreen, Color::Transparent },
          { Color::DarkGreen, Color::Transparent },
          { Color::Gray, Color::Transparent },
          { Color::Gray, Color::Magenta },
          { Color::Yellow, Color::Magenta });

    cfg.Editor.Set(
          { Color::White, Color::Black },
          { Color::Silver, Color::Black },
          { Color::Gray, Color::Transparent },
          { Color::Yellow, Color::Black });

    cfg.LineMarker.Set(
          { Color::Black, Color::Gray },
          { Color::White, Color::Blue },
          { Color::Gray, Color::Transparent },
          { Color::Yellow, Color::Blue });

    cfg.Button.Text.Set(
          { Color::Black, Color::White },
          { Color::Black, Color::Gray },
          { Color::Gray, Color::Black },
          { Color::Black, Color::Yellow },
          { Color::Black, Color::Olive });
    cfg.Button.HotKey.Set(
          { Color::Magenta, Color::White },
          { Color::DarkRed, Color::Gray },
          { Color::Gray, Color::Black },
          { Color::Magenta, Color::Yellow },
          { Color::DarkRed, Color::Olive });
    cfg.Button.ShadowColor = { Color::Black, Color::Transparent };

    cfg.Text.Error       = { Color::Red, Color::Transparent };
    cfg.Text.Warning     = { Color::Olive, Color::Transparent };
    cfg.Text.Normal      = { Color::Silver, Color::Transparent };
    cfg.Text.Focused     = { Color::White, Color::Transparent };
    cfg.Text.Inactive    = { Color::Gray, Color::Transparent };
    cfg.Text.HotKey      = { Color::Aqua, Color::Transparent };
    cfg.Text.Hovered     = { Color::Yellow, Color::Transparent };
    cfg.Text.Highlighted = { Color::Yellow, Color::Transparent };
    cfg.Text.Emphasized1 = { Color::Aqua, Color::Transparent };
    cfg.Text.Emphasized2 = { Color::Green, Color::Transparent };
    cfg.Text.Emphasized3 = { Color::Pink, Color::Transparent };

    cfg.Cursor.Normal           = { Color::Black, Color::White };
    cfg.Cursor.OverInactiveItem = { Color::Gray, Color::White };
    cfg.Cursor.OverSelection    = { Color::Red, Color::Yellow };
    cfg.Cursor.Inactive         = { Color::Yellow, Color::Transparent };

    cfg.Selection.Editor       = { Color::Yellow, Color::Magenta };
    cfg.Selection.LineMarker   = { Color::Yellow, Color::Magenta };
    cfg.Selection.Text         = { Color::Yellow, Color::Black };
    cfg.Selection.SearchMarker = { Color::Yellow, Color::DarkRed };
    cfg.Selection.SimilarText  = { Color::Black, Color::Green };

    cfg.Symbol.Inactive  = { Color::Gray, Color::Transparent };
    cfg.Symbol.Hovered   = { Color::Black, Color::Yellow };
    cfg.Symbol.Pressed   = { Color::Black, Color::Silver };
    cfg.Symbol.Checked   = { Color::Green, Color::Transparent };
    cfg.Symbol.Unchecked = { Color::Red, Color::Transparent };
    cfg.Symbol.Unknown   = { Color::Olive, Color::Transparent };
    cfg.Symbol.Desktop   = { Color::Gray, Color::Black };
    cfg.Symbol.Arrows    = { Color::Aqua, Color::Transparent };
    cfg.Symbol.Close     = { Color::Red, Color::Transparent };
    cfg.Symbol.Maximized = { Color::Aqua, Color::Transparent };
    cfg.Symbol.Resize    = { Color::Aqua, Color::Transparent };

    cfg.ProgressStatus.Empty = { Color::White, Color::Black };
    cfg.ProgressStatus.Full  = { Color::White, Color::Teal };

    cfg.Menu.Text.Set(
          { Color::Black, Color::White },
          { Color::Black, Color::White },
          { Color::Gray, Color::White },
          { Color::Black, Color::Silver },
          { Color::Yellow, Color::Magenta });

    cfg.Menu.HotKey.Set(
          { Color::DarkRed, Color::White },
          { Color::DarkRed, Color::White },
          { Color::Gray, Color::White },
          { Color::DarkRed, Color::Silver },
          { Color::White, Color::Magenta });

    cfg.Menu.Symbol.Set(
          { Color::DarkGreen, Color::White },
          { Color::DarkGreen, Color::White },
          { Color::Gray, Color::White },
          { Color::Magenta, Color::Silver },
          { Color::White, Color::Magenta });

    cfg.Menu.ShortCut = cfg.Menu.HotKey;

    cfg.ParentMenu.Text.Set(
          { Color::Black, Color::Silver },
          { Color::Black, Color::Silver },
          { Color::Gray, Color::Silver },
          { Color::Black, Color::Gray },
          { Color::Yellow, Color::Gray });

    cfg.ParentMenu.HotKey.Set(
          { Color::DarkRed, Color::Silver },
          { Color::DarkRed, Color::Silver },
          { Color::Gray, Color::Silver },
          { Color::DarkRed, Color::Gray },
          { Color::White, Color::Gray });

    cfg.ParentMenu.ShortCut = cfg.ParentMenu.HotKey;

    cfg.ParentMenu.Symbol.Set(
          { Color::DarkGreen, Color::Silver },
          { Color::DarkGreen, Color::Silver },
          { Color::Gray, Color::Silver },
          { Color::Magenta, Color::Gray },
          { Color::White, Color::Gray });

    cfg.Header.Text.Set(
          { Color::White, Color::Magenta },
          { Color::Silver, Color::Magenta },
          { Color::Gray, Color::Transparent },
          { Color::DarkRed, Color::Silver },
          { Color::White, Color::Pink });
    cfg.Header.HotKey.Set(
          { Color::Yellow, Color::Magenta },
          { Color::Yellow, Color::Magenta },
          { Color::Gray, Color::Transparent },
          { Color::Red, Color::Silver },
          { Color::Yellow, Color::Pink });
    cfg.Header.Symbol = cfg.Header.Text;

    cfg.ScrollBar.Bar.Set(
          { Color::White, Color::Teal },
          { Color::White, Color::Teal },
          { Color::Gray, Color::Transparent },
          { Color::Yellow, Color::Silver });
    cfg.ScrollBar.Arrows = cfg.ScrollBar.Bar;
    cfg.ScrollBar.Position.Set(
          { Color::Green, Color::Teal },
          { Color::Green, Color::Teal },
          { Color::Gray, Color::Transparent },
          { Color::Yellow, Color::Silver });

    cfg.ToolTip.Arrow = { Color::Green, Color::Black };
    cfg.ToolTip.Text  = { Color::Black, Color::Aqua };

    cfg.Tab.Text.Set(
          { Color::Black, Color::Gray },
          { Color::White, Color::Gray },
          { Color::Gray, Color::Transparent },
          { Color::Black, Color::Silver },
          { Color::White, Color::Blue });

    cfg.Tab.HotKey.Set(
          { Color::DarkRed, Color::Gray },
          { Color::Yellow, Color::Gray },
          { Color::Gray, Color::Transparent },
          { Color::DarkRed, Color::Silver },
          { Color::Yellow, Color::Blue });

    cfg.Tab.ListText                     = cfg.Tab.Text;
    cfg.Tab.ListText.PressedOrSelected   = { Color::Black, Color::White };
    cfg.Tab.ListHotKey                   = cfg.Tab.HotKey;
    cfg.Tab.ListHotKey.PressedOrSelected = { Color::DarkRed, Color::White };

    cfg.Window.Background.Inactive = Color::Black;
    cfg.Window.Background.Normal   = Color::DarkBlue;
    cfg.Window.Background.Error    = Color::DarkRed;
    cfg.Window.Background.Warning  = Color::Olive;
    cfg.Window.Background.Info     = Color::DarkGreen;
}
void Config_SetDarkTheme(Config& cfg)
{
    cfg.SearchBar.Set(
          { Color::White, Color::DarkRed },
          { Color::Silver, Color::DarkRed },
          { Color::Gray, Color::DarkRed },
          { Color::Yellow, Color::DarkRed },
          { Color::White, Color::DarkRed });
    cfg.Border.Set(
          { Color::White, Color::Transparent },
          { Color::Silver, Color::Transparent },
          { Color::Gray, Color::Transparent },
          { Color::Yellow, Color::Transparent },
          { Color::Aqua, Color::Transparent });
    cfg.Lines.Set(
          { Color::White, Color::Transparent },
          { Color::Silver, Color::Transparent },
          { Color::Gray, Color::Transparent },
          { Color::Yellow, Color::Transparent },
          { Color::Aqua, Color::Transparent });
    cfg.Editor.Set(
          { Color::White, Color::DarkBlue },
          { Color::Silver, Color::DarkBlue },
          { Color::Gray, Color::Transparent },
          { Color::Yellow, Color::DarkBlue },
          { Color::White, Color::Black });
    cfg.LineMarker.Set(
          { Color::Black, Color::Gray },
          { Color::White, Color::Blue },
          { Color::Gray, Color::Transparent },
          { Color::Yellow, Color::Blue },
          { Color::Black, Color::Gray });
    cfg.Button.Text.Set(
          { Color::Black, Color::White },
          { Color::Black, Color::Silver },
          { Color::Black, Color::DarkBlue },
          { Color::Black, Color::Yellow },
          { Color::Black, Color::Olive });
    cfg.Button.HotKey.Set(
          { Color::Magenta, Color::White },
          { Color::DarkRed, Color::Silver },
          { Color::Black, Color::DarkBlue },
          { Color::Magenta, Color::Yellow },
          { Color::DarkRed, Color::Olive });
    cfg.Button.ShadowColor      = { Color::Black, Color::Transparent }; // the value inherited from Default before
    cfg.Text.Normal             = { Color::Silver, Color::Transparent };
    cfg.Text.HotKey             = { Color::Aqua, Color::Transparent };
    cfg.Text.Inactive           = { Color::DarkBlue, Color::Transparent };
    cfg.Text.Error              = { Color::Red, Color::Transparent };
    cfg.Text.Warning            = { Color::Olive, Color::Transparent };
    cfg.Text.Hovered            = { Color::Yellow, Color::Transparent };
    cfg.Text.Focused            = { Color::White, Color::Transparent };
    cfg.Text.Highlighted        = { Color::Yellow, Color::Transparent };
    cfg.Text.Emphasized1        = { Color::Aqua, Color::Transparent };
    cfg.Text.Emphasized2        = { Color::Green, Color::Transparent };
    cfg.Text.Emphasized3        = { Color::Pink, Color::Transparent };
    cfg.Symbol.Inactive         = { Color::DarkBlue, Color::Transparent };
    cfg.Symbol.Hovered          = { Color::Yellow, Color::Black };
    cfg.Symbol.Pressed          = { Color::Black, Color::Yellow };
    cfg.Symbol.Checked          = { Color::Green, Color::Transparent };
    cfg.Symbol.Unchecked        = { Color::Red, Color::Transparent };
    cfg.Symbol.Unknown          = { Color::Olive, Color::Transparent };
    cfg.Symbol.Desktop          = { Color::DarkBlue, Color::Black };
    cfg.Symbol.Arrows           = { Color::Aqua, Color::Transparent };
    cfg.Symbol.Close            = { Color::Red, Color::Transparent };
    cfg.Symbol.Maximized        = { Color::Aqua, Color::Transparent };
    cfg.Symbol.Resize           = { Color::Aqua, Color::Transparent };
    cfg.Cursor.Normal           = { Color::Black, Color::White };
    cfg.Cursor.Inactive         = { Color::Yellow, Color::Transparent };
    cfg.Cursor.OverInactiveItem = { Color::Gray, Color::White };
    cfg.Cursor.OverSelection    = { Color::Red, Color::Yellow };
    cfg.Selection.Editor        = { Color::Yellow, Color::Magenta };
    cfg.Selection.LineMarker    = { Color::Yellow, Color::Magenta };
    cfg.Selection.Text          = { Color::Yellow, Color::Black };
    cfg.Selection.SearchMarker  = { Color::Yellow, Color::DarkRed };
    cfg.Selection.SimilarText   = { Color::Black, Color::Green };
    cfg.ProgressStatus.Empty    = { Color::White, Color::DarkBlue };
    cfg.ProgressStatus.Full     = { Color::White, Color::Gray };
    cfg.Menu.Text.Set(
          { Color::Black, Color::White },
          { Color::Black, Color::Gray },
          { Color::Silver, Color::Gray },
          { Color::Black, Color::Silver },
          { Color::Yellow, Color::DarkBlue });
    cfg.Menu.HotKey.Set(
          { Color::DarkRed, Color::White },
          { Color::DarkRed, Color::Gray },
          { Color::Silver, Color::Gray },
          { Color::DarkRed, Color::Silver },
          { Color::Olive, Color::DarkBlue });
    cfg.Menu.ShortCut.Set(
          { Color::DarkRed, Color::White },
          { Color::DarkRed, Color::Gray },
          { Color::Silver, Color::Gray },
          { Color::DarkRed, Color::Silver },
          { Color::White, Color::DarkBlue });
    cfg.Menu.Symbol.Set(
          { Color::DarkGreen, Color::White },
          { Color::DarkBlue, Color::Gray },
          { Color::Silver, Color::White },
          { Color::Yellow, Color::Silver },
          { Color::White, Color::DarkBlue });
    cfg.ParentMenu.Text.Set(
          { Color::Black, Color::Silver },
          { Color::Black, Color::Gray },
          { Color::Silver, Color::Gray },
          { Color::Black, Color::Silver },
          { Color::Yellow, Color::Gray });
    cfg.ParentMenu.HotKey.Set(
          { Color::DarkRed, Color::Silver },
          { Color::DarkRed, Color::Gray },
          { Color::Silver, Color::Gray },
          { Color::DarkRed, Color::Silver },
          { Color::White, Color::Gray });
    cfg.ParentMenu.ShortCut.Set(
          { Color::DarkRed, Color::Silver },
          { Color::DarkRed, Color::Gray },
          { Color::Silver, Color::Gray },
          { Color::DarkRed, Color::Gray },
          { Color::White, Color::Gray });
    cfg.ParentMenu.Symbol.Set(
          { Color::DarkGreen, Color::Silver },
          { Color::DarkRed, Color::Gray },
          { Color::Silver, Color::Silver },
          { Color::Magenta, Color::Silver },
          { Color::White, Color::Gray });
    cfg.Header.Text.Set(
          { Color::White, Color::DarkBlue },
          { Color::Silver, Color::DarkBlue },
          { Color::Gray, Color::Transparent },
          { Color::DarkRed, Color::Silver },
          { Color::White, Color::Blue });
    cfg.Header.HotKey.Set(
          { Color::Yellow, Color::DarkBlue },
          { Color::Yellow, Color::DarkBlue },
          { Color::Gray, Color::Transparent },
          { Color::Red, Color::Silver },
          { Color::Yellow, Color::Blue });
    cfg.Header.Symbol.Set(
          { Color::White, Color::DarkBlue },
          { Color::Silver, Color::DarkBlue },
          { Color::Gray, Color::Transparent },
          { Color::DarkRed, Color::Silver },
          { Color::White, Color::Blue });
    cfg.ScrollBar.Bar.Set(
          { Color::White, Color::Teal },
          { Color::White, Color::DarkBlue },
          { Color::Gray, Color::Transparent },
          { Color::Yellow, Color::DarkBlue },
          { Color::White, Color::Teal });
    cfg.ScrollBar.Arrows.Set(
          { Color::White, Color::Teal },
          { Color::White, Color::DarkBlue },
          { Color::Gray, Color::Transparent },
          { Color::Yellow, Color::DarkBlue },
          { Color::White, Color::Teal });
    cfg.ScrollBar.Position.Set(
          { Color::Green, Color::Teal },
          { Color::Silver, Color::DarkBlue },
          { Color::Gray, Color::Transparent },
          { Color::Yellow, Color::DarkBlue },
          { Color::Green, Color::Teal });
    cfg.ToolTip.Text  = { Color::Black, Color::Aqua };
    cfg.ToolTip.Arrow = { Color::Green, Color::Black };
    cfg.Tab.Text.Set(
          { Color::Black, Color::Gray },
          { Color::White, Color::Gray },
          { Color::Gray, Color::Transparent },
          { Color::Black, Color::Silver },
          { Color::White, Color::DarkBlue });
    cfg.Tab.HotKey.Set(
          { Color::DarkRed, Color::Gray },
          { Color::Yellow, Color::Gray },
          { Color::Gray, Color::Transparent },
          { Color::DarkRed, Color::Silver },
          { Color::Yellow, Color::DarkBlue });
    cfg.Tab.ListText.Set(
          { Color::Black, Color::Gray },
          { Color::White, Color::Gray },
          { Color::Gray, Color::Transparent },
          { Color::Black, Color::Silver },
          { Color::Black, Color::White });
    cfg.Tab.ListHotKey.Set(
          { Color::DarkRed, Color::Gray },
          { Color::Yellow, Color::Gray },
          { Color::Gray, Color::Transparent },
          { Color::DarkRed, Color::Silver },
          { Color::DarkRed, Color::White });
    cfg.Window.Background.Normal   = Color::Black;
    cfg.Window.Background.Inactive = Color::Black;
    cfg.Window.Background.Error    = Color::DarkRed;
    cfg.Window.Background.Warning  = Color::Black;
    cfg.Window.Background.Info     = Color::Black;
}
void Config_SetLightTheme(Config& cfg)
{
    cfg.SearchBar.Set(
          { Color::Black, Color::Gray },
          { Color::DarkBlue, Color::Gray },
          { Color::Gray, Color::Transparent },
          { Color::DarkRed, Color::Gray },
          { Color::White, Color::DarkRed });
    cfg.Border.Set(
          { Color::Black, Color::Transparent },
          { Color::Black, Color::Transparent },
          { Color::Gray, Color::Transparent },
          { Color::Blue, Color::Transparent },
          { Color::Black, Color::Yellow });
    cfg.Lines.Set(
          { Color::Olive, Color::Transparent },
          { Color::Olive, Color::Transparent },
          { Color::Gray, Color::Transparent },
          { Color::Black, Color::Yellow },
          { Color::Black, Color::White });
    cfg.Editor.Set(
          { Color::White, Color::Black },
          { Color::Silver, Color::Black },
          { Color::Gray, Color::Transparent },
          { Color::Yellow, Color::Black },
          { Color::White, Color::Black });
    cfg.LineMarker.Set(
          { Color::Black, Color::Gray },
          { Color::Black, Color::Gray },
          { Color::Gray, Color::Transparent },
          { Color::Black, Color::Gray },
          { Color::Black, Color::Gray });
    cfg.Button.Text.Set(
          { Color::Black, Color::White },
          { Color::Black, Color::Gray },
          { Color::Gray, Color::Silver },
          { Color::Black, Color::Yellow },
          { Color::Black, Color::Olive });
    cfg.Button.HotKey.Set(
          { Color::Magenta, Color::White },
          { Color::DarkRed, Color::Gray },
          { Color::Gray, Color::Silver },
          { Color::Magenta, Color::Yellow },
          { Color::DarkRed, Color::Olive });
    cfg.Button.ShadowColor      = { Color::Black, Color::Transparent }; // the value inherited from Default before
    cfg.Text.Normal             = { Color::Black, Color::Transparent };
    cfg.Text.HotKey             = { Color::DarkRed, Color::Transparent };
    cfg.Text.Inactive           = { Color::Gray, Color::Transparent };
    cfg.Text.Error              = { Color::DarkRed, Color::Transparent };
    cfg.Text.Warning            = { Color::Red, Color::Transparent };
    cfg.Text.Hovered            = { Color::Magenta, Color::Transparent };
    cfg.Text.Focused            = { Color::Black, Color::Transparent };
    cfg.Text.Highlighted        = { Color::Olive, Color::Transparent };
    cfg.Text.Emphasized1        = { Color::Blue, Color::Transparent };
    cfg.Text.Emphasized2        = { Color::Pink, Color::Transparent };
    cfg.Text.Emphasized3        = { Color::Magenta, Color::Transparent };
    cfg.Symbol.Inactive         = { Color::Gray, Color::Transparent };
    cfg.Symbol.Hovered          = { Color::Black, Color::Yellow };
    cfg.Symbol.Pressed          = { Color::Black, Color::Silver };
    cfg.Symbol.Checked          = { Color::DarkGreen, Color::Transparent };
    cfg.Symbol.Unchecked        = { Color::DarkRed, Color::Transparent };
    cfg.Symbol.Unknown          = { Color::Teal, Color::Transparent };
    cfg.Symbol.Desktop          = { Color::Silver, Color::Gray };
    cfg.Symbol.Arrows           = { Color::Blue, Color::Transparent };
    cfg.Symbol.Close            = { Color::DarkRed, Color::Transparent };
    cfg.Symbol.Maximized        = { Color::Magenta, Color::Transparent };
    cfg.Symbol.Resize           = { Color::Blue, Color::Transparent };
    cfg.Cursor.Normal           = { Color::White, Color::DarkBlue };
    cfg.Cursor.Inactive         = { Color::Red, Color::Transparent };
    cfg.Cursor.OverInactiveItem = { Color::Gray, Color::DarkBlue };
    cfg.Cursor.OverSelection    = { Color::Yellow, Color::DarkBlue };
    cfg.Selection.Editor        = { Color::Black, Color::Yellow };
    cfg.Selection.LineMarker    = { Color::Yellow, Color::Magenta };
    cfg.Selection.Text          = { Color::Yellow, Color::Black };
    cfg.Selection.SearchMarker  = { Color::Yellow, Color::DarkRed };
    cfg.Selection.SimilarText   = { Color::Black, Color::Green };
    cfg.ProgressStatus.Empty    = { Color::White, Color::Olive };
    cfg.ProgressStatus.Full     = { Color::White, Color::Yellow };
    cfg.Menu.Text.Set(
          { Color::Black, Color::White },
          { Color::Black, Color::White },
          { Color::Gray, Color::White },
          { Color::Black, Color::Silver },
          { Color::Yellow, Color::DarkRed });
    cfg.Menu.HotKey.Set(
          { Color::DarkRed, Color::White },
          { Color::DarkRed, Color::White },
          { Color::Gray, Color::White },
          { Color::DarkRed, Color::Silver },
          { Color::White, Color::DarkRed });
    cfg.Menu.ShortCut.Set(
          { Color::DarkRed, Color::White },
          { Color::DarkRed, Color::White },
          { Color::Gray, Color::White },
          { Color::DarkRed, Color::Silver },
          { Color::White, Color::DarkRed });
    cfg.Menu.Symbol.Set(
          { Color::DarkGreen, Color::White },
          { Color::DarkGreen, Color::White },
          { Color::Gray, Color::White },
          { Color::Magenta, Color::Silver },
          { Color::White, Color::DarkRed });
    cfg.ParentMenu.Text.Set(
          { Color::Black, Color::Silver },
          { Color::Black, Color::Silver },
          { Color::Gray, Color::Silver },
          { Color::Black, Color::Gray },
          { Color::Yellow, Color::Gray });
    cfg.ParentMenu.HotKey.Set(
          { Color::DarkRed, Color::Silver },
          { Color::DarkRed, Color::Silver },
          { Color::Gray, Color::Silver },
          { Color::DarkRed, Color::Gray },
          { Color::White, Color::Gray });
    cfg.ParentMenu.ShortCut.Set(
          { Color::DarkRed, Color::Silver },
          { Color::DarkRed, Color::Silver },
          { Color::Gray, Color::Silver },
          { Color::DarkRed, Color::Gray },
          { Color::White, Color::Gray });
    cfg.ParentMenu.Symbol.Set(
          { Color::DarkGreen, Color::Silver },
          { Color::DarkGreen, Color::Silver },
          { Color::Gray, Color::Silver },
          { Color::Magenta, Color::Gray },
          { Color::White, Color::Gray });
    cfg.Header.Text.Set(
          { Color::White, Color::DarkRed },
          { Color::Silver, Color::DarkRed },
          { Color::Gray, Color::Transparent },
          { Color::DarkRed, Color::Yellow },
          { Color::White, Color::Red });
    cfg.Header.HotKey.Set(
          { Color::Yellow, Color::DarkRed },
          { Color::Yellow, Color::DarkRed },
          { Color::Gray, Color::Transparent },
          { Color::Red, Color::Yellow },
          { Color::Yellow, Color::Red });
    cfg.Header.Symbol.Set(
          { Color::White, Color::DarkRed },
          { Color::Silver, Color::DarkRed },
          { Color::Gray, Color::Transparent },
          { Color::DarkRed, Color::Yellow },
          { Color::White, Color::Red });
    cfg.ScrollBar.Bar.Set(
          { Color::White, Color::Teal },
          { Color::Black, Color::Gray },
          { Color::Gray, Color::Transparent },
          { Color::Yellow, Color::Gray },
          { Color::White, Color::Teal });
    cfg.ScrollBar.Arrows.Set(
          { Color::White, Color::Teal },
          { Color::Black, Color::Gray },
          { Color::Gray, Color::Transparent },
          { Color::Blue, Color::Gray },
          { Color::White, Color::Teal });
    cfg.ScrollBar.Position.Set(
          { Color::Green, Color::Teal },
          { Color::Black, Color::Gray },
          { Color::Gray, Color::Transparent },
          { Color::Blue, Color::Silver },
          { Color::Green, Color::Teal });
    cfg.ToolTip.Text  = { Color::Black, Color::Aqua };
    cfg.ToolTip.Arrow = { Color::Black, Color::Silver };
    cfg.Tab.Text.Set(
          { Color::Black, Color::Gray },
          { Color::Black, Color::Silver },
          { Color::Gray, Color::Transparent },
          { Color::Black, Color::Yellow },
          { Color::White, Color::Gray });
    cfg.Tab.HotKey.Set(
          { Color::DarkRed, Color::Gray },
          { Color::DarkRed, Color::Silver },
          { Color::Gray, Color::Transparent },
          { Color::DarkRed, Color::Yellow },
          { Color::DarkRed, Color::Gray });
    cfg.Tab.ListText.Set(
          { Color::Black, Color::Silver },
          { Color::Black, Color::Silver },
          { Color::Gray, Color::Transparent },
          { Color::Black, Color::Yellow },
          { Color::Black, Color::White });
    cfg.Tab.ListHotKey.Set(
          { Color::DarkRed, Color::Silver },
          { Color::DarkRed, Color::Silver },
          { Color::Gray, Color::Transparent },
          { Color::DarkRed, Color::Yellow },
          { Color::DarkRed, Color::White });
    cfg.Window.Background.Normal   = Color::Silver;
    cfg.Window.Background.Inactive = Color::Silver;
    cfg.Window.Background.Error    = Color::Red;
    cfg.Window.Background.Warning  = Color::Silver;
    cfg.Window.Background.Info     = Color::Silver;
}

// ---------------------------------------------------------------------------------------------------------------------
// Built-in themes defined by a palette of roles: Config_SetPaletteTheme writes every field of the configuration from
// it, so a palette theme can not leave a color group unset. The colors were checked against the 16-color palettes of
// the frontends (Windows console / SDL / browser use the Campbell palette, ncurses the xterm one): roles shown next to
// each other never share a color pair, and text keeps a readable contrast in both palettes.
// ---------------------------------------------------------------------------------------------------------------------
struct PaletteTheme
{
    // window backgrounds
    Color Back, InactiveBack, ErrorBack, WarningBack, InfoBack;
    // drawn over the window background (transparent background)
    Color Text, Focused, Inactive, Frame, Accent, Hover, Error, Warning, Emphasized1, Emphasized2, Emphasized3, Checked,
          Unchecked, Shadow;
    ColorPair HotKey, TextHovered, Highlighted; // may carry a background (high contrast / monochrome themes)
    ColorPair Desktop;
    // controls with their own background
    ColorPair Panel, PanelHotKey; // buttons, headers, tabs
    ColorPair Focus, FocusHotKey; // focused button / header, selected tab or menu item
    ColorPair Hot, HotHotKey;     // hovered
    ColorPair Selection;          // selected text / items, pressed button
    ColorPair InactiveControl;    // disabled button
    ColorPair Menu, MenuHotKey, MenuInactive, ParentMenu, ParentMenuHotKey, ParentMenuInactive;
    ColorPair Cursor, CursorInactive, CursorOverInactive, CursorOverSelection;
    ColorPair SearchMarker, SimilarText;
    Color EditorBack;
    ColorPair ScrollBar, ScrollThumb, ToolTip, ProgressEmpty, ProgressFull;
};

void Config_SetPaletteTheme(Config& cfg, const PaletteTheme& p)
{
    constexpr Color T = Color::Transparent;

    cfg.SearchBar.Set(p.Focus, p.Panel, { p.Inactive, p.Back }, p.Hot, p.Selection);
    cfg.Border.Set({ p.Accent, T }, { p.Frame, T }, { p.Inactive, T }, { p.Hover, T }, p.Selection);
    cfg.Lines.Set({ p.Focused, T }, { p.Frame, T }, { p.Inactive, T }, { p.Hover, T }, p.Selection);
    cfg.Editor.Set(
          { p.Focused, p.EditorBack },
          { p.Text, p.EditorBack },
          { p.Inactive, p.EditorBack },
          { p.Hover, p.EditorBack },
          p.Selection);
    cfg.LineMarker.Set(p.Focus, p.Panel, { p.Inactive, p.EditorBack }, p.Hot, p.Selection);

    cfg.Button.Text.Set(p.Focus, p.Panel, p.InactiveControl, p.Hot, p.Selection);
    cfg.Button.HotKey.Set(p.FocusHotKey, p.PanelHotKey, p.InactiveControl, p.HotHotKey, p.Selection);
    cfg.Button.ShadowColor = { p.Shadow, T };

    cfg.Text.Normal      = { p.Text, T };
    cfg.Text.HotKey      = p.HotKey;
    cfg.Text.Inactive    = { p.Inactive, T };
    cfg.Text.Error       = { p.Error, T };
    cfg.Text.Warning     = { p.Warning, T };
    cfg.Text.Hovered     = p.TextHovered;
    cfg.Text.Focused     = { p.Focused, T };
    cfg.Text.Highlighted = p.Highlighted;
    cfg.Text.Emphasized1 = { p.Emphasized1, T };
    cfg.Text.Emphasized2 = { p.Emphasized2, T };
    cfg.Text.Emphasized3 = { p.Emphasized3, T };

    cfg.Symbol.Inactive  = { p.Inactive, T };
    cfg.Symbol.Hovered   = p.Hot;
    cfg.Symbol.Pressed   = p.Selection;
    cfg.Symbol.Checked   = { p.Checked, T };
    cfg.Symbol.Unchecked = { p.Unchecked, T };
    cfg.Symbol.Unknown   = { p.Warning, T };
    cfg.Symbol.Desktop   = p.Desktop;
    cfg.Symbol.Arrows    = { p.Accent, T };
    cfg.Symbol.Close     = { p.Error, T };
    cfg.Symbol.Maximized = { p.Accent, T };
    cfg.Symbol.Resize    = { p.Accent, T };

    cfg.Cursor.Normal           = p.Cursor;
    cfg.Cursor.Inactive         = p.CursorInactive;
    cfg.Cursor.OverInactiveItem = p.CursorOverInactive;
    cfg.Cursor.OverSelection    = p.CursorOverSelection;

    cfg.Selection.Editor       = p.Selection;
    cfg.Selection.LineMarker   = p.Selection;
    cfg.Selection.Text         = p.Selection;
    cfg.Selection.SearchMarker = p.SearchMarker;
    cfg.Selection.SimilarText  = p.SimilarText;

    cfg.ProgressStatus.Empty = p.ProgressEmpty;
    cfg.ProgressStatus.Full  = p.ProgressFull;

    cfg.Menu.Text.Set(p.Menu, p.Menu, p.MenuInactive, p.Hot, p.Focus);
    cfg.Menu.HotKey.Set(p.MenuHotKey, p.MenuHotKey, p.MenuInactive, p.HotHotKey, p.FocusHotKey);
    cfg.Menu.ShortCut = cfg.Menu.HotKey;
    cfg.Menu.Symbol   = cfg.Menu.HotKey;

    cfg.ParentMenu.Text.Set(p.ParentMenu, p.ParentMenu, p.ParentMenuInactive, p.Hot, p.Focus);
    cfg.ParentMenu.HotKey.Set(
          p.ParentMenuHotKey, p.ParentMenuHotKey, p.ParentMenuInactive, p.HotHotKey, p.FocusHotKey);
    cfg.ParentMenu.ShortCut = cfg.ParentMenu.HotKey;
    cfg.ParentMenu.Symbol   = cfg.ParentMenu.HotKey;

    cfg.Header.Text.Set(p.Focus, p.Panel, { p.Inactive, T }, p.Hot, p.Selection);
    cfg.Header.HotKey.Set(p.FocusHotKey, p.PanelHotKey, { p.Inactive, T }, p.HotHotKey, p.Selection);
    cfg.Header.Symbol = cfg.Header.Text;

    cfg.ScrollBar.Bar.Set(p.ScrollBar, p.ScrollBar, { p.Inactive, T }, p.Hot, p.Selection);
    cfg.ScrollBar.Arrows = cfg.ScrollBar.Bar;
    cfg.ScrollBar.Position.Set(p.ScrollThumb, p.ScrollThumb, { p.Inactive, T }, p.Hot, p.Selection);

    cfg.ToolTip.Text  = p.ToolTip;
    cfg.ToolTip.Arrow = { p.ToolTip.Background, p.Back };

    cfg.Tab.Text.Set(p.Panel, p.Panel, { p.Inactive, T }, p.Hot, p.Focus);
    cfg.Tab.HotKey.Set(p.PanelHotKey, p.PanelHotKey, { p.Inactive, T }, p.HotHotKey, p.FocusHotKey);
    cfg.Tab.ListText                     = cfg.Tab.Text;
    cfg.Tab.ListText.PressedOrSelected   = p.Selection;
    cfg.Tab.ListHotKey                   = cfg.Tab.HotKey;
    cfg.Tab.ListHotKey.PressedOrSelected = p.Selection;

    cfg.Window.Background.Normal   = p.Back;
    cfg.Window.Background.Inactive = p.InactiveBack;
    cfg.Window.Background.Error    = p.ErrorBack;
    cfg.Window.Background.Warning  = p.WarningBack;
    cfg.Window.Background.Info     = p.InfoBack;
}

// Accessibility: bright text on black; every highlight (focus, hover, selection, cursor) is a solid background; the
// focused window border is yellow, the other borders white.
constexpr PaletteTheme HIGH_CONTRAST_THEME = {
    .Back = Color::Black, .InactiveBack = Color::Black, .ErrorBack = Color::DarkRed, .WarningBack = Color::Black,
    .InfoBack = Color::Black,
    .Text = Color::Silver, .Focused = Color::White, .Inactive = Color::Gray, .Frame = Color::White,
    .Accent = Color::Yellow, .Hover = Color::Aqua, .Error = Color::Red, .Warning = Color::Yellow,
    .Emphasized1 = Color::Aqua, .Emphasized2 = Color::Green, .Emphasized3 = Color::Pink, .Checked = Color::Green,
    .Unchecked = Color::Red, .Shadow = Color::Gray,
    .HotKey = { Color::Yellow, Color::Transparent }, .TextHovered = { Color::Aqua, Color::Transparent },
    .Highlighted = { Color::Black, Color::Yellow },
    .Desktop = { Color::Gray, Color::Black },
    .Panel = { Color::White, Color::DarkBlue }, .PanelHotKey = { Color::Yellow, Color::DarkBlue },
    .Focus = { Color::Black, Color::Yellow }, .FocusHotKey = { Color::DarkRed, Color::Yellow },
    .Hot = { Color::Black, Color::Aqua }, .HotHotKey = { Color::DarkRed, Color::Aqua },
    .Selection = { Color::Black, Color::White },
    .InactiveControl = { Color::Gray, Color::Black },
    .Menu = { Color::White, Color::Black }, .MenuHotKey = { Color::Yellow, Color::Black },
    .MenuInactive = { Color::Gray, Color::Black },
    .ParentMenu = { Color::Silver, Color::Black }, .ParentMenuHotKey = { Color::Yellow, Color::Black },
    .ParentMenuInactive = { Color::Gray, Color::Black },
    .Cursor = { Color::Black, Color::Green }, .CursorInactive = { Color::Black, Color::Gray },
    .CursorOverInactive = { Color::Gray, Color::White }, .CursorOverSelection = { Color::White, Color::Magenta },
    .SearchMarker = { Color::Black, Color::Aqua }, .SimilarText = { Color::Black, Color::Green },
    .EditorBack = Color::Black,
    .ScrollBar = { Color::Black, Color::Silver }, .ScrollThumb = { Color::DarkBlue, Color::Silver },
    .ToolTip = { Color::Black, Color::Yellow },
    .ProgressEmpty = { Color::White, Color::DarkBlue }, .ProgressFull = { Color::Black, Color::Yellow },
};

// Dark blue / black: black windows, dark blue panels and menus, blue and aqua accents.
constexpr PaletteTheme MIDNIGHT_THEME = {
    .Back = Color::Black, .InactiveBack = Color::Black, .ErrorBack = Color::DarkRed, .WarningBack = Color::Magenta,
    .InfoBack = Color::DarkBlue,
    .Text = Color::Silver, .Focused = Color::White, .Inactive = Color::Gray, .Frame = Color::Silver,
    .Accent = Color::Blue, .Hover = Color::Yellow, .Error = Color::Red, .Warning = Color::Olive,
    .Emphasized1 = Color::Blue, .Emphasized2 = Color::Aqua, .Emphasized3 = Color::Pink, .Checked = Color::Green,
    .Unchecked = Color::Red, .Shadow = Color::DarkBlue,
    .HotKey = { Color::Aqua, Color::Transparent }, .TextHovered = { Color::Yellow, Color::Transparent },
    .Highlighted = { Color::White, Color::DarkBlue },
    .Desktop = { Color::DarkBlue, Color::Black },
    .Panel = { Color::White, Color::DarkBlue }, .PanelHotKey = { Color::Yellow, Color::DarkBlue },
    .Focus = { Color::Black, Color::Aqua }, .FocusHotKey = { Color::Magenta, Color::Aqua },
    .Hot = { Color::Black, Color::Silver }, .HotHotKey = { Color::DarkRed, Color::Silver },
    .Selection = { Color::Black, Color::Blue },
    .InactiveControl = { Color::Gray, Color::DarkBlue },
    .Menu = { Color::White, Color::DarkBlue }, .MenuHotKey = { Color::Yellow, Color::DarkBlue },
    .MenuInactive = { Color::Gray, Color::DarkBlue },
    .ParentMenu = { Color::Silver, Color::Black }, .ParentMenuHotKey = { Color::Aqua, Color::Black },
    .ParentMenuInactive = { Color::Gray, Color::Black },
    .Cursor = { Color::Black, Color::Aqua }, .CursorInactive = { Color::Black, Color::Gray },
    .CursorOverInactive = { Color::Gray, Color::White }, .CursorOverSelection = { Color::Black, Color::Yellow },
    .SearchMarker = { Color::Black, Color::Olive }, .SimilarText = { Color::Black, Color::Green },
    .EditorBack = Color::Black,
    .ScrollBar = { Color::Silver, Color::DarkBlue }, .ScrollThumb = { Color::Aqua, Color::DarkBlue },
    .ToolTip = { Color::Black, Color::Silver },
    .ProgressEmpty = { Color::White, Color::DarkBlue }, .ProgressFull = { Color::Black, Color::Aqua },
};

// Teal / cyan: dark blue water, aqua text and panels; teal desktop, scroll bars and tool tips.
constexpr PaletteTheme OCEAN_THEME = {
    .Back = Color::DarkBlue, .InactiveBack = Color::Black, .ErrorBack = Color::DarkRed, .WarningBack = Color::Magenta,
    .InfoBack = Color::Black,
    .Text = Color::Aqua, .Focused = Color::White, .Inactive = Color::Gray, .Frame = Color::Aqua,
    .Accent = Color::White, .Hover = Color::Yellow, .Error = Color::Red, .Warning = Color::Olive,
    .Emphasized1 = Color::Green, .Emphasized2 = Color::Pink, .Emphasized3 = Color::Silver, .Checked = Color::Green,
    .Unchecked = Color::Red, .Shadow = Color::Black,
    .HotKey = { Color::Yellow, Color::Transparent }, .TextHovered = { Color::White, Color::Transparent },
    .Highlighted = { Color::Black, Color::Aqua },
    .Desktop = { Color::DarkBlue, Color::Teal },
    .Panel = { Color::Black, Color::Aqua }, .PanelHotKey = { Color::DarkRed, Color::Aqua },
    .Focus = { Color::Black, Color::White }, .FocusHotKey = { Color::DarkRed, Color::White },
    .Hot = { Color::Black, Color::Yellow }, .HotHotKey = { Color::DarkRed, Color::Yellow },
    .Selection = { Color::Black, Color::Green },
    .InactiveControl = { Color::Gray, Color::Black },
    .Menu = { Color::Black, Color::Aqua }, .MenuHotKey = { Color::DarkRed, Color::Aqua },
    .MenuInactive = { Color::Gray, Color::Aqua },
    .ParentMenu = { Color::Black, Color::Silver }, .ParentMenuHotKey = { Color::DarkRed, Color::Silver },
    .ParentMenuInactive = { Color::Gray, Color::Silver },
    .Cursor = { Color::Black, Color::Aqua }, .CursorInactive = { Color::Black, Color::Teal },
    .CursorOverInactive = { Color::Gray, Color::White }, .CursorOverSelection = { Color::Black, Color::White },
    .SearchMarker = { Color::Black, Color::Olive }, .SimilarText = { Color::White, Color::Magenta },
    .EditorBack = Color::DarkBlue,
    .ScrollBar = { Color::Black, Color::Teal }, .ScrollThumb = { Color::White, Color::Teal },
    .ToolTip = { Color::Black, Color::Teal },
    .ProgressEmpty = { Color::Black, Color::Teal }, .ProgressFull = { Color::Black, Color::White },
};

// Green / olive: black windows with olive frames, dark green panels, olive focus and green highlights.
constexpr PaletteTheme FOREST_THEME = {
    .Back = Color::Black, .InactiveBack = Color::Black, .ErrorBack = Color::DarkRed, .WarningBack = Color::Magenta,
    .InfoBack = Color::DarkBlue,
    .Text = Color::Silver, .Focused = Color::White, .Inactive = Color::Gray, .Frame = Color::Olive,
    .Accent = Color::Green, .Hover = Color::Yellow, .Error = Color::Red, .Warning = Color::Olive,
    .Emphasized1 = Color::Green, .Emphasized2 = Color::Olive, .Emphasized3 = Color::Aqua, .Checked = Color::Green,
    .Unchecked = Color::Red, .Shadow = Color::Gray,
    .HotKey = { Color::Yellow, Color::Transparent }, .TextHovered = { Color::Green, Color::Transparent },
    .Highlighted = { Color::Black, Color::Olive },
    .Desktop = { Color::DarkGreen, Color::Black },
    .Panel = { Color::Black, Color::DarkGreen }, .PanelHotKey = { Color::DarkBlue, Color::DarkGreen },
    .Focus = { Color::Black, Color::Olive }, .FocusHotKey = { Color::DarkBlue, Color::Olive },
    .Hot = { Color::Black, Color::Green }, .HotHotKey = { Color::DarkBlue, Color::Green },
    .Selection = { Color::Black, Color::Yellow },
    .InactiveControl = { Color::Gray, Color::Black },
    .Menu = { Color::Black, Color::Silver }, .MenuHotKey = { Color::DarkRed, Color::Silver },
    .MenuInactive = { Color::Gray, Color::Silver },
    .ParentMenu = { Color::Black, Color::Gray }, .ParentMenuHotKey = { Color::White, Color::Gray },
    .ParentMenuInactive = { Color::Silver, Color::Gray },
    .Cursor = { Color::Black, Color::Green }, .CursorInactive = { Color::Black, Color::Gray },
    .CursorOverInactive = { Color::Gray, Color::White }, .CursorOverSelection = { Color::Black, Color::Aqua },
    .SearchMarker = { Color::White, Color::DarkRed }, .SimilarText = { Color::Black, Color::Aqua },
    .EditorBack = Color::Black,
    .ScrollBar = { Color::Black, Color::DarkGreen }, .ScrollThumb = { Color::Yellow, Color::DarkGreen },
    .ToolTip = { Color::Black, Color::Olive },
    .ProgressEmpty = { Color::Black, Color::DarkGreen }, .ProgressFull = { Color::Black, Color::Yellow },
};

// Monochrome green terminal: green on black, highlights in inverse video. Gray / silver / white stand for the dim and
// bright intensities; red is kept for errors only.
constexpr PaletteTheme RETRO_GREEN_THEME = {
    .Back = Color::Black, .InactiveBack = Color::Black, .ErrorBack = Color::DarkRed, .WarningBack = Color::Black,
    .InfoBack = Color::Black,
    .Text = Color::Green, .Focused = Color::Green, .Inactive = Color::Gray, .Frame = Color::DarkGreen,
    .Accent = Color::Green, .Hover = Color::Green, .Error = Color::Red, .Warning = Color::White,
    .Emphasized1 = Color::Silver, .Emphasized2 = Color::DarkGreen, .Emphasized3 = Color::White,
    .Checked = Color::Green, .Unchecked = Color::DarkGreen, .Shadow = Color::DarkGreen,
    .HotKey = { Color::Black, Color::DarkGreen }, .TextHovered = { Color::Black, Color::Green },
    .Highlighted = { Color::Black, Color::Green },
    .Desktop = { Color::DarkGreen, Color::Black },
    .Panel = { Color::Black, Color::DarkGreen }, .PanelHotKey = { Color::Green, Color::Black },
    .Focus = { Color::Black, Color::Green }, .FocusHotKey = { Color::Green, Color::Black },
    .Hot = { Color::Green, Color::Black }, .HotHotKey = { Color::Black, Color::Green },
    .Selection = { Color::Black, Color::Silver },
    .InactiveControl = { Color::Gray, Color::Black },
    .Menu = { Color::Black, Color::DarkGreen }, .MenuHotKey = { Color::Green, Color::Black },
    .MenuInactive = { Color::Gray, Color::Black },
    .ParentMenu = { Color::DarkGreen, Color::Black }, .ParentMenuHotKey = { Color::Green, Color::Black },
    .ParentMenuInactive = { Color::Gray, Color::Black },
    .Cursor = { Color::Black, Color::Green }, .CursorInactive = { Color::Black, Color::DarkGreen },
    .CursorOverInactive = { Color::Black, Color::Gray }, .CursorOverSelection = { Color::White, Color::DarkGreen },
    .SearchMarker = { Color::Black, Color::Gray }, .SimilarText = { Color::Black, Color::DarkGreen },
    .EditorBack = Color::Black,
    .ScrollBar = { Color::DarkGreen, Color::Black }, .ScrollThumb = { Color::Green, Color::Black },
    .ToolTip = { Color::Black, Color::Green },
    .ProgressEmpty = { Color::Black, Color::DarkGreen }, .ProgressFull = { Color::Black, Color::Green },
};

bool WriteKeyToString(Color col, std::string_view name, Utils::String& output)
{
    CHECK(output.Add(name), false, "");
    CHECK(output.Add(" = "), false, "");
    CHECK(output.Add(AppCUI::Utils::ColorUtils::GetColorName(col)), false, "");
    CHECK(output.Add("\n"), false, "");
    return true;
}
bool WriteKeyToString(ColorPair col, std::string_view name, std::string_view field, Utils::String& output)
{
    CHECK(output.Add(name), false, "");
    if (field.length() > 0)
    {
        CHECK(output.AddChar('.'), false, "");
        CHECK(output.Add(field), false, "");
    }
    CHECK(output.Add(" = "), false, "");
    CHECK(output.Add(AppCUI::Utils::ColorUtils::GetColorName(col.Foreground)), false, "");
    CHECK(output.Add(","), false, "");
    CHECK(output.Add(AppCUI::Utils::ColorUtils::GetColorName(col.Background)), false, "");
    CHECK(output.Add("\n"), false, "");
    return true;
}
bool WriteKeyToString(const ObjectColorState& col, std::string_view name, Utils::String& output)
{
    CHECK(WriteKeyToString(col.Focused, name, "Focused", output), false, "");
    CHECK(WriteKeyToString(col.Normal, name, "Normal", output), false, "");
    CHECK(WriteKeyToString(col.Inactive, name, "Inactive", output), false, "");
    CHECK(WriteKeyToString(col.Hovered, name, "Hovered", output), false, "");
    CHECK(WriteKeyToString(col.PressedOrSelected, name, "Pressed", output), false, "");
    return true;
}
#define WRITE_COLORSTATE(key, name) CHECK(WriteKeyToString(key, name, temp), false, "Fail to write key: %s", name);
#define WRITE_COLORPAIR(key, name)  CHECK(WriteKeyToString(key, name, "", temp), false, "Fail to write key: %s", name);
#define WRITE_COLOR(key, name)      CHECK(WriteKeyToString(key, name, temp), false, "Fail to write key: %s", name);
#define WRITE_SECTION(name)         CHECK(temp.Add(name), false, "Fail to create section: %s", name);

// a key that is missing keeps the current value (the base theme applied before loading)
#define READ_COLORSTATE(key, name)                                                                                     \
    config.key.Focused           = sect.GetValue(#name ".Focused").ToColorPair(config.key.Focused);                    \
    config.key.Normal            = sect.GetValue(#name ".Normal").ToColorPair(config.key.Normal);                      \
    config.key.Inactive          = sect.GetValue(#name ".Inactive").ToColorPair(config.key.Inactive);                  \
    config.key.Hovered           = sect.GetValue(#name ".Hovered").ToColorPair(config.key.Hovered);                    \
    config.key.PressedOrSelected = sect.GetValue(#name ".Pressed").ToColorPair(config.key.PressedOrSelected);
#define READ_COLORPAIR(key, name) config.key = sect.GetValue(#name).ToColorPair(config.key);
#define READ_COLOR(key, name)     config.key = sect.GetValue(#name).ToColor(config.key);

bool Internal::Config::SaveToString(Application::Config& config, std::string& output)
{
    AppCUI::Utils::LocalString<8192> temp;

    // sections
    WRITE_SECTION("[General]\n");
    WRITE_COLORSTATE(config.SearchBar, "SearchBar");
    WRITE_COLORSTATE(config.Border, "Border");
    WRITE_COLORSTATE(config.Lines, "Lines");
    WRITE_COLORSTATE(config.Editor, "Editor");
    WRITE_COLORSTATE(config.LineMarker, "LineMarker");

    WRITE_SECTION("\n[Button]\n");
    WRITE_COLORSTATE(config.Button.Text, "Text");
    WRITE_COLORSTATE(config.Button.HotKey, "HotKey");
    WRITE_COLORPAIR(config.Button.ShadowColor, "Shadow");

    WRITE_SECTION("\n[Text]\n");
    WRITE_COLORPAIR(config.Text.Normal, "Normal");
    WRITE_COLORPAIR(config.Text.HotKey, "HotKey");
    WRITE_COLORPAIR(config.Text.Inactive, "Inactive");
    WRITE_COLORPAIR(config.Text.Error, "Error");
    WRITE_COLORPAIR(config.Text.Warning, "Warning");
    WRITE_COLORPAIR(config.Text.Hovered, "Hovered");
    WRITE_COLORPAIR(config.Text.Focused, "Focused");
    WRITE_COLORPAIR(config.Text.Highlighted, "Highlighted");
    WRITE_COLORPAIR(config.Text.Emphasized1, "Emphasized1");
    WRITE_COLORPAIR(config.Text.Emphasized2, "Emphasized2");
    WRITE_COLORPAIR(config.Text.Emphasized3, "Emphasized3");

    WRITE_SECTION("\n[Symbol]\n");
    WRITE_COLORPAIR(config.Symbol.Inactive, "Inactive");
    WRITE_COLORPAIR(config.Symbol.Hovered, "Hovered");
    WRITE_COLORPAIR(config.Symbol.Pressed, "Pressed");
    WRITE_COLORPAIR(config.Symbol.Checked, "Checked");
    WRITE_COLORPAIR(config.Symbol.Unchecked, "Unchecked");
    WRITE_COLORPAIR(config.Symbol.Unknown, "Unknown");
    WRITE_COLORPAIR(config.Symbol.Desktop, "Desktop");
    WRITE_COLORPAIR(config.Symbol.Arrows, "Arrows");
    WRITE_COLORPAIR(config.Symbol.Close, "Close");
    WRITE_COLORPAIR(config.Symbol.Maximized, "Maximized");
    WRITE_COLORPAIR(config.Symbol.Resize, "Resize");

    WRITE_SECTION("\n[Cursor]\n");
    WRITE_COLORPAIR(config.Cursor.Inactive, "Inactive");
    WRITE_COLORPAIR(config.Cursor.Normal, "Normal");
    WRITE_COLORPAIR(config.Cursor.OverInactiveItem, "OverInactiveItem");
    WRITE_COLORPAIR(config.Cursor.OverSelection, "OverSelection");

    WRITE_SECTION("\n[Selection]\n");
    WRITE_COLORPAIR(config.Selection.Editor, "Editor");
    WRITE_COLORPAIR(config.Selection.LineMarker, "LineMarker");
    WRITE_COLORPAIR(config.Selection.Text, "Text");
    WRITE_COLORPAIR(config.Selection.SearchMarker, "SearchMarker");
    WRITE_COLORPAIR(config.Selection.SimilarText, "SimilarText");

    WRITE_SECTION("\n[ProgressStatus]\n");
    WRITE_COLORPAIR(config.ProgressStatus.Empty, "Empty");
    WRITE_COLORPAIR(config.ProgressStatus.Full, "Full");

    WRITE_SECTION("\n[Menu]\n");
    WRITE_COLORSTATE(config.Menu.Text, "Text");
    WRITE_COLORSTATE(config.Menu.HotKey, "HotKey");
    WRITE_COLORSTATE(config.Menu.ShortCut, "ShortCut");
    WRITE_COLORSTATE(config.Menu.Symbol, "Symbol");

    WRITE_SECTION("\n[ParentMenu]\n");
    WRITE_COLORSTATE(config.ParentMenu.Text, "Text");
    WRITE_COLORSTATE(config.ParentMenu.HotKey, "HotKey");
    WRITE_COLORSTATE(config.ParentMenu.ShortCut, "ShortCut");
    WRITE_COLORSTATE(config.ParentMenu.Symbol, "Symbol");

    WRITE_SECTION("\n[Header]\n");
    WRITE_COLORSTATE(config.Header.Text, "Text");
    WRITE_COLORSTATE(config.Header.HotKey, "HotKey");
    WRITE_COLORSTATE(config.Header.Symbol, "Symbol");

    WRITE_SECTION("\n[ScrollBar]\n");
    WRITE_COLORSTATE(config.ScrollBar.Bar, "Bar");
    WRITE_COLORSTATE(config.ScrollBar.Arrows, "Arrows");
    WRITE_COLORSTATE(config.ScrollBar.Position, "Position");

    WRITE_SECTION("\n[ToolTip]\n");
    WRITE_COLORPAIR(config.ToolTip.Text, "Text");
    WRITE_COLORPAIR(config.ToolTip.Arrow, "Arrow");

    WRITE_SECTION("\n[Tab]\n");
    WRITE_COLORSTATE(config.Tab.Text, "Text");
    WRITE_COLORSTATE(config.Tab.HotKey, "HotKey");
    WRITE_COLORSTATE(config.Tab.ListText, "ListText");
    WRITE_COLORSTATE(config.Tab.ListHotKey, "ListHotKey");

    WRITE_SECTION("\n[Window]\n");
    WRITE_COLOR(config.Window.Background.Normal, "Normal");
    WRITE_COLOR(config.Window.Background.Inactive, "Inactive");
    WRITE_COLOR(config.Window.Background.Error, "Error");
    WRITE_COLOR(config.Window.Background.Warning, "Warning");
    WRITE_COLOR(config.Window.Background.Info, "Info");

    if (!config.SerializeCustomColors(temp))
        return false;

    output = temp.ToStringView();
    return true;
}
bool Internal::Config::Save(Application::Config& config, const std::filesystem::path& outputFile)
{
    std::string text;
    CHECK(SaveToString(config, text), false, "Fail to serialize the theme");
    return File::WriteContent(outputFile, string_view(text));
}
bool Internal::Config::Load(Application::Config& config, const std::filesystem::path& inputFile)
{
    Utils::IniObject ini;
    CHECK(ini.CreateFromFile(inputFile), false, "");
    return LoadFromIni(config, ini);
}
bool Internal::Config::LoadFromString(Application::Config& config, string_view text)
{
    Utils::IniObject ini;
    CHECK(ini.CreateFromString(text), false, "");
    return LoadFromIni(config, ini);
}
bool Internal::Config::LoadFromIni(Application::Config& config, Utils::IniObject& ini)
{
    Utils::IniSection sect;
    if ((sect = ini.GetSection("General")).Exists())
    {
        READ_COLORSTATE(SearchBar, SearchBar);
        READ_COLORSTATE(Border, Border);
        READ_COLORSTATE(Lines, Lines);
        READ_COLORSTATE(Editor, Editor);
        READ_COLORSTATE(LineMarker, LineMarker);
    }
    if ((sect = ini.GetSection("Button")).Exists())
    {
        READ_COLORSTATE(Button.Text, Text);
        READ_COLORSTATE(Button.HotKey, HotKey);
        READ_COLORPAIR(Button.ShadowColor, ShadowColor); // older name of the key
        READ_COLORPAIR(Button.ShadowColor, Shadow);      // the name Save writes
    }
    if ((sect = ini.GetSection("Text")).Exists())
    {
        READ_COLORPAIR(Text.Normal, Normal);
        READ_COLORPAIR(Text.HotKey, HotKey);
        READ_COLORPAIR(Text.Inactive, Inactive);
        READ_COLORPAIR(Text.Error, Error);
        READ_COLORPAIR(Text.Warning, Warning);
        READ_COLORPAIR(Text.Hovered, Hovered);
        READ_COLORPAIR(Text.Focused, Focused);
        READ_COLORPAIR(Text.Highlighted, Highlighted);
        READ_COLORPAIR(Text.Emphasized1, Emphasized1);
        READ_COLORPAIR(Text.Emphasized2, Emphasized2);
        READ_COLORPAIR(Text.Emphasized3, Emphasized3);
    }
    if ((sect = ini.GetSection("Symbol")).Exists())
    {
        READ_COLORPAIR(Symbol.Inactive, Inactive);
        READ_COLORPAIR(Symbol.Hovered, Hovered);
        READ_COLORPAIR(Symbol.Pressed, Pressed);
        READ_COLORPAIR(Symbol.Checked, Checked);
        READ_COLORPAIR(Symbol.Unchecked, Unchecked);
        READ_COLORPAIR(Symbol.Unknown, Unknown);
        READ_COLORPAIR(Symbol.Desktop, Desktop);
        READ_COLORPAIR(Symbol.Arrows, Arrows);
        READ_COLORPAIR(Symbol.Close, Close);
        READ_COLORPAIR(Symbol.Maximized, Maximized);
        READ_COLORPAIR(Symbol.Resize, Resize);
    }
    if ((sect = ini.GetSection("Cursor")).Exists())
    {
        READ_COLORPAIR(Cursor.Normal, Normal);
        READ_COLORPAIR(Cursor.Inactive, Inactive);
        READ_COLORPAIR(Cursor.OverInactiveItem, OverInactiveItem);
        READ_COLORPAIR(Cursor.OverSelection, OverSelection);
    }
    if ((sect = ini.GetSection("Selection")).Exists())
    {
        READ_COLORPAIR(Selection.Editor, Editor);
        READ_COLORPAIR(Selection.LineMarker, LineMarker);
        READ_COLORPAIR(Selection.Text, Text);
        READ_COLORPAIR(Selection.SearchMarker, SearchMarker);
        READ_COLORPAIR(Selection.SimilarText, SimilarText);
    }
    if ((sect = ini.GetSection("ProgressStatus")).Exists())
    {
        READ_COLORPAIR(ProgressStatus.Empty, Empty);
        READ_COLORPAIR(ProgressStatus.Full, Full);
    }
    if ((sect = ini.GetSection("Menu")).Exists())
    {
        READ_COLORSTATE(Menu.Text, Text);
        READ_COLORSTATE(Menu.HotKey, HotKey);
        READ_COLORSTATE(Menu.ShortCut, ShortCut);
        READ_COLORSTATE(Menu.Symbol, Symbol);
    }
    if ((sect = ini.GetSection("ParentMenu")).Exists())
    {
        READ_COLORSTATE(ParentMenu.Text, Text);
        READ_COLORSTATE(ParentMenu.HotKey, HotKey);
        READ_COLORSTATE(ParentMenu.ShortCut, ShortCut);
        READ_COLORSTATE(ParentMenu.Symbol, Symbol);
    }
    if ((sect = ini.GetSection("Header")).Exists())
    {
        READ_COLORSTATE(Header.Text, Text);
        READ_COLORSTATE(Header.HotKey, HotKey);
        READ_COLORSTATE(Header.Symbol, Symbol);
    }
    if ((sect = ini.GetSection("ScrollBar")).Exists())
    {
        READ_COLORSTATE(ScrollBar.Bar, Bar);
        READ_COLORSTATE(ScrollBar.Arrows, Arrows);
        READ_COLORSTATE(ScrollBar.Position, Position);
    }
    if ((sect = ini.GetSection("ToolTip")).Exists())
    {
        READ_COLORPAIR(ToolTip.Text, Text);
        READ_COLORPAIR(ToolTip.Arrow, Arrow);
    }
    if ((sect = ini.GetSection("Tab")).Exists())
    {
        READ_COLORSTATE(Tab.Text, Text);
        READ_COLORSTATE(Tab.HotKey, HotKey);
        READ_COLORSTATE(Tab.ListText, ListText);
        READ_COLORSTATE(Tab.ListHotKey, ListHotKey);
    }
    if ((sect = ini.GetSection("Window")).Exists())
    {
        READ_COLOR(Window.Background.Normal, Normal);
        READ_COLOR(Window.Background.Inactive, Inactive);
        READ_COLOR(Window.Background.Error, Error);
        READ_COLOR(Window.Background.Warning, Warning);
        READ_COLOR(Window.Background.Info, Info);
    }

    return config.DeserializeCustomColors(ini);
}
void Internal::Config::SetTheme(Application::Config& config, ThemeType theme)
{
    switch (theme)
    {
    case ThemeType::Default:
        Config_SetDefaultTheme(config);
        break;
    case ThemeType::Dark:
        Config_SetDarkTheme(config);
        break;
    case ThemeType::Light:
        Config_SetLightTheme(config);
        break;
    case ThemeType::HighContrast:
        Config_SetPaletteTheme(config, HIGH_CONTRAST_THEME);
        break;
    case ThemeType::Midnight:
        Config_SetPaletteTheme(config, MIDNIGHT_THEME);
        break;
    case ThemeType::Ocean:
        Config_SetPaletteTheme(config, OCEAN_THEME);
        break;
    case ThemeType::Forest:
        Config_SetPaletteTheme(config, FOREST_THEME);
        break;
    case ThemeType::RetroGreen:
        Config_SetPaletteTheme(config, RETRO_GREEN_THEME);
        break;
    default:
        Config_SetDefaultTheme(config);
        break;
    }
}

std::string stringVectorToInitField(std::string_view fieldName, std::vector<std::string>& data)
{
    const char* first = data.empty() ? "" : data[0].c_str();
    LocalString<512> entriesString;
    entriesString.SetFormat("%s", first);
    for (size_t i = 1; i < data.size(); i++)
    {
        entriesString.AddFormat(",%s", data[i].c_str());
    }

    LocalString<512> buffer;
    buffer.SetFormat("%s = [%s]", fieldName.data(), entriesString.GetText());
    std::string res = buffer.GetText();
    return res;
}

bool Application::Config::SerializeCustomColors(AppCUI::Utils::LocalString<8192>& bufferToSerializeTo)
{
    AppCUI::Utils::LocalString<8192>& temp = bufferToSerializeTo;
    LocalString<128> sectionName;
    for (const auto& [categoryName, categoryColors] : CustomColors)
    {
        sectionName.SetFormat("\n[CustomColors.%s]\n", categoryName.c_str());
        WRITE_SECTION(sectionName.GetText());
        {
            std::vector<std::string> fieldNames, fieldTypes;
            fieldNames.reserve(categoryColors.data.size());
            fieldTypes.reserve(categoryColors.data.size());
            for (const auto& [colorName, customColor] : categoryColors.data)
            {
                fieldNames.push_back(colorName);
                if (customColor.IsColorPair())
                {
                    fieldTypes.emplace_back("ColorPair");
                }
                else if (customColor.IsColorState())
                {
                    fieldTypes.emplace_back("ColorState");
                }
            }
            auto fieldNamesIni = stringVectorToInitField("FieldNames", fieldNames);
            auto fieldTypesIni = stringVectorToInitField("FieldTypes", fieldTypes);
            temp.AddFormat("%s\n%s\n", fieldNamesIni.c_str(), fieldTypesIni.c_str());
        }
        
        for (const auto& [colorName, customColor] : categoryColors.data)
        {
            if (customColor.IsColorPair())
            {
                const auto* colorPair = customColor.TryGetColorPair();
                WRITE_COLORPAIR(*colorPair, colorName.c_str());
            }
            else if (customColor.IsColorState())
            {
                const auto* colorState = customColor.TryGetColorState();
                WRITE_COLORSTATE(*colorState, colorName.c_str());
            }
            else
            {
                assert(false); // should not happen
                // if we reach here, it means that the custom color is a single Color
                // should be implemented if needed
            }
        }
    }
    return true;
}

bool Config::DeserializeCustomColors(Utils::IniObject& configFile)
{
    for (auto it : configFile)
    {
        auto secName = it.GetName();
        auto nameIt  = secName.find("CustomColors.");
        if (nameIt == std::string::npos)
            continue;
        auto categoryName      = secName.substr(13 /*strlen("CustomColors.")*/, secName.size() - nameIt);
        Utils::IniSection sect = configFile.GetSection(secName);
        if (!sect.Exists())
        {
            assert(false); // should not happen
            continue;
        }
        auto fieldNamesIni = sect.GetValue("FieldNames");
        auto fieldTypesIni = sect.GetValue("FieldTypes");
        if (!fieldNamesIni.HasValue() || !fieldTypesIni.HasValue() || !fieldNamesIni.IsArray() ||
            !fieldTypesIni.IsArray() || fieldNamesIni.GetArrayCount() != fieldTypesIni.GetArrayCount())
            continue;
        auto count                                                = fieldNamesIni.GetArrayCount();
        Application::Config::CustomColorNameStorage colorsStorage = {};
        for (uint32 i = 0; i < count; i++)
        {
            auto entryName = fieldNamesIni[i].ToStringView();
            auto entryType = fieldTypesIni[i].ToStringView();
            if (entryName.empty() || entryType.empty())
                break;
            if (entryType == "ColorState")
            {
                static_assert(
                      OBJECT_COLOR_STATE_COUNT == 5, "If OBJECT_COLOR_STATE_COUNT changes, update the code below!");
                LocalString<128> buffer;
                ObjectColorState colorState = {};
                for (uint32 state = 0; state < OBJECT_COLOR_STATE_COUNT; ++state)
                {
                    buffer.SetFormat("%s.%s", entryName.data(), OBJECT_COLOR_STATE_NAMES[state].data());
                    auto colorPair               = sect.GetValue(buffer.GetText()).ToColorPair();
                    colorState.StatesList[state] = colorPair;
                }
                colorsStorage[entryName.data()] = CustomColor(colorState);
                continue;
            }
            if (entryType == "ColorPair")
            {
                auto colorPair                  = sect.GetValue(entryName).ToColorPair();
                colorsStorage[entryName.data()] = CustomColor(colorPair);
                continue;
            }
            // else: unknown type -> skip
            LOG_WARNING("Unknown entryType: %s", entryType.data());
        }
        // overlay the loaded values: the colors of the category that the theme does not mention (e.g. registered after
        // the theme was saved) keep their values, so a listener can always read every registered name
        auto& category = CustomColors[std::string(categoryName)];
        for (auto& [colorName, color] : colorsStorage)
            category.data[colorName] = std::move(color);
    }
    return true;
}
} // namespace AppCUI
