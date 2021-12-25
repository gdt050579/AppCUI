#pragma once

#include <ctime>
#include <charconv>

#include "State.hpp"
#include "Game.hpp"

namespace Tetris
{
using namespace AppCUI::Controls;
using namespace AppCUI::Utils;
using namespace AppCUI::Input;
using namespace AppCUI::Graphics;

class GameOverState : public State, public Handlers::OnKeyEventInterface
{
  public:
    explicit GameOverState(const std::shared_ptr<GameData>& data);

    GameOverState(const GameOverState& other)     = default;
    GameOverState(GameOverState&& other) noexcept = default;
    GameOverState& operator=(const GameOverState& other) = default;
    GameOverState& operator=(GameOverState&& other) noexcept = default;

    ~GameOverState();

    void Init() override;

    bool HandleEvent(Reference<Control> ctrl, Event eventType, int controlID) override;
    bool Update() override;
    void Draw(Renderer& renderer) override;

    void Pause() override;
    void Resume() override;

  private:
    const std::shared_ptr<GameData>& data;

    Reference<TabPage> page           = nullptr;
    Reference<Panel> gameOver         = nullptr;
    Reference<Label> scoreLabel       = nullptr;
    Reference<Label> timeElapsedLabel = nullptr;
    Reference<Label> messageLabel     = nullptr;

    unsigned int score;
    unsigned long timeElapsed; // in seconds

    bool OnKeyEvent(Reference<Control> control, Key keyCode, char16_t unicodeChar);
};
} // namespace Tetris
