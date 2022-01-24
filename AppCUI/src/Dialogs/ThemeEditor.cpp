#include "AppCUI.hpp"

using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;

namespace AppCUI::Dialogs
{
class ThemeEditorDialog : public Window
{
    AppCUI::Application::Config cfg;
  public:
    ThemeEditorDialog(const AppCUI::Application::Config& configObject) : Window("Theme editor", "d:c,w:80,h:24",WindowFlags::Sizeable)
    {
        cfg = configObject;
    }
};

void ThemeEditor::Show()
{
    auto* cfg = Application::GetAppConfig();
    if (cfg)
    {
        ThemeEditorDialog dlg(*cfg);
        dlg.Show();
    }
}
} // namespace AppCUI::Dialogs