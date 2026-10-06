// Samples of docs/source/initialization.rst - compiled with the examples (docs/snippets/CMakeLists.txt).
#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;

bool QuickInitialization()
{
    // [quick]
    if (!Application::Init())
    {
        // AppCUI failed to initialize
        return false;
    }
    // [/quick]
    return true;
}

bool InitializationFromIni()
{
    // [ini]
    if (!Application::Init(InitializationFlags::LoadSettingsFile))
    {
        // AppCUI failed to initialize
        return false;
    }
    // [/ini]
    return true;
}

bool InitializationWithMenuAndCommandBar()
{
    // [ini-menu]
    if (!Application::Init(InitializationFlags::LoadSettingsFile | InitializationFlags::Menu | InitializationFlags::CommandBar))
    {
        // AppCUI failed to initialize
        return false;
    }
    // [/ini-menu]
    return true;
}

bool CustomInitialization()
{
    // [custom]
    InitializationData initData;
    initData.Width               = 120;
    initData.Height              = 30;
    initData.Frontend            = FrontendType::SDL;
    initData.CharSize            = CharacterSize::Small;
    initData.Theme               = ThemeType::Dark;
    initData.SpecialCharacterSet = SpecialCharacterSetType::Unicode;
    initData.Flags               = InitializationFlags::Menu | InitializationFlags::CommandBar;

    if (!Application::Init(initData))
    {
        // AppCUI failed to initialize
        return false;
    }
    // [/custom]
    return true;
}

int main(int argc, char** argv)
{
    // the sample to run is selected by the first argument (quick, ini, ini-menu, custom)
    const std::string_view mode = argc > 1 ? argv[1] : "quick";
    bool ok                     = false;
    if (mode == "ini")
        ok = InitializationFromIni();
    else if (mode == "ini-menu")
        ok = InitializationWithMenuAndCommandBar();
    else if (mode == "custom")
        ok = CustomInitialization();
    else
        ok = QuickInitialization();
    if (!ok)
        return 1;
    auto window = Controls::Factory::Window::Create("Initialization", "d:c,w:40,h:6");
    Application::AddWindow(std::move(window));
    Application::Run();
    return 0;
}
