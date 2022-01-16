#include "AppCUI.hpp"
namespace AppCUI::Application
{
using namespace Graphics;

void Config::SetDarkTheme()
{
    this->Desktop.Color                    = ColorPair{ Color::Gray, Color::Black };
    this->Desktop.DesktopFillCharacterCode = 186;

    this->Window.ActiveColor        = ColorPair{ Color::White, Color::DarkBlue };
    this->Window.InactiveColor      = ColorPair{ Color::Silver, Color::Black };
    this->Window.TitleActiveColor   = ColorPair{ Color::Yellow, Color::DarkBlue };
    this->Window.TitleInactiveColor = ColorPair{ Color::Silver, Color::Black };

    this->Window.ControlBar.Separators.Focused  = ColorPair{ Color::Yellow, Color::DarkBlue };
    this->Window.ControlBar.Separators.Normal   = ColorPair{ Color::Silver, Color::Black };
    this->Window.ControlBar.Item.Normal.Text    = ColorPair{ Color::Gray, Color::Black };
    this->Window.ControlBar.Item.Normal.HotKey  = ColorPair{ Color::Silver, Color::Black };
    this->Window.ControlBar.Item.Focused.Text   = ColorPair{ Color::Aqua, Color::DarkBlue };
    this->Window.ControlBar.Item.Focused.HotKey = ColorPair{ Color::White, Color::DarkBlue };
    this->Window.ControlBar.Item.Hover.Text     = ColorPair{ Color::Black, Color::Aqua };
    this->Window.ControlBar.Item.Hover.HotKey   = ColorPair{ Color::Black, Color::Aqua };
    this->Window.ControlBar.Item.Pressed.Text   = ColorPair{ Color::Black, Color::Yellow };
    this->Window.ControlBar.Item.Pressed.HotKey = ColorPair{ Color::Black, Color::Yellow };
    this->Window.ControlBar.Item.Checked.Text   = ColorPair{ Color::Black, Color::Gray };
    this->Window.ControlBar.Item.Checked.HotKey = ColorPair{ Color::White, Color::Gray };
    this->Window.ControlBar.CloseButton         = ColorPair{ Color::Red, Color::DarkBlue };
    this->Window.ControlBar.Tag                 = ColorPair{ Color::Green, Color::DarkBlue };
    this->Window.ControlBar.CheckMark           = ColorPair{ Color::White, Color::DarkBlue };
    this->Window.ControlBar.Text                = ColorPair{ Color::Silver, Color::DarkBlue };

    this->DialogError.ActiveColor        = ColorPair{ Color::White, Color::DarkRed };
    this->DialogError.InactiveColor      = ColorPair{ Color::Silver, Color::DarkRed };
    this->DialogError.TitleActiveColor   = ColorPair{ Color::Yellow, Color::DarkRed };
    this->DialogError.TitleInactiveColor = ColorPair{ Color::Silver, Color::DarkRed };

    this->DialogError.ControlBar.Separators.Focused  = ColorPair{ Color::Yellow, Color::DarkRed };
    this->DialogError.ControlBar.Separators.Normal   = ColorPair{ Color::Silver, Color::DarkRed };
    this->DialogError.ControlBar.Item.Normal.Text    = ColorPair{ Color::Gray, Color::DarkRed };
    this->DialogError.ControlBar.Item.Normal.HotKey  = ColorPair{ Color::Silver, Color::DarkRed };
    this->DialogError.ControlBar.Item.Focused.Text   = ColorPair{ Color::Aqua, Color::DarkRed };
    this->DialogError.ControlBar.Item.Focused.HotKey = ColorPair{ Color::White, Color::DarkRed };
    this->DialogError.ControlBar.Item.Hover.Text     = ColorPair{ Color::Black, Color::Aqua };
    this->DialogError.ControlBar.Item.Hover.HotKey   = ColorPair{ Color::Black, Color::Aqua };
    this->DialogError.ControlBar.Item.Pressed.Text   = ColorPair{ Color::Black, Color::Yellow };
    this->DialogError.ControlBar.Item.Pressed.HotKey = ColorPair{ Color::Black, Color::Yellow };
    this->DialogError.ControlBar.Item.Checked.Text   = ColorPair{ Color::White, Color::Gray };
    this->DialogError.ControlBar.Item.Checked.HotKey = ColorPair{ Color::Silver, Color::Gray };
    this->DialogError.ControlBar.CloseButton         = ColorPair{ Color::Yellow, Color::DarkRed };
    this->DialogError.ControlBar.Tag                 = ColorPair{ Color::Silver, Color::DarkRed };
    this->DialogError.ControlBar.CheckMark           = ColorPair{ Color::Green, Color::DarkRed };
    this->DialogError.ControlBar.Text                = ColorPair{ Color::Silver, Color::DarkRed };

    this->DialogNotify.ActiveColor        = ColorPair{ Color::White, Color::DarkGreen };
    this->DialogNotify.InactiveColor      = ColorPair{ Color::Silver, Color::DarkGreen };
    this->DialogNotify.TitleActiveColor   = ColorPair{ Color::Yellow, Color::DarkGreen };
    this->DialogNotify.TitleInactiveColor = ColorPair{ Color::Silver, Color::DarkGreen };

    this->DialogNotify.ControlBar.Separators.Focused  = ColorPair{ Color::Yellow, Color::DarkGreen };
    this->DialogNotify.ControlBar.Separators.Normal   = ColorPair{ Color::Silver, Color::DarkGreen };
    this->DialogNotify.ControlBar.Item.Normal.Text    = ColorPair{ Color::Gray, Color::DarkGreen };
    this->DialogNotify.ControlBar.Item.Normal.HotKey  = ColorPair{ Color::Silver, Color::DarkGreen };
    this->DialogNotify.ControlBar.Item.Focused.Text   = ColorPair{ Color::Aqua, Color::DarkGreen };
    this->DialogNotify.ControlBar.Item.Focused.HotKey = ColorPair{ Color::White, Color::DarkGreen };
    this->DialogNotify.ControlBar.Item.Hover.Text     = ColorPair{ Color::Black, Color::Aqua };
    this->DialogNotify.ControlBar.Item.Hover.HotKey   = ColorPair{ Color::Black, Color::Aqua };
    this->DialogNotify.ControlBar.Item.Pressed.Text   = ColorPair{ Color::Black, Color::Yellow };
    this->DialogNotify.ControlBar.Item.Pressed.HotKey = ColorPair{ Color::Black, Color::Yellow };
    this->DialogNotify.ControlBar.Item.Checked.Text   = ColorPair{ Color::White, Color::Gray };
    this->DialogNotify.ControlBar.Item.Checked.HotKey = ColorPair{ Color::Silver, Color::Gray };
    this->DialogNotify.ControlBar.CloseButton         = ColorPair{ Color::White, Color::DarkGreen };
    this->DialogNotify.ControlBar.Tag                 = ColorPair{ Color::Yellow, Color::DarkGreen };
    this->DialogNotify.ControlBar.CheckMark           = ColorPair{ Color::Silver, Color::DarkGreen };
    this->DialogNotify.ControlBar.Text                = ColorPair{ Color::Silver, Color::DarkGreen };

    this->DialogWarning.ActiveColor        = ColorPair{ Color::White, Color::Olive };
    this->DialogWarning.InactiveColor      = ColorPair{ Color::Silver, Color::Olive };
    this->DialogWarning.TitleActiveColor   = ColorPair{ Color::Yellow, Color::Olive };
    this->DialogWarning.TitleInactiveColor = ColorPair{ Color::Silver, Color::Olive };

    this->DialogWarning.ControlBar.Separators.Focused  = ColorPair{ Color::Yellow, Color::Olive };
    this->DialogWarning.ControlBar.Separators.Normal   = ColorPair{ Color::Silver, Color::Olive };
    this->DialogWarning.ControlBar.Item.Normal.Text    = ColorPair{ Color::Gray, Color::Olive };
    this->DialogWarning.ControlBar.Item.Normal.HotKey  = ColorPair{ Color::Silver, Color::Olive };
    this->DialogWarning.ControlBar.Item.Focused.Text   = ColorPair{ Color::Aqua, Color::Olive };
    this->DialogWarning.ControlBar.Item.Focused.HotKey = ColorPair{ Color::White, Color::Olive };
    this->DialogWarning.ControlBar.Item.Hover.Text     = ColorPair{ Color::Black, Color::Aqua };
    this->DialogWarning.ControlBar.Item.Hover.HotKey   = ColorPair{ Color::Black, Color::Aqua };
    this->DialogWarning.ControlBar.Item.Pressed.Text   = ColorPair{ Color::Black, Color::Yellow };
    this->DialogWarning.ControlBar.Item.Pressed.HotKey = ColorPair{ Color::Black, Color::Yellow };
    this->DialogWarning.ControlBar.Item.Checked.Text   = ColorPair{ Color::White, Color::Gray };
    this->DialogWarning.ControlBar.Item.Checked.HotKey = ColorPair{ Color::Silver, Color::Gray };
    this->DialogWarning.ControlBar.CloseButton         = ColorPair{ Color::Red, Color::Olive };
    this->DialogWarning.ControlBar.Tag                 = ColorPair{ Color::Green, Color::Olive };
    this->DialogWarning.ControlBar.CheckMark           = ColorPair{ Color::White, Color::Olive };
    this->DialogWarning.ControlBar.Text                = ColorPair{ Color::Silver, Color::Olive };

    this->Splitter.Buttons.Normal  = ColorPair{ Color::Green, Color::Transparent };
    this->Splitter.Buttons.Hover   = ColorPair{ Color::Black, Color::Aqua };
    this->Splitter.Buttons.Clicked = ColorPair{ Color::Red, Color::Transparent };

    this->Tab.PageColor              = ColorPair{ Color::White, Color::Blue };
    this->Tab.PageHotKeyColor        = ColorPair{ Color::Yellow, Color::Blue };
    this->Tab.TabBarColor            = ColorPair{ Color::Black, Color::Gray };
    this->Tab.TabBarHotKeyColor      = ColorPair{ Color::DarkRed, Color::Gray };
    this->Tab.HoverColor             = ColorPair{ Color::Yellow, Color::Magenta };
    this->Tab.HoverHotKeyColor       = ColorPair{ Color::White, Color::Magenta };
    this->Tab.ListSelectedPageColor  = ColorPair{ Color::Black, Color::White };
    this->Tab.ListSelectedPageHotKey = ColorPair{ Color::DarkRed, Color::White };

    this->ScrollBar.Arrows   = ColorPair{ Color::White, Color::Teal };
    this->ScrollBar.Bar      = ColorPair{ Color::Black, Color::Teal };
    this->ScrollBar.Position = ColorPair{ Color::Green, Color::Teal };

    this->ListView.Item.Category = ColorPair{ Color::Yellow, Color::Transparent };

    this->ListView.CheckedSymbol         = ColorPair{ Color::Green, Color::Transparent };
    this->ListView.UncheckedSymbol       = ColorPair{ Color::DarkRed, Color::Transparent };
    this->ListView.InactiveColor         = ColorPair{ Color::Gray, Color::Transparent };
    this->ListView.FocusColor            = ColorPair{ Color::Black, Color::White };
    this->ListView.FocusAndSelectedColor = ColorPair{ Color::Red, Color::White };
    this->ListView.SelectionColor        = ColorPair{ Color::Yellow, Color::Transparent };
    this->ListView.StatusColor           = ColorPair{ Color::Yellow, Color::Transparent };

    this->ListView.Highlight.Normal   = ColorPair{ Color::Gray, Color::Transparent };
    this->ListView.Highlight.Selected = ColorPair{ Color::Yellow, Color::DarkRed };

    this->NumericSelector.Text.Normal     = ColorPair{ Color::Black, Color::Gray };
    this->NumericSelector.Text.Focused    = ColorPair{ Color::Black, Color::White };
    this->NumericSelector.Text.Inactive   = ColorPair{ Color::Gray, Color::Black };
    this->NumericSelector.Text.Hover      = ColorPair{ Color::Black, Color::Yellow };
    this->NumericSelector.Text.WrongValue = ColorPair{ Color::Black, Color::Red };

    this->ToolTip.Arrow = ColorPair{ Color::Green, Color::Black };
    this->ToolTip.Text  = ColorPair{ Color::Black, Color::Aqua };

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

    this->PropertyList.Cursor         = ColorPair{ Color::Black, Color::White };
    this->PropertyList.CursorReadOnly = ColorPair{ Color::Gray, Color::White };
    this->PropertyList.Inactive       = ColorPair{ Color::Gray, Color::Transparent };
    this->PropertyList.Category.Text  = ColorPair{ Color::White, Color::Magenta };
    this->PropertyList.Category.Arrow = ColorPair{ Color::Green, Color::Magenta };
    this->PropertyList.Category.Stats = ColorPair{ Color::Gray, Color::Magenta };
    this->PropertyList.Item.Checked   = ColorPair{ Color::Aqua, Color::Transparent };
    this->PropertyList.Item.Unchecked = ColorPair{ Color::Red, Color::Transparent };

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

    this->TextSelectionColor     = { Color::Yellow, Color::Magenta };
    this->TextSelectedLineMarker = { Color::Yellow, Color::Magenta };

    this->PasswordMarker.Set(
          { Color::Aqua, Color::Black },
          { Color::Silver, Color::Black },
          { Color::Gray, Color::Transparent },
          { Color::Yellow, Color::Black });

    this->Button.Set(
          { Color::Black, Color::White },
          { Color::Black, Color::Gray },
          { Color::Gray, Color::Black },
          { Color::Black, Color::Yellow },
          { Color::Black, Color::Olive });
    this->ButtonHotKey.Set(
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
    this->Text.Cursor      = { Color::Black, Color::White };
    this->Text.Emphasized1 = { Color::Aqua, Color::Transparent };
    this->Text.Emphasized2 = { Color::Green, Color::Transparent };

    this->Bakcground.Focused = Color::DarkBlue;
    this->Bakcground.Regular = Color::Black;
    this->Bakcground.Error   = Color::DarkRed;
    this->Bakcground.Warning = Color::Olive;
    this->Bakcground.Notify  = Color::DarkGreen;
    this->Bakcground.Tab     = Color::Blue;

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
}
} // namespace AppCUI::Application
