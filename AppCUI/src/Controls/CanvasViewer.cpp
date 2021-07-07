#include "../../include/ControlContext.h"


using namespace AppCUI::Controls;
using namespace AppCUI::Console;
using namespace AppCUI::Input;

bool CanvasViewer::Create(Control* parent, const char * layout, unsigned int canvasWidth, unsigned int canvasHeight, ViewerFlags::Type flags)
{
    return this->Create(parent, "", layout, canvasWidth, canvasHeight, flags);
}
bool CanvasViewer::Create(Control* parent, const char * title, const char * layout, unsigned int canvasWidth, unsigned int canvasHeight, ViewerFlags::Type flags)
{
    CHECK(canvasWidth > 0, false, "Canvas Width must be greater than 0.");
    CHECK(canvasHeight > 0, false, "Canvas Height must be greater than 0.");
    CONTROL_INIT_CONTEXT(CanvasControlContext);
    CHECK(Init(parent, title, layout, true), false, "Failed to create Canvas viewer object");
    CREATE_TYPECONTROL_CONTEXT(CanvasControlContext, Members, false);
    Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | GATTR_VSCROLL| GATTR_HSCROLL | flags;
    Members->CanvasScrollX = 0;
    Members->CanvasScrollY = 0;
    CHECK(Members->canvas.Create(canvasWidth, canvasHeight), false, "Fail to create a canvas of size %d x %d", canvasWidth, canvasHeight);
    return true;
}



CanvasViewer::~CanvasViewer() {
	DELETE_CONTROL_CONTEXT(CanvasControlContext);
}

void CanvasViewer::Paint(Console::Renderer & renderer)
{
	CREATE_TYPECONTROL_CONTEXT(CanvasControlContext, Members, ); 
    auto * col = &Members->Cfg->View.Normal;
    if (!this->IsEnabled())
        col = &Members->Cfg->View.Inactive;
    else if (Members->Focused)
        col = &Members->Cfg->View.Focused;
    else if (Members->MouseIsOver)
        col = &Members->Cfg->View.Hover;

    if (Members->Flags & ViewerFlags::BORDER)
    {
        renderer.DrawRectSize(0, 0, Members->Layout.Width, Members->Layout.Height, col->Border, false);
        if (Members->Layout.Width > 6) {
            renderer.WriteCharacterBuffer(3, 0, Members->Layout.Width - 6, Members->Text, col->Text, col->Hotkey, Members->HotKeyOffset, TextAlignament::Left | TextAlignament::Padding);
        }
        if (!renderer.SetClipMargins(1, 1, 1, 1))
            return; // clipping is not visible --> no need to try to draw the rest
    }
    if (!this->IsEnabled())
        renderer.DrawCanvas(Members->CanvasScrollX, Members->CanvasScrollY, Members->canvas, Members->Cfg->View.InactiveCanvasColor);
    else
        renderer.DrawCanvas(Members->CanvasScrollX, Members->CanvasScrollY, Members->canvas);

}


bool CanvasViewer::OnKeyEvent(AppCUI::Input::Key::Type KeyCode, char AsciiCode) {
    CREATE_TYPECONTROL_CONTEXT(CanvasControlContext, Members, false);

    switch (KeyCode) {
    case Key::Down:
        Members->CanvasScrollY--; 
        if (Members->CanvasScrollY < -((int)(Members->canvas.GetHeight())))
            Members->CanvasScrollY = -((int)(Members->canvas.GetHeight()));
        return true;
    case Key::Up:
        Members->CanvasScrollY = MINVALUE(Members->CanvasScrollY + 1, 0);
        return true;
    case Key::Right:
        Members->CanvasScrollX--;
        if (Members->CanvasScrollX < -((int)(Members->canvas.GetWidth())))
            Members->CanvasScrollX = -((int)(Members->canvas.GetWidth()));
        return true;
    case Key::Left:
        Members->CanvasScrollX = MINVALUE(Members->CanvasScrollX + 1, 0);
        return true;
    default:
        break;
    }
    return false;
}
bool AppCUI::Controls::CanvasViewer::OnMouseEnter()
{
    return true;
}
bool AppCUI::Controls::CanvasViewer::OnMouseLeave()
{
    return true;
}

Canvas*	CanvasViewer::GetCanvas()
{
	CREATE_TYPECONTROL_CONTEXT(CanvasControlContext, Members, nullptr);
	return &(Members->canvas);
}
