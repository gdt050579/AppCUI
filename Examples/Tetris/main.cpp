#include "Game.hpp"

using namespace AppCUI::Graphics;
using namespace AppCUI::Controls;
using namespace AppCUI::Utils;

class Game2 : public SingleApp
{
public:
    Reference<Tab> tab;
    Reference<Label> lbl;
    Reference<Button> button;

    unsigned int timer = 0;

    Game2()
    {
        tab      = Factory::Tab::Create(this, "d:c", TabFlags::HideTabs);
        auto pg1 = Factory::TabPage::Create(tab, "");
        auto pg2 = Factory::TabPage::Create(tab, "");

        lbl    = Factory::Label::Create(pg1, "lbl", "d:c, w:20, h:1");
        button = Factory::Button::Create(pg2, "button", "d:c, w:20, h:1");

        tab->SetCurrentTabPage(0);
    }

    bool OnEvent(AppCUI::Controls::Control* ctrl, AppCUI::Controls::Event eventType, int controlID) override
    {


        return false;
    }

    void Paint(AppCUI::Graphics::Renderer& r) override
    {
        r.Clear(' ', AppCUI::Graphics::ColorPair{ AppCUI::Graphics::Color::White, AppCUI::Graphics::Color::DarkBlue });
    }

    bool OnFrameUpdate() override
    {
        timer++;

        if (timer % 30 == 0)
        {
            LocalString<128> ls;
            ls.Format("%u", static_cast<unsigned int>(timer / 30));
            lbl->SetText(ls.GetText());

            return true;
        }

        if (timer > 90)
        {
            tab->SetCurrentTabPage(1);
        }

        return false;
    }
};

int main()
{
    if (AppCUI::Application::Init(
              AppCUI::Application::InitializationFlags::SingleWindowApp |
              AppCUI::Application::InitializationFlags::EnableFPSMode) == false)
    {
        return 1;
    }

    AppCUI::Application::RunSingleApp(std::make_unique<Game>());
    // AppCUI::Application::RunSingleApp(std::make_unique<Game2>());

    return 0;
}
