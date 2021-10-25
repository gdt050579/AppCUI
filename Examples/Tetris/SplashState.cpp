#include "SplashState.hpp"
#include "MainMenuState.hpp"

#include <utility>
#include <chrono>

SplashState::SplashState(const std::shared_ptr<GameData>& data, const AppCUI::Controls::SingleApp* app)
    : data(data), initialTime(clock()), app(const_cast<AppCUI::Controls::SingleApp*>(app))
{
    label = AppCUI::Controls::Factory::Label::Create(this->app, "Tetris", "x:50%,y:50%, w:20%");

    AppCUI::Utils::LocalString<128> text;
    text.Format("%u", stateDuration);
    labelCounter = AppCUI::Controls::Factory::Label::Create(this->app, text.GetText(), "x:51%,y:52%, w:20%");
}

SplashState::~SplashState()
{
    app->RemoveControl(label);
    app->RemoveControl(labelCounter);
}

void SplashState::Init()
{
}

bool SplashState::HandleEvent(AppCUI::Controls::Control* ctrl, AppCUI::Controls::Event eventType, int controlID)
{
    return false;
}

bool SplashState::Update()
{
    AppCUI::Utils::LocalString<128> text;
    text.Format("%u", stateDuration - static_cast<unsigned long long>((clock() - initialTime) * 1.0 / CLOCKS_PER_SEC));
    labelCounter->SetText(text.GetText());

    if ((clock() - initialTime) * 1.0 / CLOCKS_PER_SEC > stateDuration)
    {
        data->machine->PushState<MainMenuState>(data, app, true);
    }

    return true;
}

void SplashState::Draw(AppCUI::Graphics::Renderer& renderer)
{
    renderer.Clear(
          ' ', AppCUI::Graphics::ColorPair{ AppCUI::Graphics::Color::White, AppCUI::Graphics::Color::DarkBlue });
}

void SplashState::Pause()
{
}

void SplashState::Resume()
{
}
