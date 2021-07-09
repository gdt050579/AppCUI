#include "AppCUI.h"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

const char * python_code = R"PYTHON(
import os;
#this code will print all files that starts with letter 'a'
for fname in os.listdir("."):
    if fname.startswith("a"):
        print("Found: "+fname)
)PYTHON";

class PythonEditorWin : public AppCUI::Controls::Window
{
    TextArea editor;
public:
    PythonEditorWin()
    {
        this->Create("Python Editor", "a:c,w:70,h:20");
        editor.Create(this, python_code, "x:0,y:0,w:100%,h:100%", TextAreaFlags::SHOW_LINE_NUMBERS);
    }
    bool OnEvent(const void* sender, Event::Type eventType, int controlID) override
    {
        if (eventType == Event::EVENT_WINDOW_CLOSE)
        {
            Application::Close();
            return true;
        }
        return false;
    }
};
int main()
{
    Application::Init();
    Application::AddWindow(new PythonEditorWin());
    Application::Run();
    return 0;
}
