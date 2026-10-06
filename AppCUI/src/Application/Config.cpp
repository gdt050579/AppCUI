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

#define READ_COLORSTATE(key, name)                                                                                     \
    config.key.Focused           = sect.GetValue(#name ".Focused").ToColorPair();                                      \
    config.key.Normal            = sect.GetValue(#name ".Normal").ToColorPair();                                       \
    config.key.Inactive          = sect.GetValue(#name ".Inactive").ToColorPair();                                     \
    config.key.Hovered           = sect.GetValue(#name ".Hovered").ToColorPair();                                      \
    config.key.PressedOrSelected = sect.GetValue(#name ".Pressed").ToColorPair();
#define READ_COLORPAIR(key, name) config.key = sect.GetValue(#name).ToColorPair();
#define READ_COLOR(key, name)     config.key = sect.GetValue(#name).ToColor();

bool Internal::Config::Save(Application::Config& config, const std::filesystem::path& outputFile)
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

    // save
    return File::WriteContent(outputFile, temp.ToStringView());
}
bool Internal::Config::Load(Application::Config& config, const std::filesystem::path& inputFile)
{
    Utils::IniObject ini;
    Utils::IniSection sect;
    CHECK(ini.CreateFromFile(inputFile), false, "");
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
        READ_COLORPAIR(Button.ShadowColor, ShadowColor);
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
        CategoryColorsData categoryColorsData   = {};
        categoryColorsData.data                 = std::move(colorsStorage);
        auto existingCategory                   = CustomColors.find(std::string(categoryName));
        if (existingCategory != CustomColors.end())
            categoryColorsData.previewInterface = existingCategory->second.previewInterface;
        CustomColors[std::string(categoryName)] = std::move(categoryColorsData);
    }
    return true;
}
} // namespace AppCUI
