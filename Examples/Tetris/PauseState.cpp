#include "PauseState.hpp"

constexpr unsigned int ResumeButtonID         = 0x0001;
constexpr unsigned int ExitToMainMenuButtonID = 0x0002;
constexpr unsigned int ExitButtonID           = 0x0003;

PauseState::PauseState(const std::shared_ptr<GameData>& data) : data(data)
{
    page         = AppCUI::Controls::Factory::TabPage::Create(data->tab, "d:c");
    menu         = AppCUI::Controls::Factory::Panel::Create(page, "Pause Menu", "d:c,h:9,w:30");
    resumeButton = AppCUI::Controls::Factory::Button::Create(menu, "Resume Game", "x:2,y:1,w:25", ResumeButtonID);
    exitToMainMenuButton =
          AppCUI::Controls::Factory::Button::Create(menu, "Exit to Main Menu", "x:2,y:3,w:25", ExitToMainMenuButtonID);
    exitButton = AppCUI::Controls::Factory::Button::Create(menu, "Exit Game", "x:2,y:5,w:25", ExitButtonID);

    menu->Handlers()->OnKeyEvent = this;
}

PauseState::~PauseState()
{
    data->tab->RemoveControl(page);
}

void PauseState::Init()
{
    data->tab->SetCurrentTabPage(page);
}

bool PauseState::HandleEvent(
      AppCUI::Utils::Reference<AppCUI::Controls::Control> ctrl, AppCUI::Controls::Event eventType, int controlID)
{
    switch (eventType)
    {
    case AppCUI::Controls::Event::ButtonClicked:
        return DoActionForControl(controlID);
    case AppCUI::Controls::Event::TerminateApplication:
        AppCUI::Application::Close();
        return true;
    default:
        break;
    }

    return false;
}

bool PauseState::Update()
{
    return false;
}

void PauseState::Draw(AppCUI::Graphics::Renderer& renderer)
{
    renderer.HideCursor();
    renderer.Clear(
          ' ', AppCUI::Graphics::ColorPair{ AppCUI::Graphics::Color::White, AppCUI::Graphics::Color::DarkBlue });
}

void PauseState::Pause()
{
}

void PauseState::Resume()
{
    data->tab->SetCurrentTabPage(page);
}

bool PauseState::DoActionForControl(int controlID)
{
    switch (controlID)
    {
    case ResumeButtonID:
        this->data->machine->PopState();
        break;
    case ExitToMainMenuButtonID:
        data->machine->PushState<MainMenuState>(data, true);
        break;
    case ExitButtonID:
        exitButton->RaiseEvent(AppCUI::Controls::Event::TerminateApplication);
        break;
    default:
        break;
    }

    return true;
}

bool PauseState::OnKeyEvent(
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
    case AppCUI::Input::Key::Escape:
        this->data->machine->PopState();
        return true;
    default:
        break;
    }

    return false;
}
