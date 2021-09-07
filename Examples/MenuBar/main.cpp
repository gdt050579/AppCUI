#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;

std::string_view KeyWords[] = { "if",      "do",         "while",   "until",   "case",     "return", "local",
                                "global",  "for",        "foreach", "error",   "exit",     "repeat", "class",
                                "struct",  "union",      "auto",    "try",     "catch",    "hrow",   "finally",
                                "extends", "implements", "public",  "private", "protected" };
std::string_view Types[]    = { "bool",   "char",   "int",     "short",    "long",    "float",
                             "double", "size_t", "char8_t", "char16_t", "char32_t" };


class ContextMenuExample : public AppCUI::Controls::Window
{
    Label l1;
    Menu ctxMenu;

  public:
    ContextMenuExample()
    {
        this->Create("Context menu", "d:c,w:64,h:10");
        l1.Create(this, "Right click below to open a contextual menu", "x:1,y:1,w:62,h:2");
        ctxMenu.AddCommandItem("Command &1", 100);
        ctxMenu.AddCommandItem("Command &2", 200);
        ctxMenu.AddCommandItem("Command &3", 300);
    }
    bool OnEvent(Control*, Event eventType, int) override
    {
        if (eventType == Event::EVENT_WINDOW_CLOSE)
        {
            Application::Close();
            return true;
        }
        return false;
    }
    void OnMousePressed(int x, int y, MouseButton button) override
    {
        if ((button & MouseButton::Right) != MouseButton::None)
            ctxMenu.Show(this, x, y);
        Window::OnMousePressed(x, y, button);
    }
    bool OnUpdateCommandBar(CommandBar& cmd) override
    {
        cmd.SetCommand(Key::F1, "Do something", 1000);
        cmd.SetCommand(Key::F2, "Do something else", 1001);
        return true;
    }
};

void BuildAppMenu()
{
    Menu* files = Application::AddMenu("&File");
    files->AddCommandItem("&Save", 100, Key::Ctrl | Key::S);
    files->AddCommandItem("Save All", 101);
    files->AddCommandItem("&Open", 102, Key::Ctrl | Key::O);
    files->AddSeparator();
    files->AddCommandItem("E&xit", 103, Key::Ctrl | Key::Q);
    Menu* edit = Application::AddMenu("&Edit");
    edit->AddCommandItem("&Copy", 200, Key::Ctrl | Key::Insert);
    edit->AddCommandItem("&paste", 201, Key::Shift | Key::Insert);
    edit->AddCommandItem("&Cut", 202, Key::Shift | Key::Delete);
    edit->AddCommandItem("Special copy", 203, Key::Ctrl | Key::Alt | Key::Shift | Key::Insert);
    Menu* lang = Application::AddMenu("&Language");
    for (auto i : KeyWords)
        lang->AddRadioItem(i, 1000);
    lang->AddSeparator();
    for (auto i : Types)
        lang->AddRadioItem(i, 1000);
    Menu* help = Application::AddMenu("&Help");
    help->AddCommandItem("&About", 300);
    help->AddCommandItem("Search &online", 301);
    help->AddSeparator();
    help->AddCommandItem("Check for &updates", 302);
}
int main()
{
    if (!Application::Init(InitializationFlags::Menu | InitializationFlags::CommandBar))
        return 1;
    Application::AddWindow(new ContextMenuExample());
    BuildAppMenu();
    Application::Run();
    return 0;
}
