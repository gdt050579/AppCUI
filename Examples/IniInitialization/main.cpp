#include "AppCUI.hpp"
#include <iostream>

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

const char* ini = R"INI(
[AppCUI]
Frontend = default      ; possible values: default,SDL, terminal, windows
Size = default          ; possible values: a size (width x height), maximized, fullscreen
CharacterSize = default ; possible values: default, tiny, small, normal, large, huge
Fixed = false           ; possible values: true or false
)INI";

class SimpleWin : public AppCUI::Controls::Window
{
    Label l1;

  public:
    SimpleWin()
    {
        this->Create("Test", "d:c,w:40,h:10");
        l1.Create(
              this,
              "This is a simple Window example that creates a 40x20 fixed window that  will close if you click on the "
              "[X]  button from the bottom-top corner.",
              "x:1,y:1,w:36,h:5");
    }
    bool OnEvent(Control*, Event eventType, int) override
    {
        if (eventType == Event::WindowClose)
        {
            Application::Close();
            return true;
        }
        return false;
    }
};
int main()
{
    Log::ToFile("IniInitialization.log");
    if (std::filesystem::exists("IniInitialization.ini") == false)
    {
        LOG_INFO("IniInitialization.ini is missing (a new one will be created)");
        AppCUI::OS::File f;
        CHECK(f.Create("IniInitialization.ini"), 1, "Fail to create IniInitialization.ini");
        CHECK(f.Write(ini, Utils::String::Len(ini)), false, "Fail to write data to: IniInitialization.ini");
        f.Close();
        LOG_INFO("IniInitialization.ini created succesifully. Modify it and re-run this executable");
        return 0;
    }
    Application::InitializationData initData;
    initData.Flags = Application::InitializationFlags::LoadSettingsFile;
    if (!Application::Init(initData))
        return 1;
    Application::AddWindow(std::make_unique<SimpleWin>());
    Application::Run();
    return 0;
}
