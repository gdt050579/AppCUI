#include "../../include/Internal.h"
#include "../../include/ControlContext.h"

using namespace AppCUI::Internal;
using namespace AppCUI::Console;
using namespace AppCUI::Input;

bool DesktopControl::Create(unsigned int _width, unsigned int _height)
{
	CONTROL_INIT_CONTEXT(ControlContext);
    AppCUI::Utils::LocalString<128> temp;
    
	CHECK(Init(nullptr, "", temp.Format("x:0,y:0,w:%d,h:%d", _width, _height), false), false, "Failed to create desktop !");
	CREATE_CONTROL_CONTEXT(this, Members, false);
    Members->ScreenClip.Set(0, 0, _width, _height);
	Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
	return true;
}
void DesktopControl::Paint(AppCUI::Console::Renderer & renderer)
{
    CREATE_TYPECONTROL_CONTEXT(ControlContext, Members, );
    renderer.Clear(Members->Cfg->Desktop.DesktopFillCharacterCode, Members->Cfg->Desktop.Color);
    char temp[2] = { ' ', 0 };
    for (int tr = 160; tr < 200; tr++) {
        temp[0] = tr;
        renderer.WriteSingleLineText(tr - 160, 0, temp, 15);
    }
    renderer.FillHorizontalLine(5, 10, 40, '=', 14);
    renderer.FillVerticalLine(5, 10, 20, '|', 14);

    renderer.FillRectWidthHeight(60, 10, 5, 1, 'M', 12);
}
bool DesktopControl::OnKeyEvent(AppCUI::Input::Key::Type keyCode, char AsciiCode)
{
    if (keyCode == Key::Escape)
    {
        AppCUI::Application::Close();
        return true;
    }
    return false;
}
