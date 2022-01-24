#include "AppCUI.hpp"
namespace AppCUI::Application
{
using namespace Graphics;

void Config::SetDarkTheme()
{
    this->Tree.Text.Normal          = ColorPair{ Color::White, Color::Transparent };
    this->Tree.Text.Focused         = ColorPair{ Color::Black, Color::White };
    this->Tree.Text.Inactive        = ColorPair{ Color::Gray, Color::Black };
    this->Tree.Text.Filter          = ColorPair{ Color::White, Color::DarkRed };
    this->Tree.Text.SearchActive    = ColorPair{ Color::Silver, Color::Transparent };
    this->Tree.Symbol.Collapsed     = ColorPair{ Color::Green, Color::Transparent };
    this->Tree.Symbol.Expanded      = ColorPair{ Color::Red, Color::Transparent };
    this->Tree.Symbol.SingleElement = ColorPair{ Color::Black, Color::Transparent };
    this->Tree.Column.Text          = ColorPair{ Color::Gray, Color::Transparent };
    this->Tree.Column.Header        = ColorPair{ Color::Transparent, Color::Pink };
    this->Tree.Separator.Normal     = ColorPair{ Color::Gray, Color::Transparent };
    this->Tree.Separator.Focused    = ColorPair{ Color::Gray, Color::Pink };

    this->Grid.Lines.Normal              = ColorPair{ Color::White, Color::Transparent };
    this->Grid.Lines.Selected            = ColorPair{ Color::White, Color::Transparent };
    this->Grid.Lines.Hovered             = ColorPair{ Color::Yellow, Color::Transparent };
    this->Grid.Lines.Duplicate           = ColorPair{ Color::Green, Color::Transparent };
    this->Grid.Background.Grid           = ColorPair{ Color::Transparent, Color::DarkBlue };
    this->Grid.Background.Cell.Normal    = ColorPair{ Color::Transparent, Color::DarkBlue };
    this->Grid.Background.Cell.Selected  = ColorPair{ Color::Transparent, Color::White };
    this->Grid.Background.Cell.Hovered   = ColorPair{ Color::Transparent, Color::Yellow };
    this->Grid.Background.Cell.Duplicate = ColorPair{ Color::Transparent, Color::Green };
    this->Grid.Text.Normal               = ColorPair{ Color::White, Color::Transparent };
    this->Grid.Text.Selected             = ColorPair{ Color::Gray, Color::Transparent };
    this->Grid.Text.Hovered              = ColorPair{ Color::Black, Color::Transparent };
    this->Grid.Text.Duplicate            = ColorPair{ Color::Black, Color::Transparent };
    this->Grid.Header                    = ColorPair{ Color::Black, Color::Magenta };

    //=========================================[NEW FORMAT]=================================
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
          { Color::Yellow, Color::Magenta });

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
    this->Cursor.OverSelectection = { Color::Red, Color::Yellow };
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
    this->Symbol.Uncheked  = { Color::Red, Color::Transparent };
    this->Symbol.Unknwon   = { Color::Olive, Color::Transparent };
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

    this->Menu.ScrollButtons.Set(
          { Color::DarkBlue, Color::White },
          { Color::DarkBlue, Color::White },
          { Color::Gray, Color::White },
          { Color::Magenta, Color::White },
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

    this->ParentMenu.ScrollButtons.Set({ Color::Gray, Color::Silver });

    this->ParentMenu.ShortCut = this->ParentMenu.HotKey;

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
