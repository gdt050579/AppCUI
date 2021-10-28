#include "PauseState.hpp"

PauseState::PauseState(const std::shared_ptr<GameData>& data) : data(data)
{
    page = AppCUI::Controls::Factory::TabPage::Create(data->tab, "");
    label =
        AppCUI::Controls::Factory::Label::Create(page, "Pause state. Please Esc to resume...", "x:50%,y:50%, w:20%");

    page->SetOnKeyEventHandler(
        [](AppCUI::Controls::Control* control, AppCUI::Input::Key KeyCode, int AsciiCode, void* Context) -> bool
        {
            switch (KeyCode)
            {
            case AppCUI::Input::Key::Escape:
                reinterpret_cast<PauseState*>(Context)->data->machine->PopState();
                return true;
            default:
                break;
            }

            return false;
        },
        this);
}

PauseState::~PauseState()
{
    data->tab->RemoveControl(page);
}

void PauseState::Init()
{
    data->tab->SetCurrentTabPage(page);
}

bool PauseState::HandleEvent(AppCUI::Utils::Reference<AppCUI::Controls::Control> ctrl, AppCUI::Controls::Event eventType, int controlID)
{
    return false;
}

bool PauseState::Update()
{
    return false;
}

void PauseState::Draw(AppCUI::Graphics::Renderer& renderer)
{
}

void PauseState::Pause()
{
}

void PauseState::Resume()
{
    data->tab->SetCurrentTabPage(page);
}
