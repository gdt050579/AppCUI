#pragma once

#include "State.hpp"
#include "Game.hpp"
#include "PauseState.hpp"

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

    enum class PieceType
    {
        I = 0,
        O = 1,
        T = 2,
        S = 3,
        L = 4,
        Z = 5,
        J = 6
    };

    class Piece
    {
      private:
        static const unsigned int matrixSize = 4;
        unsigned int width                   = 1;
        unsigned int height                  = 1;
        unsigned int scale                   = 1;
        AppCUI::Graphics::ColorPair color{ AppCUI::Graphics::Color::White, AppCUI::Graphics::Color::Transparent };
        char matrix[matrixSize][matrixSize]{ 0 };
        AppCUI::Utils::Reference<AppCUI::Controls::Control> control = nullptr;

      public:
        Piece(const PieceType type,
              unsigned int scale,
              const AppCUI::Utils::Reference<AppCUI::Controls::Control> control)
        {
            this->control = control;
            this->scale   = scale;

            switch (type)
            {
            case RunningState::PieceType::I:
                matrix[0][0] = 1;
                matrix[1][0] = 1;
                matrix[2][0] = 1;
                matrix[3][0] = 1;
                break;
            case RunningState::PieceType::O:
                matrix[0][0] = 1;
                matrix[0][1] = 1;
                matrix[1][0] = 1;
                matrix[1][1] = 1;
                break;
            case RunningState::PieceType::T:
                matrix[0][0] = 1;
                matrix[0][1] = 1;
                matrix[0][2] = 1;
                matrix[1][1] = 1;
                break;
            case RunningState::PieceType::S:
                matrix[0][1] = 1;
                matrix[0][2] = 1;
                matrix[1][0] = 1;
                matrix[1][1] = 1;
                break;
            case RunningState::PieceType::L:
                matrix[0][0] = 1;
                matrix[1][0] = 1;
                matrix[1][1] = 1;
                matrix[1][2] = 1;
                break;
            case RunningState::PieceType::Z:
                matrix[0][0] = 1;
                matrix[0][1] = 1;
                matrix[1][1] = 1;
                matrix[1][2] = 1;
                break;
            case RunningState::PieceType::J:
                matrix[0][2] = 1;
                matrix[1][0] = 1;
                matrix[1][1] = 1;
                matrix[1][2] = 1;
                break;
            default:
                break;
            }
        }

        bool Draw(AppCUI::Graphics::Renderer& renderer)
        {
            int x = 1;
            int y = 1;

            for (auto i = 0U; i < matrixSize; i++)
            {
                for (auto j = 0U; j < matrixSize; j++)
                {
                    if (matrix[i][j] == 0)
                    {
                        continue;
                    }

                    renderer.DrawRectSize(x, y, width * scale * 2, height * scale, color, false);
                    x += width * scale * 2;
                }

                x = 1;
                y += height * scale;
            }

            return true;
        }
    };

  private:
    const std::shared_ptr<GameData>& data;

    unsigned int score = 0;
    std::vector<Piece> pieces{};

    AppCUI::Utils::Reference<AppCUI::Controls::TabPage> page      = nullptr;
    AppCUI::Utils::Reference<AppCUI::Controls::Panel> leftPanel   = nullptr;
    AppCUI::Utils::Reference<AppCUI::Controls::Panel> nextPiece   = nullptr;
    AppCUI::Utils::Reference<AppCUI::Controls::Panel> nextPiece01 = nullptr;
    AppCUI::Utils::Reference<AppCUI::Controls::Panel> nextPiece02 = nullptr;
    AppCUI::Utils::Reference<AppCUI::Controls::Panel> nextPiece03 = nullptr;
    AppCUI::Utils::Reference<AppCUI::Controls::Panel> rightPanel  = nullptr;
    AppCUI::Utils::Reference<AppCUI::Controls::Panel> stats       = nullptr;
    AppCUI::Utils::Reference<AppCUI::Controls::Label> scoreLabel  = nullptr;

    class PaintControlImplementation : public AppCUI::Controls::Handlers::PaintControlInterface
    {
        RunningState& rs;

      public:
        PaintControlImplementation(RunningState& rs) : rs(rs)
        {
        }

        void PaintControl(
              AppCUI::Controls::Reference<AppCUI::Controls::Control> control, AppCUI::Graphics::Renderer& renderer)
        {
            control->Paint(renderer);

            for (auto& piece : rs.pieces)
            {
                piece.Draw(renderer);
            }
        }
    };

    PaintControlImplementation pci{ *this };
};
