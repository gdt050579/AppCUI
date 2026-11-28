#include "Internal.hpp"

#ifdef MessageBox
#    undef MessageBox // <windows.h> (through Internal.hpp) maps it to MessageBoxA / MessageBoxW
#endif

namespace AppCUI::Dialogs
{
using namespace Controls;

namespace
{
    constexpr int THEME_PICKER_CMD_APPLY  = 1;
    constexpr int THEME_PICKER_CMD_EDIT   = 2;
    constexpr int THEME_PICKER_CMD_CANCEL = 3;
    constexpr uint64 NO_THEME             = 0xFFFFFFFFFFFFFFFFULL;

    string_view ThemeSourceName(Application::ThemeSource source)
    {
        switch (source)
        {
        case Application::ThemeSource::BuiltIn:
            return "built-in";
        case Application::ThemeSource::File:
            return ".theme file";
        case Application::ThemeSource::Settings:
            return "settings";
        }
        return "";
    }

    // Every move through the list applies the theme to the whole application (and notifies the theme listeners); the
    // theme that was active when the dialog opened is restored unless a theme is applied.
    class ThemePickerDialog : public Window
    {
        Reference<ListView> list;
        std::vector<Application::ThemeInfo> themes;
        Application::Config original; // the colors to restore on Cancel
        std::string originalName;
        std::string previewName; // the previewed theme (empty: the selected theme could not be loaded)
        bool applied;
        bool initializing;

      public:
        ThemePickerDialog()
            : Window("Themes", "d:c,w:56,h:20", WindowFlags::None), original(*Application::GetAppConfig()),
              originalName(Application::GetCurrentThemeName()), previewName(originalName), applied(false),
              initializing(true)
        {
            list = Factory::ListView::Create(
                  this, "l:1,t:1,r:1,b:3", { "n:&Theme,a:l,w:30", "n:Source,a:l,w:16" }, ListViewFlags::None);
            themes = Application::EnumerateThemes();
            ListViewItem current;
            for (uint32 index = 0; index < static_cast<uint32>(themes.size()); index++)
            {
                const auto& theme = themes[index];
                auto item         = list->AddItem({ std::string_view(theme.DisplayName), ThemeSourceName(theme.Source) });
                item.SetData(index);
                if (Utils::String::Equals(theme.Name.c_str(), originalName.c_str(), true))
                    current = item;
            }
            if (current.IsValid())
                list->SetCurrentItem(current);
            else
                previewName.clear(); // the active colors are not a listed theme (edited, or the file is gone)
            Factory::Button::Create(this, "&Apply", "l:1,b:0,w:14", THEME_PICKER_CMD_APPLY);
            Factory::Button::Create(this, "&Edit...", "l:17,b:0,w:14", THEME_PICKER_CMD_EDIT);
            Factory::Button::Create(this, "&Cancel", "r:1,b:0,w:14", THEME_PICKER_CMD_CANCEL);
            list->SetFocus();
            initializing = false;
        }

        bool WasApplied() const
        {
            return applied;
        }
        void Restore()
        {
            Internal::Themes::Activate(original, originalName);
        }

        void PreviewSelectedTheme()
        {
            const auto index = list->GetCurrentItem().GetData(NO_THEME);
            if (index >= themes.size())
                return;
            auto app = Application::GetApplication();
            Application::Config config = app->config;
            std::string name;
            if (Internal::Themes::Apply(config, themes[index].Name, Application::ThemeType::Default, app, name))
            {
                Internal::Themes::Activate(config, name);
                previewName = name;
            }
            else
            {
                previewName.clear(); // keep the colors of the previous theme on screen
            }
        }
        bool IsSelectedThemePreviewed()
        {
            const auto index = list->GetCurrentItem().GetData(NO_THEME);
            return index < themes.size() && !previewName.empty() &&
                   Utils::String::Equals(themes[index].Name.c_str(), previewName.c_str(), true);
        }
        void ApplyTheme()
        {
            if (!IsSelectedThemePreviewed())
                PreviewSelectedTheme(); // e.g. Enter on the first item when the active colors were not listed
            if (previewName.empty())
            {
                MessageBox::ShowError("Themes", "The selected theme could not be loaded !");
                return;
            }
            applied = true;
            Internal::Themes::AskToKeepAfterRestart(Application::GetApplication()->config, previewName, false);
            Exit(Result::Ok);
        }
        void EditTheme()
        {
            // the editor starts from the previewed colors; if it applies them, the picker is done
            if (Internal::Themes::ShowEditor())
            {
                applied = true;
                Exit(Result::Ok);
            }
        }

        bool OnEvent(Reference<Control> control, Event eventType, int ID) override
        {
            switch (eventType)
            {
            case Event::ListViewCurrentItemChanged:
                if (!initializing)
                    PreviewSelectedTheme();
                return true;
            case Event::ListViewItemPressed: // Enter / double click on a theme
            case Event::WindowAccept:
                ApplyTheme();
                return true;
            case Event::WindowClose: // Esc
                Exit(Result::Cancel);
                return true;
            case Event::ButtonClicked:
                if (ID == THEME_PICKER_CMD_APPLY)
                    ApplyTheme();
                else if (ID == THEME_PICKER_CMD_EDIT)
                    EditTheme();
                else if (ID == THEME_PICKER_CMD_CANCEL)
                    Exit(Result::Cancel);
                return true;
            default:
                break;
            }
            return Window::OnEvent(control, eventType, ID);
        }
    };
} // namespace

bool ThemePicker::Show()
{
    auto app = Application::GetApplication();
    CHECK(app && app->Inited, false, "Application has not been initialized !");
    ThemePickerDialog dlg;
    dlg.Show();
    if (dlg.WasApplied())
        return true;
    dlg.Restore();
    return false;
}
} // namespace AppCUI::Dialogs
