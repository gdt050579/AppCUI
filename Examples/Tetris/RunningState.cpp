#include "RunningState.hpp"

RunningState::RunningState(const std::shared_ptr<GameData>& data) : data(data)
{
    page = AppCUI::Controls::Factory::TabPage::Create(data->tab, "");
    label =
          AppCUI::Controls::Factory::Label::Create(page, "Running state. Please Esc to pause...", "x:50%,y:50%, w:20%");

    page->SetOnKeyEventHandler(
          [](AppCUI::Controls::Control* control, AppCUI::Input::Key KeyCode, int AsciiCode, void* Context) -> bool
          {
              switch (KeyCode)
              {
              case AppCUI::Input::Key::Escape:
                  reinterpret_cast<RunningState*>(Context)->data->machine->PushState<PauseState>(
                        reinterpret_cast<RunningState*>(Context)->data, false);
                  return true;
              default:
                  break;
              }

              return false;
          },
          this);
}

RunningState::~RunningState()
{
    data->tab->RemoveControl(page);
}

void RunningState::Init()
{
    data->tab->SetCurrentTabPage(page);
}

bool RunningState::HandleEvent(
      AppCUI::Utils::Reference<AppCUI::Controls::Control> ctrl, AppCUI::Controls::Event eventType, int controlID)
{
    return false;
}

bool RunningState::Update()
{
    return false;
}

void RunningState::Draw(AppCUI::Graphics::Renderer& renderer)
{
    renderer.Clear(
          ' ', AppCUI::Graphics::ColorPair{ AppCUI::Graphics::Color::White, AppCUI::Graphics::Color::DarkBlue });
}

void RunningState::Pause()
{
}

void RunningState::Resume()
{
    data->tab->SetCurrentTabPage(page);
}
