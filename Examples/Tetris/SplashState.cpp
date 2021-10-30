#include "SplashState.hpp"
#include "MainMenuState.hpp"

#include <chrono>

SplashState::SplashState(const std::shared_ptr<GameData>& data) : data(data), initialTime(clock())
{
    page   = AppCUI::Controls::Factory::TabPage::Create(data->tab, "");
    viewer = AppCUI::Controls::Factory::ImageViewer::Create(page, "d:c,w:86,h:9", AppCUI::Controls::ViewerFlags::None);

    img.Create(
          35,
          7,
          "77777747777477777747777774774777777"
          "44774447744444774447744774774774444"
          "44774447744444774447744774444774444"
          "44774447777444774447777774774777777"
          "44774447744444774447747744774444477"
          "44774447744444774447747744774444477"
          "44774447777444774447747774774777777");

    viewer->SetImage(
          img,
          AppCUI::Graphics::ImageRenderingMethod::PixelTo64ColorsLargeBlock,
          AppCUI::Graphics::ImageScaleMethod::NoScale);
}

SplashState::~SplashState()
{
    data->tab->RemoveControl(page);
}

void SplashState::Init()
{
    data->tab->SetCurrentTabPage(page);
}

bool SplashState::HandleEvent(
      AppCUI::Utils::Reference<AppCUI::Controls::Control> ctrl, AppCUI::Controls::Event eventType, int controlID)
{
    return false;
}

bool SplashState::Update()
{
    if ((clock() - initialTime) * 1.0 / CLOCKS_PER_SEC > stateDuration)
    {
        data->machine->PushState<MainMenuState>(data, true);
    }

    return true;
}

void SplashState::Draw(AppCUI::Graphics::Renderer& renderer)
{
    renderer.HideCursor();
    renderer.Clear(
          ' ', AppCUI::Graphics::ColorPair{ AppCUI::Graphics::Color::White, AppCUI::Graphics::Color::DarkRed });
}

void SplashState::Pause()
{
}

void SplashState::Resume()
{
    data->tab->SetCurrentTabPage(page);
}
