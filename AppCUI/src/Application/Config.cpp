#include "AppCUI.hpp"
namespace AppCUI::Application
{
using namespace Graphics;

void Config::SetDarkTheme()
{
    this->SearchBar.Set(
          { Color::White, Color::DarkRed },
          { Color::Silver, Color::DarkRed },
          { Color::Gray, Color::DarkRed },
          { Color::Yellow, Color::DarkRed });

    this->Border.Set(Color::White, Color::Silver, Color::Gray, Color::Yellow, Color::Transparent);

    this->Lines.Set(
          { Color::DarkGreen, Color::Transparent },
          { Color::DarkGreen, Color::Transparent },
          { Color::Gray, Color::Transparent },
          { Color::Yellow, Color::Magenta },
          { Color::Black, Color::White });

    this->Editor.Set(
          { Color::White, Color::Black },
          { Color::Silver, Color::Black },
          { Color::Gray, Color::Transparent },
          { Color::Yellow, Color::Black });

    this->LineMarker.Set(
          { Color::Black, Color::Gray },
          { Color::White, Color::Blue },
          { Color::Gray, Color::Transparent },
          { Color::Yellow, Color::Blue });

    this->PasswordMarker.Set(
          { Color::Aqua, Color::Black },
          { Color::Silver, Color::Black },
          { Color::Gray, Color::Transparent },
          { Color::Yellow, Color::Black });

    this->Button.Text.Set(
          { Color::Black, Color::White },
          { Color::Black, Color::Gray },
          { Color::Gray, Color::Black },
          { Color::Black, Color::Yellow },
          { Color::Black, Color::Olive });
    this->Button.HotKey.Set(
          { Color::Magenta, Color::White },
          { Color::DarkRed, Color::Gray },
          { Color::Gray, Color::Black },
          { Color::Magenta, Color::Yellow },
          { Color::DarkRed, Color::Olive });
        this->Button.ShadowColor = { Color::Black, Color::Transparent };

    this->Text.Error       = { Color::Red, Color::Transparent };
    this->Text.Warning     = { Color::Olive, Color::Transparent };
    this->Text.Normal      = { Color::Silver, Color::Transparent };
    this->Text.Focused     = { Color::White, Color::Transparent };
    this->Text.Inactive    = { Color::Gray, Color::Transparent };
    this->Text.HotKey      = { Color::Aqua, Color::Transparent };
    this->Text.Hovered     = { Color::Yellow, Color::Transparent };
    this->Text.Highlighted = { Color::Yellow, Color::Transparent };
    this->Text.Emphasized1 = { Color::Aqua, Color::Transparent };
    this->Text.Emphasized2 = { Color::Green, Color::Transparent };

    this->Cursor.Normal           = { Color::Black, Color::White };
    this->Cursor.OverInactiveItem = { Color::Gray, Color::White };
    this->Cursor.OverSelection    = { Color::Red, Color::Yellow };
    this->Cursor.Inactive         = { Color::Yellow, Color::Transparent };

    this->Selection.Editor       = { Color::Yellow, Color::Magenta };
    this->Selection.LineMarker   = { Color::Yellow, Color::Magenta };
    this->Selection.Text         = { Color::Yellow, Color::Black };
    this->Selection.SearchMarker = { Color::Yellow, Color::DarkRed };
    this->Selection.SimilarText  = { Color::Black, Color::Green };

    this->Symbol.Inactive  = { Color::Gray, Color::Transparent };
    this->Symbol.Hovered   = { Color::Black, Color::Yellow };
    this->Symbol.Pressed   = { Color::Black, Color::Silver };
    this->Symbol.Checked   = { Color::Green, Color::Transparent };
    this->Symbol.Unchecked = { Color::Red, Color::Transparent };
    this->Symbol.Unknown   = { Color::Olive, Color::Transparent };
    this->Symbol.Desktop   = { Color::Gray, Color::Black };
    this->Symbol.Arrows    = { Color::Aqua, Color::Transparent };
    this->Symbol.Close     = { Color::Red, Color::Transparent };
    this->Symbol.Maximized = { Color::Aqua, Color::Transparent };
    this->Symbol.Resize    = { Color::Aqua, Color::Transparent };

    this->ProgressStatus.Empty = { Color::White, Color::Black };
    this->ProgressStatus.Full  = { Color::White, Color::Teal };

    this->Menu.Text.Set(
          { Color::Black, Color::White },
          { Color::Black, Color::White },
          { Color::Gray, Color::White },
          { Color::Black, Color::Silver },
          { Color::Yellow, Color::Magenta });

    this->Menu.HotKey.Set(
          { Color::DarkRed, Color::White },
          { Color::DarkRed, Color::White },
          { Color::Gray, Color::White },
          { Color::DarkRed, Color::Silver },
          { Color::White, Color::Magenta });

    this->Menu.Symbol.Set(
          { Color::DarkGreen, Color::White },
          { Color::DarkGreen, Color::White },
          { Color::Gray, Color::White },
          { Color::Magenta, Color::Silver },
          { Color::White, Color::Magenta });

    this->Menu.ShortCut = this->Menu.HotKey;

    this->ParentMenu.Text.Set(
          { Color::Black, Color::Silver },
          { Color::Black, Color::Silver },
          { Color::Gray, Color::Silver },
          { Color::Black, Color::Gray },
          { Color::Yellow, Color::Gray });

    this->ParentMenu.HotKey.Set(
          { Color::DarkRed, Color::Silver },
          { Color::DarkRed, Color::Silver },
          { Color::Gray, Color::Silver },
          { Color::DarkRed, Color::Gray },
          { Color::White, Color::Gray });


    this->ParentMenu.ShortCut = this->ParentMenu.HotKey;

    this->ParentMenu.Symbol.Set(
          { Color::DarkGreen, Color::Silver },
          { Color::DarkGreen, Color::Silver },
          { Color::Gray, Color::Silver },
          { Color::Magenta, Color::Gray },
          { Color::White, Color::Gray });

    this->Header.Text.Set(
          { Color::White, Color::Magenta },
          { Color::Silver, Color::Magenta },
          { Color::Gray, Color::Transparent },
          { Color::DarkRed, Color::Silver },
          { Color::White, Color::Pink });
    this->Header.HotKey.Set(
          { Color::Yellow, Color::Magenta },
          { Color::Yellow, Color::Magenta },
          { Color::Gray, Color::Transparent },
          { Color::Red, Color::Silver },
          { Color::Yellow, Color::Pink });
    this->Header.Symbol = this->Header.Text;

    this->ScrollBar.Bar.Set(
          { Color::White, Color::Teal },
          { Color::White, Color::Teal },
          { Color::Gray, Color::Transparent },
          { Color::Yellow, Color::Silver });
    this->ScrollBar.Arrows = this->ScrollBar.Bar;
    this->ScrollBar.Position.Set(
          { Color::Green, Color::Teal },
          { Color::Green, Color::Teal },
          { Color::Gray, Color::Transparent },
          { Color::Yellow, Color::Silver });

    this->ToolTip.Arrow = { Color::Green, Color::Black };
    this->ToolTip.Text  = { Color::Black, Color::Aqua };

    this->Tab.Text.Set(
          { Color::Black, Color::Gray },
          { Color::White, Color::Gray },
          { Color::Gray, Color::Transparent },
          { Color::Black, Color::Silver },
          { Color::White, Color::Blue });

    this->Tab.HotKey.Set(
          { Color::DarkRed, Color::Gray },
          { Color::Yellow, Color::Gray },
          { Color::Gray, Color::Transparent },
          { Color::DarkRed, Color::Silver },
          { Color::Yellow, Color::Blue });

    this->Tab.ListText                     = this->Tab.Text;
    this->Tab.ListText.PressedOrSelected   = { Color::Black, Color::White };
    this->Tab.ListHotKey                   = this->Tab.HotKey;
    this->Tab.ListHotKey.PressedOrSelected = { Color::DarkRed, Color::White };

    this->Window.Background.Inactive = Color::Black;
    this->Window.Background.Normal   = Color::DarkBlue;
    this->Window.Background.Error    = Color::DarkRed;
    this->Window.Background.Warning  = Color::Olive;
    this->Window.Background.Info     = Color::DarkGreen;
}
} // namespace AppCUI::Application
