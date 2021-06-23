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
    if (Members->Layout.Height == 1)
        renderer.WriteSingleLineTextWithHotKey(0, 0, Members->Text, Members->Cfg->Label.NormalColor, Members->Cfg->Label.HotKeyColor, Members->Text.Len());
    else
        renderer.WriteMultiLineTextWithHotKey(0, 0, Members->Text, Members->Cfg->Label.NormalColor, Members->Cfg->Label.HotKeyColor, Members->Text.Len());
}