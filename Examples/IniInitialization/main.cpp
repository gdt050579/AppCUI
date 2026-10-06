#include "AppCUI.hpp"
#include <iostream>

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

/*
[AppCUI]
Frontend = default      ; possible values: default,SDL, terminal, windows
Size = default          ; possible values: a size (width x height), maximized, fullscreen
CharacterSize = default ; possible values: default, tiny, small, normal, large, huge
Fixed = false           ; possible values: true or false
*/

class SimpleWin : public AppCUI::Controls::Window
{
  public:
    SimpleWin() : Window("Test", "d:c,w:40,h:10", WindowFlags::None)
    {
        Factory::Label::Create(
              this,
              "This is a simple Window example that creates a 40x20 fixed window that  will close if you click on the "
              "[X]  button from the bottom-top corner.",
              "x:1,y:1,w:36,h:5");
    }
};
int main()
{
    Log::ToFile("IniInitialization.log");
    LOG_INFO("Settings file: %s", AppCUI::Application::GetAppSettingsFile().c_str());
    if (!Application::Init(InitializationFlags::LoadSettingsFile))
        return 1;
    auto ini = Application::GetAppSettings();
    CHECK(ini, false, "Application::GetAppSettings() returned a null object ");

    if (ini->HasSection("AppCUI") == false)
    {
        Application::UpdateAppCUISettings(true);
        CHECK(Application::SaveAppSettings(), 1, "Fail to save application settings !");
        LOG_INFO("IniInitialization.ini created successfully. Modify it and re-run this executable");
        return 0;
    }
    Application::AddWindow(std::make_unique<SimpleWin>());
    Application::Run();
    return 0;
}
