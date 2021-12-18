#pragma once

#include "State.hpp"
#include "Game.hpp"
#include "PauseState.hpp"
#include "Piece.hpp"

#include <random>
#include <array>

using namespace AppCUI::Utils;
using namespace AppCUI::Input;
using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;

class RunningState : public State, public Handlers::OnKeyEventInterface
{
  public:
    explicit RunningState(const std::shared_ptr<GameData>& data);

    RunningState(const RunningState& other)     = default;
    RunningState(RunningState&& other) noexcept = default;
    RunningState& operator=(const RunningState& other) = default;
    RunningState& operator=(RunningState&& other) noexcept = default;

    ~RunningState();

    void Init() override;

    bool HandleEvent(Reference<::Control> ctrl, Event eventType, int controlID) override;
    bool Update() override;
    void Draw(Renderer& renderer) override;

    void Pause() override;
    void Resume() override;

    bool OnKeyEvent(Reference<::Control> control, Key keyCode, char16_t unicodeChar);

  private:
    class PaintControlImplementationRightPiecePanels : public Handlers::PaintControlInterface
    {
        RunningState& rs;
        const unsigned int id;

      public:
        PaintControlImplementationRightPiecePanels(RunningState& rs, unsigned int id);
        void PaintControl(::Reference<::Control> control, ::Renderer& renderer) override;
    };

    class PaintControlImplementationLeftPanel : public Handlers::PaintControlInterface
    {
        RunningState& rs;

      public:
        PaintControlImplementationLeftPanel(RunningState& rs);
        void PaintControl(Reference<::Control> control, ::Renderer& renderer) override;
    };

    class OnKeyEventInterfaceImplementationLeftPanel : public Handlers::OnKeyEventInterface
    {
        RunningState& rs;

      public:
        OnKeyEventInterfaceImplementationLeftPanel(RunningState& rs);
        bool OnKeyEvent(Reference<::Control> control, Key keyCode, char16_t unicodeChar) override;
    };

    const std::shared_ptr<GameData>& data;

    unsigned int score = 0;
    std::deque<Piece> pieces{};

    clock_t initialTime{};
    const unsigned int maxPiecesInQueue      = 3U;
    unsigned long delta                      = 0;
    unsigned long currentPieceUpdated        = -1;
    const unsigned int pieceScaleInLeftPanel = 4;

    std::random_device r;
    std::default_random_engine e1{ r() };
    std::uniform_int_distribution<int> uniform_dist{ 0, static_cast<int>(PieceType::End) - 1 };

    std::vector<Piece> piecesProcessed;
    std::optional<Piece> currentPiece;
    std::array<std::array<bool, 50>, 50> matrix{ false };
    int maxtrixVSize  = 0;
    int maxtrixHSize  = 0;
    int matrixXLeft   = 0;
    int matrixXRight  = 0;
    int matrixYTop    = 0;
    int matrixYBottom = 0;

    Reference<TabPage> page          = nullptr;
    Reference<Panel> leftPanel       = nullptr;
    Reference<Panel> nextPiece       = nullptr;
    Reference<Panel> nextPiece01     = nullptr;
    Reference<Panel> nextPiece02     = nullptr;
    Reference<Panel> nextPiece03     = nullptr;
    Reference<Panel> rightPanel      = nullptr;
    Reference<Panel> stats           = nullptr;
    Reference<Label> scoreLabel      = nullptr;
    Reference<Label> timePassedLabel = nullptr;

    PaintControlImplementationRightPiecePanels pcirpp01{ *this, 0 };
    PaintControlImplementationRightPiecePanels pcirpp02{ *this, 1 };
    PaintControlImplementationRightPiecePanels pcirpp03{ *this, 2 };

    PaintControlImplementationLeftPanel pcilp{ *this };

    OnKeyEventInterfaceImplementationLeftPanel okeiilp{ *this };

    void SetMatrixData();
};
