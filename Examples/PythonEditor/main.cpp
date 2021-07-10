#include "AppCUI.h"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

const char * python_code = R"PYTHON(
import os;
d = {}
#this code will count how many files that starts with a specific letter exists
for fname in os.listdir("."):
    fname = fname.lower()
    if not fname[0] in d:
        d[fname[0]] = [fname]
    else:
        d[fname[0]] += [fname]

#show statistics
for letter in d:
    print("Letter: "+letter+" => "+str(len(d[letter]))
    index = 1
    for fname in d[letter]:
        print("%3d => %s"%(index,fname))
        index += 1

#other statistics
total = 0
for letter in d:
    total += len(d[letter])
print("Total files: "+str(total))
)PYTHON";

class PythonEditorWin : public AppCUI::Controls::Window
{
    TextArea editor;
public:
    PythonEditorWin()
    {
        this->Create("Python Editor", "a:c,w:40,h:20");
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
