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
        Create("Tree view example", "d:c, w:80%, h:60%");
        open.Create(this, "&Open", "x:1%, y:6%, w:10%", static_cast<unsigned int>(ControlIds::ButtonShowOpen));
        vertical.Create(this, "x:6%, y:0, w:11%, h:15%", true);
        horizontal.Create(this, "x:1%, y:15%, w:99%, h:5%", false);
        currentFolder.Create(this, std::filesystem::current_path().u8string(), "x:12%, y:1%, h:15%, w:87%");
        tree.Create(this, "x:1%, y:20%, w:99%, h:85%", TreeFlags::None);
        tree.SetToggleItemHandle(PopulateTree);

        tree.ClearItems();
        const auto path = std::filesystem::current_path().u16string();
        const auto root = tree.AddItem(
              InvalidItemHandle,
              std::filesystem::current_path().filename().u16string(),
              nullptr,
              false,
              std::filesystem::current_path().u16string(),
              std::filesystem::is_directory(path));
        PopulateTree(tree, root, &path);
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

                    const auto path = std::filesystem::path(res->u16string());
                    const auto root = tree.AddItem(
                          InvalidItemHandle,
                          path.filename().u16string(),
                          nullptr,
                          false,
                          path.u16string(),
                          std::filesystem::is_directory(path));
                    PopulateTree(tree, root, &path);
                }

                return true;
            }
            }
        }

        return false;
    }

    static bool PopulateTree(Tree& tree, const ItemHandle handle, const void* context)
    {
        const auto fsPath = std::filesystem::path(*reinterpret_cast<std::u16string*>(const_cast<void*>(context)));
        try
        {
            const auto rdi = std::filesystem::directory_iterator(fsPath);
            for (const auto& p : rdi)
            {
                tree.AddItem(
                      handle, p.path().filename().u16string(), nullptr, false, p.path().u16string(), p.is_directory());
            }
        }
        catch (std::exception e)
        {
            LOG_ERROR("%s", e.what());
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