#include "PauseState.hpp"

namespace Tetris
{
constexpr unsigned int ResumeButtonID         = 0x0001;
constexpr unsigned int ExitToMainMenuButtonID = 0x0002;
constexpr unsigned int ExitButtonID           = 0x0003;

PauseState::PauseState(const std::shared_ptr<GameData>& data) : data(data)
{
    page         = Factory::TabPage::Create(data->tab, "d:c");
    gameOver     = Factory::Panel::Create(page, "Pause Menu", "d:c,h:9,w:30");
    resumeButton = Factory::Button::Create(gameOver, "Resume Game", "x:2,y:1,w:25", ResumeButtonID);
    exitToMainMenuButton =
          Factory::Button::Create(gameOver, "Exit to Main Menu", "x:2,y:3,w:25", ExitToMainMenuButtonID);
    exitButton = Factory::Button::Create(gameOver, "Exit Game", "x:2,y:5,w:25", ExitButtonID);

    gameOver->Handlers()->OnKeyEvent = this;
}

PauseState::~PauseState()
{
    data->tab->RemoveControl(page);
}

void PauseState::Init()
{
    data->tab->SetCurrentTabPage(page);
}

bool PauseState::HandleEvent(Reference<Control> /*ctrl*/, Event eventType, int controlID)
{
    switch (eventType)
    {
    case Event::ButtonClicked:
        return DoActionForControl(controlID);
    case Event::TerminateApplication:
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

void PauseState::Draw(Renderer& renderer)
{
    renderer.HideCursor();
    renderer.Clear(' ', ColorPair{ Color::White, Color::DarkBlue });
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
        exitButton->RaiseEvent(Event::TerminateApplication);
        break;
    default:
        break;
    }

    return true;
}

bool PauseState::OnKeyEvent(Reference<Control> control, Key keyCode, char16_t /*unicodeChar*/)

{
    switch (keyCode)
    {
    case Key::Down:
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

    case Key::Up:
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

    case Key::Escape:
        this->data->machine->PopState();
        return true;

    default:
        break;
    }

    return false;
}
} // namespace Tetris
