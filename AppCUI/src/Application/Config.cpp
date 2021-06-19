#include "../../include/AppCUI.h"

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
    this->CommandBar.Hover.NameColor = COLOR(Color::DarkRed, Color::Silver);
    this->CommandBar.Pressed.KeyColor = COLOR(Color::Black, Color::Magenta);
    this->CommandBar.Pressed.NameColor = COLOR(Color::Yellow, Color::Magenta);



}
