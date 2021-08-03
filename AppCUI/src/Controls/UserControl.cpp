#include "ControlContext.hpp"

using namespace AppCUI::Controls;

bool UserControl::Create(Control* parent, const AppCUI::Utils::ConstString& caption, const char* layout)
{
    CONTROL_INIT_CONTEXT(ControlContext);
    CHECK(Init(parent, caption, layout, false), false, "Failed to create user control !");
    CREATE_CONTROL_CONTEXT(this, Members, false);
    Members->Flags = GATTR_VISIBLE | GATTR_ENABLE | GATTR_TABSTOP;
    return true;
}
bool UserControl::Create(Control* parent, const char* layout)
{
    return UserControl::Create(parent, "", layout);
}