#include "../../include/ControlContext.h"


using namespace AppCUI::Controls;
using namespace AppCUI::Console;


bool Label::Create(Control *parent, const char * text, const char * layout)
{
	CONTROL_INIT_CONTEXT(ControlContext);
    CREATE_CONTROL_CONTEXT(this, Members, false);
    Members->Layout.MinHeight = 1;
    Members->Layout.MinWidth = 1;
	CHECK(Init(parent, text, layout,  true), false, "Failed to create label !");
    Members->HotKey = AppCUI::Input::Key::None; // A label can draw a hot key, but does not have an associated one
	Members->Flags = GATTR_ENABLE | GATTR_VISIBLE ;
	return true;
}

void Label::Paint(Console::Renderer & renderer)
{
	CREATE_CONTROL_CONTEXT(this, Members, );
    WriteCharacterBufferParams params(WriteCharacterBufferFlags::OVERWRITE_COLORS | WriteCharacterBufferFlags::HIGHLIGHT_HOTKEY);
    params.Color = Members->Cfg->Label.NormalColor;
    params.HotKeyColor = Members->Cfg->Label.HotKeyColor;
    params.HotKeyPosition = Members->HotKeyOffset;
    if (Members->Layout.Height == 1) {
        params.Flags |= WriteCharacterBufferFlags::SINGLE_LINE;
    } else {
        params.Flags |= WriteCharacterBufferFlags::MULTIPLE_LINES | WriteCharacterBufferFlags::WRAP_TO_WIDTH;
        params.Width = Members->Layout.Width;
    }
    renderer.WriteCharacterBuffer(0, 0, Members->Text, params);
}