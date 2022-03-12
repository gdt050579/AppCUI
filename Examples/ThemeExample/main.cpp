#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;
using namespace AppCUI::Graphics;

constexpr int CMD_CHANGE_THEME_DEFAULT    = 0;
constexpr int CMD_CHANGE_THEME_DARK       = 1;
constexpr int CMD_CHANGE_THEME_LIGHT      = 2;
constexpr int CMD_LOAD_DIALOG             = 3;
constexpr int CMD_LOAD_THEME_EDITOR       = 4;
constexpr int CMD_CHANGE_CHARSET_UNICODE  = 5;
constexpr int CMD_CHANGE_CHARSET_LINUXTTY = 6;
constexpr int CMD_CHANGE_CHARSET_ASCII    = 7;

class ExampleWin : public Window
{
  public:
    ExampleWin() : Window("Test", "d:c,w:70,h:24", WindowFlags::None)
    {
        Factory::Label::Create(this, "This is an example of how themes work on AppCUI", "x:8,y:1,w:66");
        auto p = Factory::Panel::Create(this, "Themes", "x:2,y:3,w:20,h:5");
        Factory::RadioBox::Create(p, "Default", "x:1,y:0,w:15", 100, CMD_CHANGE_THEME_DEFAULT)->SetChecked(true);
        Factory::RadioBox::Create(p, "Dark", "x:1,y:1,w:15", 100, CMD_CHANGE_THEME_DARK);
        Factory::RadioBox::Create(p, "Light", "x:1,y:2,w:15", 100, CMD_CHANGE_THEME_LIGHT);

        p = Factory::Panel::Create(this, "CharSet", "x:2,y:9,w:20,h:5");
        Factory::RadioBox::Create(p, "Unicode", "x:1,y:0,w:15", 101, CMD_CHANGE_CHARSET_UNICODE)->SetChecked(true);
        Factory::RadioBox::Create(p, "Linux TTY", "x:1,y:1,w:15", 101, CMD_CHANGE_CHARSET_LINUXTTY);
        Factory::RadioBox::Create(p, "Ascii", "x:1,y:2,w:15", 101, CMD_CHANGE_CHARSET_ASCII);

        auto lv = Factory::ListView::Create(
              this, "x:25,y:3,w:40,h:10", ListViewFlags::Sortable | ListViewFlags::CheckBoxes);
        lv->AddColumn("&Name", TextAlignament::Left, 15);
        lv->AddColumn("&Grade", TextAlignament::Right, 10);
        lv->AddColumn("&Class", TextAlignament::Left, 15);
        for (uint32 count = 0; count < 100; count++)
        {
            lv->AddItem("John", "9", "Math");
            lv->AddItem("Mary", "8", "English");
            lv->AddItem("Jeffrey", "7", "Math");
            lv->AddItem("Carl", "9", "Sport");
        }

        Factory::ComboBox::Create(this, "x:25,y:14,w:40", "Apple,Orange,Grapes")->SetCurentItemIndex(0);

        Factory::Button::Create(this, "&Load Dialog", "l:2,b:0,w:21", CMD_LOAD_DIALOG);
        Factory::Button::Create(this, "Theme &Editor", "l:2,b:2,w:21", CMD_LOAD_THEME_EDITOR);

        Factory::TextArea::Create(this, "MultiLine\nText", "x:25,y:16,w:40,h:5", TextAreaFlags::ShowLineNumbers);
    }
    bool OnEvent(Reference<Control> control, Event eventType, int ID) override
    {
        if (Window::OnEvent(control, eventType, ID))
            return true;
        if (eventType == Event::CheckedStatusChanged)
        {
            switch (ID)
            {
            case CMD_CHANGE_THEME_DEFAULT:
                Application::SetTheme(Application::ThemeType::Default);
                return true;
            case CMD_CHANGE_THEME_DARK:
                Application::SetTheme(Application::ThemeType::Dark);
                return true;
            case CMD_CHANGE_THEME_LIGHT:
                Application::SetTheme(Application::ThemeType::Light);
                return true;
            case CMD_CHANGE_CHARSET_UNICODE:
                Application::SetSpecialCharacterSet(Application::SpecialCharacterSetType::Unicode);
                return true;
            case CMD_CHANGE_CHARSET_LINUXTTY:
                Application::SetSpecialCharacterSet(Application::SpecialCharacterSetType::LinuxTerminal);
                return true;
            case CMD_CHANGE_CHARSET_ASCII:
                Application::SetSpecialCharacterSet(Application::SpecialCharacterSetType::Ascii);
                return true;
            }
        }
        if (eventType == Event::ButtonClicked)
        {
            switch (ID)
            {
            case CMD_LOAD_DIALOG:
                Dialogs::FileDialog::ShowOpenFileWindow("", "Images:jpg,png,bmp", ".");
                return true;
            case CMD_LOAD_THEME_EDITOR:
                Dialogs::ThemeEditor::Show();
                return true;
            }
        }
        return false;
    }
    bool OnUpdateCommandBar(CommandBar& cmd) override
    {
        cmd.SetCommand(Key::F1, "Help", 1);
        cmd.SetCommand(Key::F2, "Save", 1);
        cmd.SetCommand(Key::F3, "Load", 1);
        cmd.SetCommand(Key::F4, "Edit", 1);
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
    Menu* help = Application::AddMenu("&Help");
    help->AddCommandItem("&About", 300);
    help->AddCommandItem("Search &online", 301);
    help->AddSeparator();
    help->AddCommandItem("Check for &updates", 302);
}
int main()
{
    InitializationData initData;
    initData.Flags = InitializationFlags::Menu | InitializationFlags::CommandBar;
    initData.Theme = ThemeType::Default;
    if (!Application::Init(initData))
        return 1;
    Application::AddWindow(std::make_unique<ExampleWin>());
    BuildAppMenu();
    Application::Run();
    return 0;
}
