#pragma once

#include <ctime>
#include <charconv>

#include "State.hpp"
#include "Game.hpp"

namespace Tetris
{
using namespace AppCUI::Utils;
using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;

class SplashState : public State
{
  public:
    explicit SplashState(const std::shared_ptr<GameData>& data);

    SplashState(const SplashState& other)     = default;
    SplashState(SplashState&& other) noexcept = default;
    SplashState& operator=(const SplashState& other) = default;
    SplashState& operator=(SplashState&& other) noexcept = default;

    ~SplashState();

    void Init() override;

    bool HandleEvent(Reference<Control> ctrl, Event eventType, int controlID) override;
    bool Update() override;
    void Draw(Renderer& renderer) override;

    void Pause() override;
    void Resume() override;

  private:
    const std::shared_ptr<GameData>& data;

    clock_t initialTime;

    const int stateDuration = 3; // seconds

    Reference<TabPage> page       = nullptr;
    Reference<ImageViewer> viewer = nullptr;
    Image img;
};
} // namespace Tetris
