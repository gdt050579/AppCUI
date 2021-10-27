#pragma once

#include "State.hpp"
#include "Game.hpp"

class MainMenuState : public State
{
  public:
    explicit MainMenuState(const std::shared_ptr<GameData>& data);

    MainMenuState(const MainMenuState& other)     = default;
    MainMenuState(MainMenuState&& other) noexcept = default;
    MainMenuState& operator=(const MainMenuState& other) = default;
    MainMenuState& operator=(MainMenuState&& other) noexcept = default;

    ~MainMenuState();

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

    AppCUI::Utils::Reference<AppCUI::Controls::TabPage> page           = nullptr;
    AppCUI::Utils::Reference<AppCUI::Controls::Panel> menu             = nullptr;
    AppCUI::Utils::Reference<AppCUI::Controls::Button> startButton     = nullptr;
    AppCUI::Utils::Reference<AppCUI::Controls::Button> highScoreButton = nullptr;
    AppCUI::Utils::Reference<AppCUI::Controls::Button> exitButton      = nullptr;

  private:
    // TODO: maybe attach a handler on each button instead
    bool DoActionForControl(int controlID) const;
};
