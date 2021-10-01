#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Dialogs;

#define BUTTON_ID_SHOW_SAVE 1
#define BUTTON_ID_SHOW_OPEN 2

class MyWin : public AppCUI::Controls::Window
{
    Button b1, b2;
    TextField l1, l2;

  public:
    MyWin()
    {
        this->Create("File example", "d:c,w:40,h:13");
        b1.Create(this, "&Save", "x:1,y:1,w:9", BUTTON_ID_SHOW_SAVE);
        l1.Create(this, "", "x:12,y:1,w:24,h:4");
        b2.Create(this, "&Open", "x:1,y:6,w:9", BUTTON_ID_SHOW_OPEN);
        l2.Create(this, "", "x:12,y:6,w:24,h:4");
    }
    bool OnEvent(Control*, Event eventType, int controlID) override
    {
        if (eventType == Event::WindowClose)
        {
            Application::Close();
            return true;
        }
        if (eventType == Event::ButtonClicked)
        {
            if (controlID == BUTTON_ID_SHOW_SAVE)
            {
                auto res = FileDialog::ShowSaveFileWindow(
                      "", "Text Files:txt,Images:[jpg,jpeg,png],Documents:[pdf,doc,docx,xlsx,xls,ppt,pptx]", ".");
                if (res.has_value())
                    l1.SetText(res->u8string());
                else
                    l1.SetText("Command canceled !");
            }
            if (controlID == BUTTON_ID_SHOW_OPEN)
            {
                auto res = FileDialog::ShowOpenFileWindow(
                      "", "Text Files:txt,Images:[jpg,jpeg,png],Documents:[pdf,doc,docx,xlsx,xls,ppt,pptx]", ".");
                if (res.has_value())
                    l2.SetText(res->u8string());
                else
                    l2.SetText("Command canceled !");
            }
            return true;
        }
        return false;
    }
};
int main()
{
    if (!Application::Init(InitializationFlags::Maximized))
        return 1;
    Application::AddWindow(std::make_unique<MyWin>());
    Application::Run();
    return 0;
}
