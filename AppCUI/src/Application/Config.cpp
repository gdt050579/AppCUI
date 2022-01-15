#include "AppCUI.hpp"
namespace AppCUI::Application
{
using namespace Graphics;

void Config::SetDarkTheme()
{
    this->Desktop.Color                    = ColorPair{ Color::Gray, Color::Black };
    this->Desktop.DesktopFillCharacterCode = 186;

    this->CommandBar.BackgroundColor   = ColorPair{ Color::Black, Color::White };
    this->CommandBar.ShiftKeysColor    = ColorPair{ Color::Gray, Color::White };
    this->CommandBar.Normal.KeyColor   = ColorPair{ Color::DarkRed, Color::White };
    this->CommandBar.Normal.NameColor  = ColorPair{ Color::Black, Color::White };
    this->CommandBar.Hover.KeyColor    = ColorPair{ Color::DarkRed, Color::Silver };
    this->CommandBar.Hover.NameColor   = ColorPair{ Color::Black, Color::Silver };
    this->CommandBar.Pressed.KeyColor  = ColorPair{ Color::White, Color::Magenta };
    this->CommandBar.Pressed.NameColor = ColorPair{ Color::Yellow, Color::Magenta };

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

    this->ListView.ColumnNormal.Text     = ColorPair{ Color::Silver, Color::Magenta };
    this->ListView.ColumnNormal.HotKey   = ColorPair{ Color::Yellow, Color::Transparent };
    this->ListView.ColumnInactive.Text   = ColorPair{ Color::Gray, Color::Transparent };
    this->ListView.ColumnInactive.HotKey = ColorPair{ Color::Gray, Color::Transparent };
    this->ListView.ColumnHover.Text      = ColorPair{ Color::Yellow, Color::Magenta };
    this->ListView.ColumnHover.HotKey    = ColorPair{ Color::Yellow, Color::Magenta };
    this->ListView.ColumnSort.Text       = ColorPair{ Color::White, Color::Pink };
    this->ListView.ColumnSort.HotKey     = ColorPair{ Color::Black, Color::Pink };

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

    this->ProgressStatus.Title            = ColorPair{ Color::Yellow, Color::DarkBlue };
    this->ProgressStatus.Text             = ColorPair{ Color::Silver, Color::DarkBlue };
    this->ProgressStatus.TerminateMessage = ColorPair{ Color::Gray, Color::DarkBlue };
    this->ProgressStatus.EmptyProgressBar = ColorPair{ Color::White, Color::Black };
    this->ProgressStatus.FullProgressBar  = ColorPair{ Color::White, Color::Teal };
    this->ProgressStatus.Time             = ColorPair{ Color::Aqua, Color::DarkBlue };
    this->ProgressStatus.Percentage       = ColorPair{ Color::Yellow, Color::DarkBlue };

    this->Menu.Activ.Background = ColorPair{ Color::Black, Color::White };

    this->Menu.Activ.Normal.Text     = ColorPair{ Color::Black, Color::White };
    this->Menu.Activ.Normal.HotKey   = ColorPair{ Color::DarkRed, Color::White };
    this->Menu.Activ.Normal.ShortCut = ColorPair{ Color::DarkRed, Color::White };
    this->Menu.Activ.Normal.Check    = ColorPair{ Color::DarkGreen, Color::White };
    this->Menu.Activ.Normal.Uncheck  = ColorPair{ Color::DarkBlue, Color::White };

    this->Menu.Activ.Selected.Text     = ColorPair{ Color::White, Color::Magenta };
    this->Menu.Activ.Selected.HotKey   = ColorPair{ Color::Yellow, Color::Magenta };
    this->Menu.Activ.Selected.ShortCut = ColorPair{ Color::Yellow, Color::Magenta };
    this->Menu.Activ.Selected.Check    = ColorPair{ Color::Aqua, Color::Magenta };
    this->Menu.Activ.Selected.Uncheck  = ColorPair{ Color::Silver, Color::Magenta };

    this->Menu.Activ.Inactive.Text     = ColorPair{ Color::Silver, Color::White };
    this->Menu.Activ.Inactive.HotKey   = ColorPair{ Color::Silver, Color::White };
    this->Menu.Activ.Inactive.ShortCut = ColorPair{ Color::Silver, Color::White };
    this->Menu.Activ.Inactive.Check    = ColorPair{ Color::Silver, Color::White };
    this->Menu.Activ.Inactive.Uncheck  = ColorPair{ Color::Silver, Color::White };

    this->Menu.Parent.Background = ColorPair{ Color::Gray, Color::Silver };

    this->Menu.Parent.Normal.Text     = ColorPair{ Color::Black, Color::Silver };
    this->Menu.Parent.Normal.HotKey   = ColorPair{ Color::DarkRed, Color::Silver };
    this->Menu.Parent.Normal.ShortCut = ColorPair{ Color::DarkRed, Color::Silver };
    this->Menu.Parent.Normal.Check    = ColorPair{ Color::DarkGreen, Color::Silver };
    this->Menu.Parent.Normal.Uncheck  = ColorPair{ Color::DarkBlue, Color::Silver };

    this->Menu.Parent.Selected.Text     = ColorPair{ Color::White, Color::Gray };
    this->Menu.Parent.Selected.HotKey   = ColorPair{ Color::White, Color::Gray };
    this->Menu.Parent.Selected.ShortCut = ColorPair{ Color::White, Color::Gray };
    this->Menu.Parent.Selected.Check    = ColorPair{ Color::White, Color::Gray };
    this->Menu.Parent.Selected.Uncheck  = ColorPair{ Color::White, Color::Gray };

    this->Menu.Parent.Inactive.Text     = ColorPair{ Color::Gray, Color::Silver };
    this->Menu.Parent.Inactive.HotKey   = ColorPair{ Color::Gray, Color::Silver };
    this->Menu.Parent.Inactive.ShortCut = ColorPair{ Color::Gray, Color::Silver };
    this->Menu.Parent.Inactive.Check    = ColorPair{ Color::Gray, Color::Silver };
    this->Menu.Parent.Inactive.Uncheck  = ColorPair{ Color::Gray, Color::Silver };

    this->Menu.Activ.Button.Normal   = ColorPair{ Color::Blue, Color::White };
    this->Menu.Activ.Button.Inactive = ColorPair{ Color::Silver, Color::White };
    this->Menu.Activ.Button.Hover    = ColorPair{ Color::Black, Color::Aqua };
    this->Menu.Activ.Button.Pressed  = ColorPair{ Color::Red, Color::White };

    this->Menu.Parent.Button.Normal   = ColorPair{ Color::Gray, Color::Silver };
    this->Menu.Parent.Button.Inactive = ColorPair{ Color::Gray, Color::Silver };
    this->Menu.Parent.Button.Hover    = ColorPair{ Color::Gray, Color::Silver };
    this->Menu.Parent.Button.Pressed  = ColorPair{ Color::Gray, Color::Silver };

    this->MenuBar.BackgroundColor     = ColorPair{ Color::Black, Color::White };
    this->MenuBar.Normal.HotKeyColor  = ColorPair{ Color::DarkRed, Color::White };
    this->MenuBar.Normal.NameColor    = ColorPair{ Color::Black, Color::White };
    this->MenuBar.Hover.HotKeyColor   = ColorPair{ Color::DarkRed, Color::Silver };
    this->MenuBar.Hover.NameColor     = ColorPair{ Color::Black, Color::Silver };
    this->MenuBar.Pressed.HotKeyColor = ColorPair{ Color::White, Color::Magenta };
    this->MenuBar.Pressed.NameColor   = ColorPair{ Color::Yellow, Color::Magenta };

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
    this->PropertyList.Item.Text      = ColorPair{ Color::Silver, Color::Transparent };
    this->PropertyList.Item.Value     = ColorPair{ Color::Yellow, Color::Transparent };
    this->PropertyList.Item.ReadOnly  = ColorPair{ Color::Gray, Color::Transparent };
    this->PropertyList.Item.Checked   = ColorPair{ Color::Aqua, Color::Transparent };
    this->PropertyList.Item.Unchecked = ColorPair{ Color::Red, Color::Transparent };
    this->PropertyList.Item.Error     = ColorPair{ Color::Red, Color::Transparent };

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
          { Color::Black, Color::Yellow });
    this->ButtonHotKey.Set(
          { Color::Magenta, Color::White },
          { Color::DarkRed, Color::Gray },
          { Color::Gray, Color::Black },
          { Color::Magenta, Color::Yellow });

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
}
} // namespace AppCUI::Application
