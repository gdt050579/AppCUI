#pragma once

#include "AppCUI.hpp"
#include "StateMachine.hpp"

namespace Snake
{
using namespace AppCUI::Utils;
using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;

struct GameData
{
    std::shared_ptr<StateMachine> machine = std::make_shared<StateMachine>(StateMachine());
    Reference<Tab> tab                    = nullptr;

    unsigned int score        = 0;
    unsigned long timeElapsed = 0;

    GameData() = default;

    GameData(const GameData& other)     = delete;
    GameData(GameData&& other) noexcept = default;

    explicit GameData(GameData* other){};
    GameData& operator=(const GameData& other) = delete;
    GameData& operator=(GameData&& other) noexcept = delete;

    ~GameData() = default;
};

enum class game_progress
{
    Playing = 0,
    Pause   = 1,
    Won     = 2,
    Lose    = 3
};

class Game : public SingleApp
{
  public:
    Game() noexcept;

    Game(const Game& other)     = default;
    Game(Game&& other) noexcept = default;
    Game& operator=(const Game& other) = delete;
    Game& operator=(Game&& other) noexcept = delete;

    ~Game() = default;

    bool OnEvent(Reference<Control> ctrl, Event eventType, int controlID) override;
    void Paint(Renderer& rd) override;
    bool OnFrameUpdate() override;

  private:
    std::shared_ptr<GameData> data = std::make_shared<GameData>(GameData());
};
} // namespace Tetris
