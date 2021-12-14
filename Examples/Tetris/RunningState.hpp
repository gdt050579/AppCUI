#pragma once

#include "State.hpp"
#include "Game.hpp"
#include "PauseState.hpp"
#include "Piece.hpp"

#include <random>

class RunningState : public State, public AppCUI::Controls::Handlers::OnKeyEventInterface
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

    bool OnKeyEvent(
          AppCUI::Controls::Reference<AppCUI::Controls::Control> control,
          AppCUI::Input::Key keyCode,
          char16_t unicodeChar);

  private:
    const std::shared_ptr<GameData>& data;

    unsigned int score = 0;
    std::deque<Piece> pieces{};

    clock_t initialTime{};
    const int pieceGeneration = 3; // seconds

    std::random_device r;
    std::default_random_engine e1{ r() };
    std::uniform_int_distribution<int> uniform_dist{ 0, static_cast<int>(PieceType::End) - 1 };

    AppCUI::Utils::Reference<AppCUI::Controls::TabPage> page          = nullptr;
    AppCUI::Utils::Reference<AppCUI::Controls::Panel> leftPanel       = nullptr;
    AppCUI::Utils::Reference<AppCUI::Controls::Panel> nextPiece       = nullptr;
    AppCUI::Utils::Reference<AppCUI::Controls::Panel> nextPiece01     = nullptr;
    AppCUI::Utils::Reference<AppCUI::Controls::Panel> nextPiece02     = nullptr;
    AppCUI::Utils::Reference<AppCUI::Controls::Panel> nextPiece03     = nullptr;
    AppCUI::Utils::Reference<AppCUI::Controls::Panel> rightPanel      = nullptr;
    AppCUI::Utils::Reference<AppCUI::Controls::Panel> stats           = nullptr;
    AppCUI::Utils::Reference<AppCUI::Controls::Label> scoreLabel      = nullptr;
    AppCUI::Utils::Reference<AppCUI::Controls::Label> timePassedLabel = nullptr;

    class PaintControlImplementation : public AppCUI::Controls::Handlers::PaintControlInterface
    {
        RunningState& rs;
        const unsigned int id;

      public:
        PaintControlImplementation(RunningState& rs, unsigned int id) : rs(rs), id(id)
        {
        }

        void PaintControl(
              AppCUI::Controls::Reference<AppCUI::Controls::Control> control, AppCUI::Graphics::Renderer& renderer)
        {
            control->Paint(renderer);

            if (rs.pieces.size() > id)
            {
                auto& piece = rs.pieces[id];
                piece.Draw(renderer, 3, true, control->GetWidth(), control->GetHeight());
            }
        }
    };

    PaintControlImplementation pci01{ *this, 0 };
    PaintControlImplementation pci02{ *this, 1 };
    PaintControlImplementation pci03{ *this, 2 };
};
