#include "SplashState.hpp"
#include "MainMenuState.hpp"

#include <chrono>

namespace Tetris
{
SplashState::SplashState(const std::shared_ptr<GameData>& data) : data(data), initialTime(clock())
{
    page   = Factory::TabPage::Create(data->tab, "");
    viewer = Factory::ImageViewer::Create(page, "d:c,w:86,h:9", ViewerFlags::HideScrollBar);

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

    viewer->SetImage(img, ImageRenderingMethod::PixelTo64ColorsLargeBlock, ImageScaleMethod::NoScale);
}

SplashState::~SplashState()
{
    data->tab->RemoveControl(page);
}

void SplashState::Init()
{
    data->tab->SetCurrentTabPage(page);
}

bool SplashState::HandleEvent(Reference<Control> ctrl, Event eventType, int controlID)
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

void SplashState::Draw(Renderer& renderer)
{
    renderer.HideCursor();
    renderer.Clear(' ', ColorPair{ Color::White, Color::DarkRed });
}

void SplashState::Pause()
{
}

void SplashState::Resume()
{
    data->tab->SetCurrentTabPage(page);
}
} // namespace Tetris
