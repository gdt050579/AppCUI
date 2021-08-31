#include "AppCUI.hpp"

using namespace AppCUI::Graphics;

void AppCUI::Application::Config::SetDarkTheme()
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

    this->Window.ActiveColor                = ColorPair{ Color::White, Color::DarkBlue };
    this->Window.InactiveColor              = ColorPair{ Color::Silver, Color::Black };
    this->Window.TitleActiveColor           = ColorPair{ Color::Yellow, Color::DarkBlue };
    this->Window.TitleInactiveColor         = ColorPair{ Color::Silver, Color::Black };
    this->Window.ControlButtonColor         = ColorPair{ Color::Aqua, Color::DarkBlue };
    this->Window.ControlButtonHoverColor    = ColorPair{ Color::Black, Color::Aqua };
    this->Window.ControlButtonPressedColor  = ColorPair{ Color::Black, Color::Yellow };
    this->Window.ControlButtonInactiveColor = ColorPair{ Color::Silver, Color::Black };

    this->DialogError.ActiveColor                = ColorPair{ Color::White, Color::DarkRed };
    this->DialogError.InactiveColor              = ColorPair{ Color::Silver, Color::DarkRed };
    this->DialogError.TitleActiveColor           = ColorPair{ Color::Yellow, Color::DarkRed };
    this->DialogError.TitleInactiveColor         = ColorPair{ Color::Silver, Color::DarkRed };
    this->DialogError.ControlButtonColor         = ColorPair{ Color::Aqua, Color::DarkRed };
    this->DialogError.ControlButtonHoverColor    = ColorPair{ Color::Black, Color::Aqua };
    this->DialogError.ControlButtonPressedColor  = ColorPair{ Color::Black, Color::Yellow };
    this->DialogError.ControlButtonInactiveColor = ColorPair{ Color::Silver, Color::Black };

    this->DialogNotify.ActiveColor                = ColorPair{ Color::White, Color::DarkGreen };
    this->DialogNotify.InactiveColor              = ColorPair{ Color::Silver, Color::DarkGreen };
    this->DialogNotify.TitleActiveColor           = ColorPair{ Color::Yellow, Color::DarkGreen };
    this->DialogNotify.TitleInactiveColor         = ColorPair{ Color::Silver, Color::DarkGreen };
    this->DialogNotify.ControlButtonColor         = ColorPair{ Color::Aqua, Color::DarkGreen };
    this->DialogNotify.ControlButtonHoverColor    = ColorPair{ Color::Black, Color::Aqua };
    this->DialogNotify.ControlButtonPressedColor  = ColorPair{ Color::Black, Color::Yellow };
    this->DialogNotify.ControlButtonInactiveColor = ColorPair{ Color::Silver, Color::Black };

    this->DialogWarning.ActiveColor                = ColorPair{ Color::White, Color::Olive };
    this->DialogWarning.InactiveColor              = ColorPair{ Color::Silver, Color::Olive };
    this->DialogWarning.TitleActiveColor           = ColorPair{ Color::Yellow, Color::Olive };
    this->DialogWarning.TitleInactiveColor         = ColorPair{ Color::Silver, Color::Olive };
    this->DialogWarning.ControlButtonColor         = ColorPair{ Color::Aqua, Color::Olive };
    this->DialogWarning.ControlButtonHoverColor    = ColorPair{ Color::Black, Color::Aqua };
    this->DialogWarning.ControlButtonPressedColor  = ColorPair{ Color::Black, Color::Yellow };
    this->DialogWarning.ControlButtonInactiveColor = ColorPair{ Color::Silver, Color::Black };

    this->Label.NormalColor = ColorPair{ Color::Silver, Color::Transparent };
    this->Label.HotKeyColor = ColorPair{ Color::Yellow, Color::Transparent };

    this->Button.Normal.TextColor     = ColorPair{ Color::White, Color::Gray };
    this->Button.Normal.HotKeyColor   = ColorPair{ Color::Yellow, Color::Gray };
    this->Button.Focused.TextColor    = ColorPair{ Color::Black, Color::White };
    this->Button.Focused.HotKeyColor  = ColorPair{ Color::Magenta, Color::White };
    this->Button.Inactive.TextColor   = ColorPair{ Color::Gray, Color::Black };
    this->Button.Inactive.HotKeyColor = ColorPair{ Color::Gray, Color::Black };
    this->Button.Hover.TextColor      = ColorPair{ Color::Black, Color::Yellow };
    this->Button.Hover.HotKeyColor    = ColorPair{ Color::Magenta, Color::Yellow };

    this->StateControl.Normal.TextColor          = ColorPair{ Color::Silver, Color::Transparent };
    this->StateControl.Normal.HotKeyColor        = ColorPair{ Color::Aqua, Color::Transparent };
    this->StateControl.Normal.StateSymbolColor   = ColorPair{ Color::Green, Color::Transparent };
    this->StateControl.Focused.TextColor         = ColorPair{ Color::White, Color::Transparent };
    this->StateControl.Focused.HotKeyColor       = ColorPair{ Color::Aqua, Color::Transparent };
    this->StateControl.Focused.StateSymbolColor  = ColorPair{ Color::Green, Color::Transparent };
    this->StateControl.Hover.TextColor           = ColorPair{ Color::Yellow, Color::Transparent };
    this->StateControl.Hover.HotKeyColor         = ColorPair{ Color::Aqua, Color::Transparent };
    this->StateControl.Hover.StateSymbolColor    = ColorPair{ Color::Green, Color::Transparent };
    this->StateControl.Inactive.TextColor        = ColorPair{ Color::Gray, Color::Transparent };
    this->StateControl.Inactive.HotKeyColor      = ColorPair{ Color::Gray, Color::Transparent };
    this->StateControl.Inactive.StateSymbolColor = ColorPair{ Color::Gray, Color::Transparent };

    this->Splitter.NormalColor = ColorPair{ Color::Silver, Color::Transparent };
    this->Splitter.ClickColor  = ColorPair{ Color::White, Color::Magenta };
    this->Splitter.HoverColor  = ColorPair{ Color::Yellow, Color::Transparent };

    this->Panel.NormalColor = ColorPair{ Color::Silver, Color::Transparent };
    this->Panel.TextColor   = ColorPair{ Color::White, Color::Transparent };

    this->Text.SelectionColor             = ColorPair{ Color::Yellow, Color::Magenta };
    this->Text.Normal.Text                = ColorPair{ Color::Silver, Color::Black };
    this->Text.Normal.Border              = ColorPair{ Color::Silver, Color::Transparent };
    this->Text.Normal.LineNumbers         = ColorPair{ Color::Gray, Color::Black };
    this->Text.Normal.CurrentLineNumber   = ColorPair{ Color::Silver, Color::Black };
    this->Text.Hover.Text                 = ColorPair{ Color::Yellow, Color::Black };
    this->Text.Hover.Border               = ColorPair{ Color::Yellow, Color::Transparent };
    this->Text.Hover.LineNumbers          = ColorPair{ Color::Gray, Color::Black };
    this->Text.Hover.CurrentLineNumber    = ColorPair{ Color::Silver, Color::Black };
    this->Text.Inactive.Text              = ColorPair{ Color::Gray, Color::Transparent };
    this->Text.Inactive.Border            = ColorPair{ Color::Gray, Color::Transparent };
    this->Text.Inactive.LineNumbers       = ColorPair{ Color::Gray, Color::Transparent };
    this->Text.Inactive.CurrentLineNumber = ColorPair{ Color::Gray, Color::Transparent };
    this->Text.Focus.Text                 = ColorPair{ Color::White, Color::Black };
    this->Text.Focus.Border               = ColorPair{ Color::Gray, Color::Transparent };
    this->Text.Focus.LineNumbers          = ColorPair{ Color::Silver, Color::Teal };
    this->Text.Focus.CurrentLineNumber    = ColorPair{ Color::Yellow, Color::Magenta };

    this->Tab.PageColor              = ColorPair{ Color::White, Color::Blue };
    this->Tab.PageHotKeyColor        = ColorPair{ Color::Yellow, Color::Blue };
    this->Tab.TabBarColor            = ColorPair{ Color::Silver, Color::Gray };
    this->Tab.TabBarHotKeyColor      = ColorPair{ Color::White, Color::Gray };
    this->Tab.HoverColor             = ColorPair{ Color::Yellow, Color::Magenta };
    this->Tab.HoverHotKeyColor       = ColorPair{ Color::White, Color::Magenta };
    this->Tab.ListSelectedPageColor  = ColorPair{ Color::Black, Color::White };
    this->Tab.ListSelectedPageHotKey = ColorPair{ Color::DarkRed, Color::White };

    this->View.Normal.Border       = ColorPair{ Color::Silver, Color::Transparent };
    this->View.Normal.Hotkey       = ColorPair{ Color::Yellow, Color::Transparent };
    this->View.Normal.Text         = ColorPair{ Color::Silver, Color::Transparent };
    this->View.Focused.Border      = ColorPair{ Color::White, Color::Transparent };
    this->View.Focused.Hotkey      = ColorPair{ Color::Yellow, Color::Transparent };
    this->View.Focused.Text        = ColorPair{ Color::Yellow, Color::Transparent };
    this->View.Hover.Border        = ColorPair{ Color::Yellow, Color::Transparent };
    this->View.Hover.Hotkey        = ColorPair{ Color::Red, Color::Transparent };
    this->View.Hover.Text          = ColorPair{ Color::White, Color::Transparent };
    this->View.Inactive.Border     = ColorPair{ Color::Gray, Color::Transparent };
    this->View.Inactive.Hotkey     = ColorPair{ Color::Gray, Color::Transparent };
    this->View.Inactive.Text       = ColorPair{ Color::Gray, Color::Transparent };
    this->View.InactiveCanvasColor = ColorPair{ Color::Black, Color::Transparent };

    this->ScrollBar.Arrows   = ColorPair{ Color::White, Color::Teal };
    this->ScrollBar.Bar      = ColorPair{ Color::Black, Color::Teal };
    this->ScrollBar.Position = ColorPair{ Color::Green, Color::Teal };

    this->ListView.Normal.Border          = ColorPair{ Color::Silver, Color::Transparent };
    this->ListView.Normal.LineSparators   = ColorPair{ Color::Silver, Color::Transparent };
    this->ListView.Inactive.Border        = ColorPair{ Color::Gray, Color::Transparent };
    this->ListView.Inactive.LineSparators = ColorPair{ Color::Gray, Color::Transparent };
    this->ListView.Hover.Border           = ColorPair{ Color::Yellow, Color::Transparent };
    this->ListView.Hover.LineSparators    = ColorPair{ Color::Silver, Color::Transparent };
    this->ListView.Focused.Border         = ColorPair{ Color::White, Color::Transparent };
    this->ListView.Focused.LineSparators  = ColorPair{ Color::Silver, Color::Transparent };

    this->ListView.ColumnNormal.Text        = ColorPair{ Color::Silver, Color::Magenta };
    this->ListView.ColumnNormal.HotKey      = ColorPair{ Color::Yellow, Color::Transparent };
    this->ListView.ColumnNormal.Separator   = ColorPair{ Color::Gray, Color::Transparent };
    this->ListView.ColumnInactive.Text      = ColorPair{ Color::Gray, Color::Transparent };
    this->ListView.ColumnInactive.HotKey    = ColorPair{ Color::Gray, Color::Transparent };
    this->ListView.ColumnInactive.Separator = ColorPair{ Color::Gray, Color::Transparent };
    this->ListView.ColumnHover.Text         = ColorPair{ Color::Yellow, Color::Magenta };
    this->ListView.ColumnHover.HotKey       = ColorPair{ Color::Yellow, Color::Magenta };
    this->ListView.ColumnHover.Separator    = ColorPair{ Color::Yellow, Color::Magenta };
    this->ListView.ColumnSort.Text          = ColorPair{ Color::White, Color::Pink };
    this->ListView.ColumnSort.HotKey        = ColorPair{ Color::Black, Color::Pink };
    this->ListView.ColumnSort.Separator     = ColorPair{ Color::Gray, Color::Pink };

    this->ListView.Item.Regular          = ColorPair{ Color::Silver, Color::Transparent };
    this->ListView.Item.Highligheted     = ColorPair{ Color::White, Color::Transparent };
    this->ListView.Item.Inactive         = ColorPair{ Color::Gray, Color::Transparent };
    this->ListView.Item.Error            = ColorPair{ Color::Red, Color::Transparent };
    this->ListView.Item.Warning          = ColorPair{ Color::Olive, Color::Transparent };
    this->ListView.CheckedSymbol         = ColorPair{ Color::Green, Color::Transparent };
    this->ListView.UncheckedSymbol       = ColorPair{ Color::DarkRed, Color::Transparent };
    this->ListView.InactiveColor         = ColorPair{ Color::Gray, Color::Transparent };
    this->ListView.FocusColor            = ColorPair{ Color::Black, Color::White };
    this->ListView.FocusAndSelectedColor = ColorPair{ Color::Red, Color::White };
    this->ListView.SelectionColor        = ColorPair{ Color::Yellow, Color::Transparent };
    this->ListView.FilterText            = ColorPair{ Color::White, Color::DarkRed };
    this->ListView.StatusColor           = ColorPair{ Color::Yellow, Color::Transparent };

    this->ProgressStatus.Border           = ColorPair{ Color::White, Color::DarkBlue };
    this->ProgressStatus.Title            = ColorPair{ Color::Yellow, Color::DarkBlue };
    this->ProgressStatus.Text             = ColorPair{ Color::Silver, Color::DarkBlue };
    this->ProgressStatus.TerminateMessage = ColorPair{ Color::Gray, Color::DarkBlue };
    this->ProgressStatus.EmptyProgressBar = ColorPair{ Color::White, Color::Black };
    this->ProgressStatus.FullProgressBar  = ColorPair{ Color::White, Color::Teal };
    this->ProgressStatus.Time             = ColorPair{ Color::Aqua, Color::DarkBlue };
    this->ProgressStatus.Percentage       = ColorPair{ Color::Yellow, Color::DarkBlue };

    this->ComboBox.Focus.Text      = ColorPair{ Color::Black, Color::White };
    this->ComboBox.Focus.Button    = ColorPair{ Color::DarkRed, Color::White };
    this->ComboBox.Normal.Text     = ColorPair{ Color::Black, Color::Silver };
    this->ComboBox.Normal.Button   = ColorPair{ Color::DarkRed, Color::Silver };
    this->ComboBox.Inactive.Text   = ColorPair{ Color::Gray, Color::Transparent };
    this->ComboBox.Inactive.Button = ColorPair{ Color::Gray, Color::Transparent };
    this->ComboBox.Hover.Text      = ColorPair{ Color::Black, Color::Yellow };
    this->ComboBox.Hover.Button    = ColorPair{ Color::DarkRed, Color::Yellow };
    this->ComboBox.Selection       = ColorPair{ Color::White, Color::DarkBlue };
    this->ComboBox.HoverOveItem    = ColorPair{ Color::DarkRed, Color::Silver };

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

    this->NumericSelector.Normal.TextColor     = ColorPair{ Color::Black, Color::Gray };
    this->NumericSelector.Focused.TextColor    = ColorPair{ Color::Black, Color::White };
    this->NumericSelector.Inactive.TextColor   = ColorPair{ Color::Gray, Color::Black };
    this->NumericSelector.Hover.TextColor      = ColorPair{ Color::Black, Color::Yellow };
    this->NumericSelector.WrongValue.TextColor = ColorPair{ Color::Black, Color::Red };

    this->ToolTip.Arrow = ColorPair{ Color::Green, Color::Black };
    this->ToolTip.Text = ColorPair{ Color::Black, Color::Aqua };
}
