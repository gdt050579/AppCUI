#pragma once

#include "AppCUI.hpp"
#include "StateMachine.hpp"

struct GameData
{
    std::shared_ptr<StateMachine> machine               = std::make_shared<StateMachine>(StateMachine());
    AppCUI::Utils::Reference<AppCUI::Controls::Tab> tab = nullptr;

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

class Game : public AppCUI::Controls::SingleApp
{
  public:
    Game() noexcept;

    Game(const Game& other)     = default;
    Game(Game&& other) noexcept = default;
    Game& operator=(const Game& other) = delete;
    Game& operator=(Game&& other) noexcept = delete;

    ~Game() = default;

    bool OnEvent(
          AppCUI::Utils::Reference<AppCUI::Controls::Control> ctrl,
          AppCUI::Controls::Event eventType,
          int controlID) override;
    void Paint(AppCUI::Graphics::Renderer& r) override;
    bool OnFrameUpdate() override;

  private:
    std::shared_ptr<GameData> data = std::make_shared<GameData>(GameData());
};
