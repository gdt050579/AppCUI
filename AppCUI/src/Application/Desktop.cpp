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
    renderer.Clear(0x2591, Members->Cfg->Desktop.Color);

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
