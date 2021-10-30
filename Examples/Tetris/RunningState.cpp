#include "RunningState.hpp"

RunningState::RunningState(const std::shared_ptr<GameData>& data) : data(data)
{
    page        = AppCUI::Controls::Factory::TabPage::Create(data->tab, "");
    leftPanel   = AppCUI::Controls::Factory::Panel::Create(page, "Game", "t:0,b:0,w:75%,x:0%,a:l");
    rightPanel  = AppCUI::Controls::Factory::Panel::Create(page, "Info", "t:0,b:0,w:25%,x:100%,a:r");
    stats       = AppCUI::Controls::Factory::Panel::Create(rightPanel, "Stats", "d:t,h:30%");
    scoreLabel  = AppCUI::Controls::Factory::Label::Create(stats, "0", "t:1,b:0,w:50%,x:2%,a:l");
    nextPiece   = AppCUI::Controls::Factory::Panel::Create(rightPanel, "Next Pieces", "d:b,h:70%");
    nextPiece01 = AppCUI::Controls::Factory::Panel::Create(nextPiece, "01", "d:t,h:35%");
    nextPiece02 = AppCUI::Controls::Factory::Panel::Create(nextPiece, "02", "d:c,h:33%");
    nextPiece03 = AppCUI::Controls::Factory::Panel::Create(nextPiece, "03", "d:b,h:33%");

    page->SetText("");
    leftPanel->SetText("");
    rightPanel->SetText("");
    scoreLabel->SetText("");

    page->Handlers()->OnKeyEvent          = this;
    nextPiece01->Handlers()->PaintControl = &pci;
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
    if (pieces.empty())
    {
        pieces.emplace_back(Piece{ PieceType::I, 3, nextPiece.DownCast<AppCUI::Controls::Control>() });
    }

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

bool RunningState::OnKeyEvent(
      AppCUI::Controls::Reference<AppCUI::Controls::Control> control, AppCUI::Input::Key keyCode, char16_t unicodeChar)

{
    switch (keyCode)
    {
    case AppCUI::Input::Key::Escape:
        this->data->machine->PushState<PauseState>(this->data, false);
        return true;
    default:
        break;
    }

    return false;
}
