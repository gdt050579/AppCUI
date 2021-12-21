#pragma once

#include "State.hpp"
#include "Game.hpp"
#include "PauseState.hpp"
#include "Piece.hpp"
#include "Board.hpp"

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

    bool HandleEvent(Reference<Control> ctrl, Event eventType, int controlID) override;
    bool Update() override;
    void Draw(Renderer& renderer) override;

    void Pause() override;
    void Resume() override;

    bool OnKeyEvent(Reference<Control> control, Key keyCode, char16_t unicodeChar);

  private:
    class PaintControlImplementationRightPiecePanels : public Handlers::PaintControlInterface
    {
        Board& board;
        const unsigned int id;

      public:
        PaintControlImplementationRightPiecePanels(Board& board, unsigned int id);
        void PaintControl(Reference<Control> control, Renderer& renderer) override;
    };

    class PaintControlImplementationLeftPanel : public Handlers::PaintControlInterface
    {
        Board& board;

      public:
        PaintControlImplementationLeftPanel(Board& board);
        void PaintControl(Reference<Control> control, Renderer& renderer) override;
    };

    class OnKeyEventInterfaceImplementationLeftPanel : public Handlers::OnKeyEventInterface
    {
        Board& board;

      public:
        OnKeyEventInterfaceImplementationLeftPanel(Board& board);
        bool OnKeyEvent(Reference<Control> control, Key keyCode, char16_t unicodeChar) override;
    };

    const std::shared_ptr<GameData>& data;

    unsigned int score = 0;

    clock_t initialTime{};
    const unsigned int maxPiecesInQueue      = 3U;
    unsigned long delta                      = 0;
    const unsigned int pieceScaleInLeftPanel = 4;

    Board board;

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

    PaintControlImplementationRightPiecePanels pcirpp01{ board, 0 };
    PaintControlImplementationRightPiecePanels pcirpp02{ board, 1 };
    PaintControlImplementationRightPiecePanels pcirpp03{ board, 2 };

    PaintControlImplementationLeftPanel pcilp{ board };

    OnKeyEventInterfaceImplementationLeftPanel okeiilp{ board };
};
