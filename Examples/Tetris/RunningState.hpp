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
    const unsigned int maxPiecesInQueue      = 3U;
    unsigned long delta                      = 0;
    unsigned long lastPieceAdvancementUpdate = -1;
    const unsigned int pieceScaleInLeftPanel = 4;

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

    std::vector<Piece> piecesProcessed;
    std::optional<Piece> currentPiece;

    class PaintControlImplementationRightPiecePanels : public AppCUI::Controls::Handlers::PaintControlInterface
    {
        RunningState& rs;
        const unsigned int id;

      public:
        PaintControlImplementationRightPiecePanels(RunningState& rs, unsigned int id) : rs(rs), id(id)
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

    PaintControlImplementationRightPiecePanels pcirpp01{ *this, 0 };
    PaintControlImplementationRightPiecePanels pcirpp02{ *this, 1 };
    PaintControlImplementationRightPiecePanels pcirpp03{ *this, 2 };

    class PaintControlImplementationLeftPanel : public AppCUI::Controls::Handlers::PaintControlInterface
    {
        RunningState& rs;

      public:
        PaintControlImplementationLeftPanel(RunningState& rs) : rs(rs)
        {
        }

        void PaintControl(
              AppCUI::Controls::Reference<AppCUI::Controls::Control> control, AppCUI::Graphics::Renderer& renderer)
        {
            control->Paint(renderer);

            if (rs.currentPiece.has_value())
            {
                if (rs.currentPiece->IsInitialPositionSet() == false)
                {
                    rs.currentPiece->SetInitialPosition(control->GetWidth() / 2, 1);
                }
                rs.currentPiece->Draw(renderer, rs.pieceScaleInLeftPanel);
            }

            for (auto& piece : rs.piecesProcessed)
            {
                piece.Draw(renderer, rs.pieceScaleInLeftPanel);
            }
        }
    };

    PaintControlImplementationLeftPanel pcilp{ *this };

    class OnKeyEventInterfaceImplementationLeftPanel : public AppCUI::Controls::Handlers::OnKeyEventInterface
    {
        RunningState& rs;

      public:
        OnKeyEventInterfaceImplementationLeftPanel(RunningState& rs) : rs(rs)
        {
        }

        bool OnKeyEvent(
              AppCUI::Controls::Reference<AppCUI::Controls::Control> control,
              AppCUI::Input::Key keyCode,
              char16_t unicodeChar) override
        {
            switch (keyCode)
            {
            case AppCUI::Input::Key::Left:
                if (rs.currentPiece.has_value())
                {
                    const auto bWidth = rs.currentPiece->GetBlockWidth(rs.pieceScaleInLeftPanel);
                    if (rs.currentPiece->GetLeftXPosition() - bWidth > 0)
                    {
                        rs.currentPiece->UpdatePosition(-bWidth, 0);
                        return true;
                    }
                }
                break;
            case AppCUI::Input::Key::Right:
                if (rs.currentPiece.has_value())
                {
                    const auto bWidth = rs.currentPiece->GetBlockWidth(rs.pieceScaleInLeftPanel);
                    if (rs.currentPiece->GetRightXPosition(rs.pieceScaleInLeftPanel) + bWidth < control->GetWidth())
                    {
                        rs.currentPiece->UpdatePosition(bWidth, 0);
                        return true;
                    }
                }
                break;

            case AppCUI::Input::Key::Down:
                if (rs.currentPiece.has_value())
                {
                    const auto bHeight = rs.currentPiece->GetBlockHeight(rs.pieceScaleInLeftPanel);
                    if (rs.currentPiece->GetBottomYPosition(rs.pieceScaleInLeftPanel) + bHeight < control->GetHeight())
                    {
                        rs.currentPiece->UpdatePosition(0, bHeight);
                        return true;
                    }
                }
                break;
            default:
                break;
            }

            return false;
        }
    };

    OnKeyEventInterfaceImplementationLeftPanel okeiilp{ *this };
};
