#include "AppCUI.hpp"

using namespace AppCUI::Controls;
using namespace AppCUI::Dialogs;
using namespace AppCUI::Graphics;

constexpr int BUTTON_ID_GOTO   = 10000;
constexpr int BUTTON_ID_DELETE = 10001;
constexpr int BUTTON_ID_CANCEL = 10002;

class InternalWindowManager : public AppCUI::Controls::Window
{
    ListView lst;
    Button btnGoTo, btnDelete, btnCancel;

  public:
    bool Create();
};

bool InternalWindowManager::Create()
{
    CHECK(Window::Create("Window manager", "d:c,w:70,h:20"), false, "");
    CHECK(lst.Create(this, "l:1,t:1,r:1,b:3"), false, "");
    CHECK(lst.AddColumn("Window caption", TextAlignament::Left, 50), false, "");
    CHECK(lst.AddColumn("TAG", TextAlignament::Left, 20), false, "");
    CHECK(btnGoTo.Create(this, "&Goto", "l:15,b:0,w:11", BUTTON_ID_GOTO), false, "");
    CHECK(btnDelete.Create(this, "&Delete", "l:27,b:0,w:11", BUTTON_ID_DELETE), false, "");
    CHECK(btnCancel.Create(this, "&Cancel", "l:39,b:0,w:11", BUTTON_ID_CANCEL), false, "");

    return true;
}


void AppCUI::Dialogs::WindowManager::Show()
{
    InternalWindowManager dlg;
    if (dlg.Create())
    {
        dlg.Show();
    }
}