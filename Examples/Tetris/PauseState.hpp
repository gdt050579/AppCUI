#pragma once

#include "State.hpp"
#include "Game.hpp"
#include "MainMenuState.hpp"

namespace Tetris
{
using namespace AppCUI::Controls;
using namespace AppCUI::Utils;
using namespace AppCUI::Graphics;
using namespace AppCUI::Input;

class PauseState : public State, public Handlers::OnKeyEventInterface
{
  public:
    explicit PauseState(const std::shared_ptr<GameData>& data);

    PauseState(const PauseState& other)     = default;
    PauseState(PauseState&& other) noexcept = default;
    PauseState& operator=(const PauseState& other) = default;
    PauseState& operator=(PauseState&& other) noexcept = default;

    ~PauseState();

    void Init() override;

    bool HandleEvent(Reference<Control> ctrl, Event eventType, int controlID) override;
    bool Update() override;
    void Draw(Renderer& renderer) override;

    void Pause() override;
    void Resume() override;

  private:
    const std::shared_ptr<GameData>& data;

    Reference<TabPage> page                = nullptr;
    Reference<Panel> gameOver              = nullptr;
    Reference<Button> resumeButton         = nullptr;
    Reference<Button> exitToMainMenuButton = nullptr;
    Reference<Button> exitButton           = nullptr;

    bool DoActionForControl(int controlID);
    bool OnKeyEvent(Reference<Control> control, Key keyCode, char16_t unicodeChar);
};
} // namespace Tetris
