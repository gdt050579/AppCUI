#include "MainMenuState.hpp"

namespace Tetris
{
constexpr unsigned int StartButtonID     = 0x0001;
constexpr unsigned int HighScoreButtonID = 0x0002;
constexpr unsigned int AboutButtonID     = 0x0003;
constexpr unsigned int ExitButtonID      = 0x0004;

MainMenuState::MainMenuState(const std::shared_ptr<GameData>& data) : data(data)
{
    page            = Factory::TabPage::Create(data->tab, "d:c");
    menu            = Factory::Panel::Create(page, "Main Menu", "d:c,h:11,w:30");
    startButton     = Factory::Button::Create(menu, "Start", "x:2,y:1,w:25", StartButtonID);
    highScoreButton = Factory::Button::Create(menu, "HighScore", "x:2,y:3,w:25", HighScoreButtonID);
    aboutButton     = Factory::Button::Create(menu, "About", "x:2,y:5,w:25", AboutButtonID);
    exitButton      = Factory::Button::Create(menu, "Exit", "x:2,y:7,w:25", ExitButtonID);

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

bool MainMenuState::HandleEvent(Reference<Control> /*ctrl*/, Event eventType, int controlID)
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

bool MainMenuState::Update()
{
    return false;
}

void MainMenuState::Draw(Renderer& renderer)
{
    renderer.HideCursor();
    renderer.Clear(' ', ColorPair{ Color::White, Color::DarkBlue });
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
        AppCUI::Dialogs::MessageBox::ShowNotification("Info", "High Score list! (not implemented)");
        break;
    case AboutButtonID:
        AppCUI::Dialogs::MessageBox::ShowNotification("Info", "Information / About! (not implemented)");
        break;
    case ExitButtonID:
        exitButton->RaiseEvent(Event::TerminateApplication);
        break;
    default:
        break;
    }

    return true;
}

bool MainMenuState::OnKeyEvent(Reference<Control> control, Key keyCode, char16_t /*unicodeChar*/)
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

    default:
        break;
    }

    return false;
}
} // namespace Tetris
