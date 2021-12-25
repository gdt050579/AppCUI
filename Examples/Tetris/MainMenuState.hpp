#pragma once

#include "State.hpp"
#include "Game.hpp"
#include "RunningState.hpp"

namespace Tetris
{
using namespace AppCUI::Controls;
using namespace AppCUI::Utils;
using namespace AppCUI::Input;

class MainMenuState : public State, public Handlers::OnKeyEventInterface
{
  public:
    explicit MainMenuState(const std::shared_ptr<GameData>& data);

    MainMenuState(const MainMenuState& other)     = default;
    MainMenuState(MainMenuState&& other) noexcept = default;
    MainMenuState& operator=(const MainMenuState& other) = default;
    MainMenuState& operator=(MainMenuState&& other) noexcept = default;

    ~MainMenuState();

    void Init() override;

    bool HandleEvent(Reference<Control> ctrl, Event eventType, int controlID) override;
    bool Update() override;
    void Draw(AppCUI::Graphics::Renderer& renderer) override;

    void Pause() override;
    void Resume() override;

  private:
    const std::shared_ptr<GameData>& data;

    Reference<TabPage> page           = nullptr;
    Reference<Panel> menu             = nullptr;
    Reference<Button> startButton     = nullptr;
    Reference<Button> highScoreButton = nullptr;
    Reference<Button> aboutButton     = nullptr;
    Reference<Button> exitButton      = nullptr;

  private:
    bool DoActionForControl(int controlID);

    bool OnKeyEvent(Reference<Control> control, Key keyCode, char16_t unicodeChar);
};
} // namespace Tetris
