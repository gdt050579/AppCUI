#include "AppCUI.hpp"
namespace AppCUI::Application
{
using namespace Graphics;
using namespace Utils;
using namespace OS;

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
          { Color::Yellow, Color::Magenta },
          { Color::Black, Color::White });

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
          { Color::Blue, Color::Transparent },
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
    cfg.Cursor.Inactive         = { Color::Gray, Color::Transparent };
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
    CHECK(WriteKeyToString(col.Normal, name, "Regular", output), false, "");
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
    this->key.Focused           = sect.GetValue(#name ".Focused").ToColorPair();                                       \
    this->key.Normal            = sect.GetValue(#name ".Normal").ToColorPair();                                        \
    this->key.Inactive          = sect.GetValue(#name ".Inactive").ToColorPair();                                      \
    this->key.Hovered           = sect.GetValue(#name ".Hovered").ToColorPair();                                       \
    this->key.PressedOrSelected = sect.GetValue(#name ".Pressed").ToColorPair();
#define READ_COLORPAIR(key, name) this->key = sect.GetValue(#name).ToColorPair();
#define READ_COLOR(key, name)     this->key = sect.GetValue(#name).ToColor();

bool Config::Save(const std::filesystem::path& outputFile)
{
    AppCUI::Utils::LocalString<8192> temp;

    // sections
    WRITE_SECTION("[General]\n");
    WRITE_COLORSTATE(this->SearchBar, "SearchBar");
    WRITE_COLORSTATE(this->Border, "Border");
    WRITE_COLORSTATE(this->Lines, "Lines");
    WRITE_COLORSTATE(this->Editor, "Editor");
    WRITE_COLORSTATE(this->LineMarker, "LineMarker");

    WRITE_SECTION("\n[Button]\n");
    WRITE_COLORSTATE(this->Button.Text, "Text");
    WRITE_COLORSTATE(this->Button.HotKey, "HotKey");
    WRITE_COLORPAIR(this->Button.ShadowColor, "Shadow");

    WRITE_SECTION("\n[Text]\n");
    WRITE_COLORPAIR(this->Text.Normal, "Normal");
    WRITE_COLORPAIR(this->Text.HotKey, "HotKey");
    WRITE_COLORPAIR(this->Text.Inactive, "Inactive");
    WRITE_COLORPAIR(this->Text.Error, "Error");
    WRITE_COLORPAIR(this->Text.Warning, "Warning");
    WRITE_COLORPAIR(this->Text.Hovered, "Hovered");
    WRITE_COLORPAIR(this->Text.Focused, "Focused");
    WRITE_COLORPAIR(this->Text.Highlighted, "Highlighted");
    WRITE_COLORPAIR(this->Text.Emphasized1, "Emphasized1");
    WRITE_COLORPAIR(this->Text.Emphasized2, "Emphasized2");

    WRITE_SECTION("\n[Symbol]\n");
    WRITE_COLORPAIR(this->Symbol.Inactive, "Inactive");
    WRITE_COLORPAIR(this->Symbol.Hovered, "Hovered");
    WRITE_COLORPAIR(this->Symbol.Pressed, "Pressed");
    WRITE_COLORPAIR(this->Symbol.Checked, "Checked");
    WRITE_COLORPAIR(this->Symbol.Unchecked, "Unchecked");
    WRITE_COLORPAIR(this->Symbol.Unknown, "Unknown");
    WRITE_COLORPAIR(this->Symbol.Desktop, "Desktop");
    WRITE_COLORPAIR(this->Symbol.Arrows, "Arrows");
    WRITE_COLORPAIR(this->Symbol.Close, "Close");
    WRITE_COLORPAIR(this->Symbol.Maximized, "Maximized");
    WRITE_COLORPAIR(this->Symbol.Resize, "Resize");

    WRITE_SECTION("\n[Cursor]\n");
    WRITE_COLORPAIR(this->Cursor.Inactive, "Inactive");
    WRITE_COLORPAIR(this->Cursor.Normal, "Normal");
    WRITE_COLORPAIR(this->Cursor.OverInactiveItem, "OverInactiveItem");
    WRITE_COLORPAIR(this->Cursor.OverSelection, "OverSelection");

    WRITE_SECTION("\n[Selection]\n");
    WRITE_COLORPAIR(this->Selection.Editor, "Editor");
    WRITE_COLORPAIR(this->Selection.LineMarker, "LineMarker");
    WRITE_COLORPAIR(this->Selection.Text, "Text");
    WRITE_COLORPAIR(this->Selection.SearchMarker, "SearchMarker");
    WRITE_COLORPAIR(this->Selection.SimilarText, "SimilarText");

    WRITE_SECTION("\n[ProgressStatus]\n");
    WRITE_COLORPAIR(this->ProgressStatus.Empty, "Empty");
    WRITE_COLORPAIR(this->ProgressStatus.Full, "Full");

    WRITE_SECTION("\n[Menu]\n");
    WRITE_COLORSTATE(this->Menu.Text, "Text");
    WRITE_COLORSTATE(this->Menu.HotKey, "HotKey");
    WRITE_COLORSTATE(this->Menu.ShortCut, "ShortCut");
    WRITE_COLORSTATE(this->Menu.Symbol, "Symbol");

    WRITE_SECTION("\n[ParentMenu]\n");
    WRITE_COLORSTATE(this->ParentMenu.Text, "Text");
    WRITE_COLORSTATE(this->ParentMenu.HotKey, "HotKey");
    WRITE_COLORSTATE(this->ParentMenu.ShortCut, "ShortCut");
    WRITE_COLORSTATE(this->ParentMenu.Symbol, "Symbol");

    WRITE_SECTION("\n[Header]\n");
    WRITE_COLORSTATE(this->Header.Text, "Text");
    WRITE_COLORSTATE(this->Header.HotKey, "HotKey");
    WRITE_COLORSTATE(this->Header.Symbol, "Symbol");

    WRITE_SECTION("\n[ScrollBar]\n");
    WRITE_COLORSTATE(this->ScrollBar.Bar, "Bar");
    WRITE_COLORSTATE(this->ScrollBar.Arrows, "Arrows");
    WRITE_COLORSTATE(this->ScrollBar.Position, "Position");

    WRITE_SECTION("\n[ToolTip]\n");
    WRITE_COLORPAIR(this->ToolTip.Text, "Text");
    WRITE_COLORPAIR(this->ToolTip.Arrow, "Arrow");

    WRITE_SECTION("\n[Tab]\n");
    WRITE_COLORSTATE(this->Tab.Text, "Text");
    WRITE_COLORSTATE(this->Tab.HotKey, "HotKey");
    WRITE_COLORSTATE(this->Tab.ListText, "ListText");
    WRITE_COLORSTATE(this->Tab.ListHotKey, "ListHotKey");

    WRITE_SECTION("\n[Window]\n");
    WRITE_COLOR(this->Window.Background.Normal, "Normal");
    WRITE_COLOR(this->Window.Background.Inactive, "Inactive");
    WRITE_COLOR(this->Window.Background.Error, "Error");
    WRITE_COLOR(this->Window.Background.Warning, "Warning");
    WRITE_COLOR(this->Window.Background.Info, "Info");

    // save
    return File::WriteContent(outputFile, temp.ToStringView());
}
bool Config::Load(const std::filesystem::path& inputFile)
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
    return true;
}
void Config::SetTheme(ThemeType theme)
{
    switch (theme)
    {
    case ThemeType::Default:
        Config_SetDefaultTheme(*this);
        break;
    case ThemeType::Dark:
        Config_SetDarkTheme(*this);
        break;
    case ThemeType::Light:
        Config_SetLightTheme(*this);
        break;
    default:
        Config_SetDefaultTheme(*this);
        break;
    }
}
} // namespace AppCUI::Application
