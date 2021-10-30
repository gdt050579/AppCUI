#include "MainMenuState.hpp"

constexpr unsigned int StartButtonID     = 0x0001;
constexpr unsigned int HighScoreButtonID = 0x0002;
constexpr unsigned int AboutButtonID     = 0x0003;
constexpr unsigned int ExitButtonID      = 0x0004;

MainMenuState::MainMenuState(const std::shared_ptr<GameData>& data) : data(data)
{
    page            = AppCUI::Controls::Factory::TabPage::Create(data->tab, "d:c");
    menu            = AppCUI::Controls::Factory::Panel::Create(page, "Menu", "d:c,h:11,w:30");
    startButton     = AppCUI::Controls::Factory::Button::Create(menu, "Start", "x:2,y:1,w:25", StartButtonID);
    highScoreButton = AppCUI::Controls::Factory::Button::Create(menu, "HighScore", "x:2,y:3,w:25", HighScoreButtonID);
    aboutButton     = AppCUI::Controls::Factory::Button::Create(menu, "About", "x:2,y:5,w:25", AboutButtonID);
    exitButton      = AppCUI::Controls::Factory::Button::Create(menu, "Exit", "x:2,y:7,w:25", ExitButtonID);

    menu->Handlers()->OnKeyEvent = this;
}

MainMenuState::~MainMenuState()
{
    data->tab->RemoveControl(page);
}

void MainMenuState::Init()
{
    data->tab->SetCurrentTabPage(page);
}

bool MainMenuState::HandleEvent(
      AppCUI::Utils::Reference<AppCUI::Controls::Control> ctrl, AppCUI::Controls::Event eventType, int controlID)
{
    switch (eventType)
    {
    case AppCUI::Controls::Event::ButtonClicked:
        return DoActionForControl(controlID);
    case AppCUI::Controls::Event::TerminateApplication:
        AppCUI::Application::Close();
        break;
    default:
        break;
    }

    return false;
}

bool MainMenuState::Update()
{
    return false;
}

void MainMenuState::Draw(AppCUI::Graphics::Renderer& renderer)
{
    renderer.HideCursor();
    renderer.Clear(
          ' ', AppCUI::Graphics::ColorPair{ AppCUI::Graphics::Color::White, AppCUI::Graphics::Color::DarkBlue });
}

void MainMenuState::Pause()
{
}

void MainMenuState::Resume()
{
    data->tab->SetCurrentTabPage(page);
}

bool MainMenuState::DoActionForControl(int controlID)
{
    switch (controlID)
    {
    case StartButtonID:
        data->machine->PushState<RunningState>(data, true);
        break;
    case HighScoreButtonID:
        AppCUI::Dialogs::MessageBox::ShowNotification("Info", "High Score list!");
        break;
    case AboutButtonID:
        AppCUI::Dialogs::MessageBox::ShowNotification("Info", "Information / About!");
        break;
    case ExitButtonID:
        exitButton->RaiseEvent(AppCUI::Controls::Event::TerminateApplication);
        break;
    default:
        break;
    }

    return true;
}

bool MainMenuState::OnKeyEvent(
      AppCUI::Controls::Reference<AppCUI::Controls::Control> control, AppCUI::Input::Key keyCode, char16_t unicodeChar)

{
    switch (keyCode)
    {
    case AppCUI::Input::Key::Down:
    {
        const auto childrenNo = control->GetChildrenCount();

        for (auto i = 0U; i < childrenNo; i++)
        {
            auto child = control->GetChild(i);
            if (child->HasFocus())
            {
                if (i + 1U < childrenNo)
                {
                    control->GetChild(i + 1U)->SetFocus();
                }
                else
                {
                    control->GetChild(0)->SetFocus();
                }
                break;
            }
        }
    }
        return true;

    case AppCUI::Input::Key::Up:
    {
        const auto childrenNo = control->GetChildrenCount();

        for (auto i = 0U; i < childrenNo; i++)
        {
            auto child = control->GetChild(i);
            if (child->HasFocus())
            {
                if (i == 0)
                {
                    control->GetChild(childrenNo - 1U)->SetFocus();
                }
                else
                {
                    control->GetChild(i - 1U)->SetFocus();
                }
                break;
            }
        }
    }
        return true;

    case AppCUI::Input::Key::Enter:
    case AppCUI::Input::Key::Space:
    {
        const auto childrenNo = control->GetChildrenCount();

        for (auto i = 0U; i < childrenNo; i++)
        {
            auto child = control->GetChild(i);
            if (child->HasFocus())
            {
                this->DoActionForControl(child->GetControlID());
                return true;
            }
        }
    }
    default:
        break;
    }

    return false;
}
