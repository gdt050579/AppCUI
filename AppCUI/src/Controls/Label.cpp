#include "../../include/ControlContext.h"


using namespace AppCUI::Controls;
using namespace AppCUI::Console;


bool Label::Create(Control *parent, const char * text, const char * layout)
{
	CONTROL_INIT_CONTEXT(ControlContext);
    CREATE_CONTROL_CONTEXT(this, Members, false);
    Members->Layout.MinHeight = 1;
    Members->Layout.MinWidth = 1;
	CHECK(Init(parent, text, layout,  false), false, "Failed to create panel !");
	
	Members->Flags = GATTR_ENABLE | GATTR_VISIBLE ;
	return true;
}

void Label::Paint(Console::Renderer & renderer)
{
	CREATE_CONTROL_CONTEXT(this, Members, );
    WriteCharacterBufferParams params(WriteCharacterBufferFlags::OVERWRITE_COLORS);
    params.Color = Members->Cfg->Label.NormalColor;
    params.HotKeyColor = Members->Cfg->Label.HotKeyColor;
    if (Members->Layout.Height == 1) {
        params.Flags |= WriteCharacterBufferFlags::SINGLE_LINE;
    } else {
        params.Flags |= WriteCharacterBufferFlags::MULTIPLE_LINES;
    }
    renderer.WriteCharacterBuffer(0, 0, Members->Text, params);
}