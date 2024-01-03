#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;

std::string_view KeyWords[] = { "if",      "do",         "while",   "until",   "case",     "return", "local",
                                "global",  "for",        "foreach", "error",   "exit",     "repeat", "class",
                                "struct",  "union",      "auto",    "try",     "catch",    "hrow",   "finally",
                                "extends", "implements", "public",  "private", "protected" };
std::string_view Types[]    = { "bool",   "char",   "int",     "short",  "long",    "float",
                             "double", "size_t", "char8_t", "char16", "char32_t" };

constexpr int MENU_CMD_SAVE         = 100;
constexpr int MENU_CMD_SAVE_ALL     = 101;
constexpr int MENU_CMD_OPEN         = 102;
constexpr int MENU_CMD_EXIT         = 103;
constexpr int MENU_CMD_COPY         = 200;
constexpr int MENU_CMD_PASTE        = 201;
constexpr int MENU_CMD_CUT          = 202;
constexpr int MENU_CMD_SPECIAL_COPY = 203;
constexpr int MENU_CMD_ABOUT        = 300;
constexpr int MENU_CMD_SEARCH       = 301;
constexpr int MENU_CMD_UPDATE       = 302;

struct
{
    int commandID;
    string_view text;
} Commands[] = { { MENU_CMD_SAVE, "Save somethig" },       { MENU_CMD_SAVE_ALL, " Save all" },
                 { MENU_CMD_OPEN, "Open a new file" },     { MENU_CMD_EXIT, "Exit current application" },
                 { MENU_CMD_COPY, "Copy to clipboard" },   { MENU_CMD_PASTE, "Past from clipboard" },
                 { MENU_CMD_CUT, "Copy and then delete" }, { MENU_CMD_SPECIAL_COPY, "A special kind of copy" },
                 { MENU_CMD_ABOUT, "About this app" },     { MENU_CMD_SEARCH, "Search something online" },
                 { MENU_CMD_UPDATE, "Check for updates" } };

class ContextMenuExample : public Window
{
    Menu ctxMenu;
    Reference<Label> status;

  public:
    ContextMenuExample() : Window("Context menu", "d:c,w:64,h:10", WindowFlags::None)
    {
        Factory::Label::Create(this, "Right click below to open a contextual menu", "x:1,y:1,w:62,h:2");
        status = Factory::Label::Create(this, "", "x:1,y:3,w:62,h:1");
        ctxMenu.AddCommandItem("Command &1", 100);
        ctxMenu.AddCommandItem("Command &2", 200);
        ctxMenu.AddCommandItem("Command &3", 300);
    }
    bool OnEvent(Reference<Control>, Event eventType, int ID) override
    {
        if (eventType == Event::WindowClose)
        {
            Application::Close();
            return true;
        }
        if (eventType == Event::Command)
        {
            for (auto i: Commands)
            {
                if (i.commandID == ID)
                {
                    status->SetText(i.text);
                    return true;
                }
            }
        }
        return false;
    }
    void OnMousePressed(int x, int y, MouseButton button, Input::Key keyCode) override
    {
        if ((button & MouseButton::Right) != MouseButton::None)
            ctxMenu.Show(this, x, y);
        Window::OnMousePressed(x, y, button, keyCode);
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
    files->AddCommandItem("&Save", MENU_CMD_SAVE, Key::Ctrl | Key::S);
    files->AddCommandItem("Save All", MENU_CMD_SAVE_ALL);
    files->AddCommandItem("&Open", MENU_CMD_OPEN, Key::Ctrl | Key::O);
    files->AddSeparator();
    files->AddCommandItem("E&xit", MENU_CMD_EXIT, Key::Ctrl | Key::Q);
    Menu* edit = Application::AddMenu("&Edit");
    edit->AddCommandItem("&Copy", MENU_CMD_COPY, Key::Ctrl | Key::Insert);
    edit->AddCommandItem("&paste", MENU_CMD_PASTE, Key::Shift | Key::Insert);
    edit->AddCommandItem("&Cut", MENU_CMD_CUT, Key::Shift | Key::Delete);
    edit->AddCommandItem("Special copy", MENU_CMD_SPECIAL_COPY, Key::Ctrl | Key::Alt | Key::Shift | Key::Insert);
    Menu* lang = Application::AddMenu("&Language");
    for (auto i : KeyWords)
        lang->AddRadioItem(i, 1000);
    lang->AddSeparator();
    for (auto i : Types)
        lang->AddRadioItem(i, 1000);
    Menu* help = Application::AddMenu("&Help");
    help->AddCommandItem("&About", MENU_CMD_ABOUT);
    help->AddCommandItem("Search &online", MENU_CMD_SEARCH);
    help->AddSeparator();
    help->AddCommandItem("Check for &updates", MENU_CMD_UPDATE);
}
int main()
{
    if (!Application::Init(InitializationFlags::Menu | InitializationFlags::CommandBar))
        return 1;
    Application::AddWindow(std::make_unique<ContextMenuExample>());
    BuildAppMenu();
    Application::Run();
    return 0;
}
