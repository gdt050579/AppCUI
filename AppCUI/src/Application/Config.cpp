#include "AppCUI.h"

using namespace AppCUI::Console;

void AppCUI::Application::Config::SetDarkTheme()
{
    this->Desktop.Color = COLOR(Color::Gray, Color::Black);
    this->Desktop.DesktopFillCharacterCode = 186;

    this->CommandBar.BackgroundColor = COLOR(Color::Black,Color::White);
    this->CommandBar.ShiftKeysColor = COLOR(Color::Gray, Color::White);
    this->CommandBar.Normal.KeyColor = COLOR(Color::DarkRed, Color::White);
    this->CommandBar.Normal.NameColor = COLOR(Color::Black, Color::White);
    this->CommandBar.Hover.KeyColor = COLOR(Color::DarkRed, Color::Silver);
    this->CommandBar.Hover.NameColor = COLOR(Color::Black, Color::Silver);
    this->CommandBar.Pressed.KeyColor = COLOR(Color::White, Color::Magenta);
    this->CommandBar.Pressed.NameColor = COLOR(Color::Yellow, Color::Magenta);

    this->Window.ActiveColor = COLOR(Color::White, Color::DarkBlue);
    this->Window.InactiveColor = COLOR(Color::Silver, Color::Black);
    this->Window.TitleActiveColor = COLOR(Color::Yellow, Color::DarkBlue);
    this->Window.TitleInactiveColor = COLOR(Color::Silver, Color::Black);
    this->Window.ControlButtonColor = COLOR(Color::Aqua, Color::DarkBlue);
    this->Window.ControlButtonHoverColor = COLOR(Color::Black, Color::Aqua);
    this->Window.ControlButtonPressedColor = COLOR(Color::Black, Color::Yellow);
    this->Window.ControlButtonInactiveColor = COLOR(Color::Silver, Color::Black);

    this->Label.NormalColor = COLOR(Color::Silver, Color::Transparent);
    this->Label.HotKeyColor = COLOR(Color::Yellow, Color::Transparent);

    this->Button.Normal.TextColor = COLOR(Color::Silver, Color::Gray);
    this->Button.Normal.HotKeyColor = COLOR(Color::Yellow, Color::Gray);
    this->Button.Focused.TextColor = COLOR(Color::Black, Color::White);
    this->Button.Focused.HotKeyColor = COLOR(Color::Magenta, Color::White);
    this->Button.Inactive.TextColor = COLOR(Color::Gray, Color::Black);
    this->Button.Inactive.HotKeyColor = COLOR(Color::Gray, Color::Black);
    this->Button.Hover.TextColor = COLOR(Color::Black, Color::Yellow);
    this->Button.Hover.HotKeyColor = COLOR(Color::Magenta, Color::Yellow);

    this->StateControl.Normal.TextColor = COLOR(Color::Silver, Color::Transparent);
    this->StateControl.Normal.HotKeyColor = COLOR(Color::Aqua, Color::Transparent);
    this->StateControl.Normal.StateSymbolColor = COLOR(Color::Green, Color::Transparent);
    this->StateControl.Focused.TextColor = COLOR(Color::White, Color::Transparent);
    this->StateControl.Focused.HotKeyColor = COLOR(Color::Aqua, Color::Transparent);
    this->StateControl.Focused.StateSymbolColor = COLOR(Color::Green, Color::Transparent);
    this->StateControl.Hover.TextColor = COLOR(Color::Yellow, Color::Transparent);
    this->StateControl.Hover.HotKeyColor = COLOR(Color::Aqua, Color::Transparent);
    this->StateControl.Hover.StateSymbolColor = COLOR(Color::Green, Color::Transparent);
    this->StateControl.Inactive.TextColor = COLOR(Color::Gray, Color::Transparent);
    this->StateControl.Inactive.HotKeyColor = COLOR(Color::Gray, Color::Transparent);
    this->StateControl.Inactive.StateSymbolColor = COLOR(Color::Gray, Color::Transparent);

}
