#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Dialogs;

class MyWin : public AppCUI::Controls::Window
{
    enum class ControlIds : unsigned int
    {
        ButtonShowOpen = 1
    };

    Button open;
    TextField currentFolder;

  public:
    MyWin()
    {
        Create("File example", "d:c, w:120, h:20");
        open.Create(this, "&Open", "x:0, y:0, w:12", static_cast<unsigned int>(ControlIds::ButtonShowOpen));
        currentFolder.Create(this, "", "x:13, y:0, h:1, w:100%");

        currentFolder.SetText(std::filesystem::current_path().u8string());
    }

    bool OnEvent(Control*, Event eventType, int controlID) override
    {
        switch (eventType)
        {
        case Event::WindowClose:
            Application::Close();
            return true;
        case Event::ButtonClicked:
            switch (static_cast<ControlIds>(controlID))
            {
            case ControlIds::ButtonShowOpen:
            {
                const auto res = FolderDialog::ShowOpenFileWindow("", currentFolder.GetText());
                if (res.has_value())
                {
                    currentFolder.SetText(res->u8string());
                }

                return true;
            }
            }
        }

        return false;
    }
};

int main()
{
    if (Application::Init() == false)
    {
        return 1;
    }

    Application::AddWindow(new MyWin());
    Application::Run();

    return 0;
}