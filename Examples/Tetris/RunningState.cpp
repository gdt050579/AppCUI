#include "RunningState.hpp"

RunningState::RunningState(const std::shared_ptr<GameData>& data) : data(data)
{
    page       = AppCUI::Controls::Factory::TabPage::Create(data->tab, "");
    game       = AppCUI::Controls::Factory::Panel ::Create(page, "-", "t:0,b:0,w:75%,x:0%,a:l");
    stats      = AppCUI::Controls::Factory::Panel::Create(page, "-", "t:0,b:0,w:25%,x:100%,a:r");
    scoreLabel = AppCUI::Controls::Factory::Label::Create(stats, "0", "t:0,b:0,w:50%,x:2%,a:l");

    page->SetText("");
    game->SetText("");
    stats->SetText("");
    scoreLabel->SetText("");

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
    AppCUI::Utils::LocalString<128> text;
    text.Format("Score: %u", score);
    scoreLabel->SetText(text.GetText());

    return true;
}

void RunningState::Draw(AppCUI::Graphics::Renderer& renderer)
{
    renderer.HideCursor();
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
