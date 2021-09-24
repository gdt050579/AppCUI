#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Dialogs;

class ExampleMainWindow : public AppCUI::Controls::Window
{
    enum class ControlIds : unsigned int
    {
        ButtonShowOpen = 1
    };

    Button open;
    TextField currentFolder;
    Splitter vertical;
    Splitter horizontal;
    Tree tree;

  public:
    ExampleMainWindow()
    {
        Create("Folder tree example", "d:c, w:80%, h:60%");
        open.Create(this, "&Open", "x:1%, y:6%, w:10%", static_cast<unsigned int>(ControlIds::ButtonShowOpen));
        vertical.Create(this, "x:6%, y:0, w:11%, h:15%", true);
        horizontal.Create(this, "x:1%, y:15%, w:99%, h:5%", false);
        currentFolder.Create(this, std::filesystem::current_path().u8string(), "x:12%, y:1%, h:15%, w:87%");
        tree.Create(this, "x:1%, y:20%, w:99%, h:85%");

        tree.ClearItems();
        PopulateTree(InvalidItemHandle, std::filesystem::current_path().u16string());
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
                const auto res = FileDialog::ShowOpenFileWindow("", "", currentFolder.GetText());
                if (res.has_value())
                {
                    currentFolder.SetText(res->u8string());
                    tree.ClearItems();
                    PopulateTree(InvalidItemHandle, res->u16string());
                }

                return true;
            }
            }
        }

        return false;
    }

    bool PopulateTree(const ItemHandle handle, const std::u16string& path)
    {
        const auto fsPath = std::filesystem::path(path);

        const ItemHandle ih = tree.AddItem(handle, fsPath.filename().u16string());

        if (std::filesystem::is_directory(fsPath) == false)
        {
            return true;
        }

        const auto rdi = std::filesystem::directory_iterator(fsPath);
        for (const auto& p : rdi)
        {
            if (p.is_directory())
            {
                PopulateTree(ih, p.path().u16string());
            }
            else
            {
                tree.AddItem(handle, p.path().filename().u16string());
            }
        }

        return true;
    }
};

int main()
{
    if (Application::Init() == false)
    {
        return 1;
    }

    Application::AddWindow(std::make_unique<ExampleMainWindow>());
    Application::Run();

    return 0;
}