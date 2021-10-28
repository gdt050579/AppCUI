#pragma once

#include "State.hpp"
#include "Game.hpp"
#include "PauseState.hpp"

class RunningState : public State
{
  public:
    explicit RunningState(const std::shared_ptr<GameData>& data);

    RunningState(const RunningState& other)     = default;
    RunningState(RunningState&& other) noexcept = default;
    RunningState& operator=(const RunningState& other) = default;
    RunningState& operator=(RunningState&& other) noexcept = default;

    ~RunningState();

    void Init() override;

    bool HandleEvent(
          AppCUI::Utils::Reference<AppCUI::Controls::Control> ctrl,
          AppCUI::Controls::Event eventType,
          int controlID) override;
    bool Update() override;
    void Draw(AppCUI::Graphics::Renderer& renderer) override;

    void Pause() override;
    void Resume() override;

  private:
    const std::shared_ptr<GameData>& data;

    unsigned int score = 0;

    AppCUI::Utils::Reference<AppCUI::Controls::TabPage> page     = nullptr;
    AppCUI::Utils::Reference<AppCUI::Controls::Panel> game       = nullptr;
    AppCUI::Utils::Reference<AppCUI::Controls::Panel> stats      = nullptr;
    AppCUI::Utils::Reference<AppCUI::Controls::Label> scoreLabel = nullptr;
};
