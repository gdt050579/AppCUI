#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;

int main()
{
    if (!Application::Init(InitializationFlags::HAS_MENU))
        return 1;
    Menu* files = Application::AddMenu("&File");
    files->AddCommandItem("&Save", 100, Key::Ctrl | Key::S);
    files->AddCommandItem("Save All", 101);
    files->AddCommandItem("&Open", 102, Key::Ctrl | Key::O);
    files->AddSeparator();
    files->AddCommandItem("E&xit", 103, Key::Ctrl | Key::Q);
    Menu* edit  = Application::AddMenu("&Edit");
    edit->AddCommandItem("&Copy", 200, Key::Ctrl | Key::Insert);
    edit->AddCommandItem("&paste", 201, Key::Shift | Key::Insert);
    edit->AddCommandItem("&Cut", 202, Key::Shift | Key::Delete);
    Menu* help = Application::AddMenu("&Help");
    help->AddCommandItem("&About", 300);
    help->AddCommandItem("Search &online", 301);
    help->AddSeparator();
    help->AddCommandItem("Check for &updates", 302);
    Application::Run();
    return 0;
}
