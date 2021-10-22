#pragma once

#include "State.hpp"
#include "Game.hpp"

class MainMenuState : public State
{
  public:
    explicit MainMenuState(const std::shared_ptr<GameData>& data, const AppCUI::Controls::SingleApp* app);

    MainMenuState(const MainMenuState& other)     = default;
    MainMenuState(MainMenuState&& other) noexcept = default;
    MainMenuState& operator=(const MainMenuState& other) = default;
    MainMenuState& operator=(MainMenuState&& other) noexcept = default;

    ~MainMenuState();

    void Init() override;

    bool HandleEvent(AppCUI::Controls::Control* ctrl, AppCUI::Controls::Event eventType, int controlID) override;
    bool Update() override;
    void Draw(AppCUI::Graphics::Renderer& renderer) override;

    void Pause() override;
    void Resume() override;

  private:
    const std::shared_ptr<GameData>& data; 
    
    AppCUI::Controls::SingleApp* app = nullptr; // to be passed further

    AppCUI::Utils::Reference<AppCUI::Controls::Button> startButton = nullptr;
};
