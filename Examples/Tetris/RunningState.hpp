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
        J = 6,

        End
    };

    class Piece
    {
      private:
        static const unsigned int cells = 4;
        unsigned int width              = 1;
        unsigned int height             = 1;
        AppCUI::Graphics::ColorPair color{ AppCUI::Graphics::Color::White, AppCUI::Graphics::Color::Transparent };
        char matrix[cells][cells]{ 0 };
        AppCUI::Utils::Reference<AppCUI::Controls::Control> control = nullptr;
        const PieceType type;

      public:
        Piece(const PieceType type, const AppCUI::Utils::Reference<AppCUI::Controls::Control> control)
            : control(control), type(type)
        {
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
                matrix[1][0] = 1;
                matrix[2][0] = 1;
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

        bool Draw(AppCUI::Graphics::Renderer& renderer, int scale, bool center, int w, int h)
        {
            if (center)
            {
                if (w < 2 || h < 2)
                {
                    return false;
                }

                const auto size = GetSize(type, scale);
                const int x     = std::max<>(1, static_cast<int>((w - size.Width) / 2));
                const int y     = std::max<>(1, static_cast<int>(static_cast<int>((h - 2 - size.Height)) / 2));

                return Draw(renderer, scale, x, y);
            }
            return Draw(renderer, scale, 1, 1);
        }

        AppCUI::Graphics::Size GetSize(PieceType type, int scale) const
        {
            const auto w = width * scale * 2;
            const auto h = height * scale;

            switch (type)
            {
            case RunningState::PieceType::I:
                return { w, h * 4 };
            case RunningState::PieceType::O:
                return { w * 2, h * 2 };
            case RunningState::PieceType::T:
                return { w * 2, h * 3 };
            case RunningState::PieceType::S:
                return { w * 3, h * 2 };
            case RunningState::PieceType::L:
                return { w * 3, h * 2 };
            case RunningState::PieceType::Z:
                return { w * 3, h * 2 };
            case RunningState::PieceType::J:
                return { w * 3, h * 2 };
            default:
                break;
            }

            return { 0, 0 };
        }

        bool Draw(AppCUI::Graphics::Renderer& renderer, int scale, const int x, const int y)
        {
            int xx = x;
            int yy = y;

            for (auto i = 0U; i < cells; i++)
            {
                for (auto j = 0U; j < cells; j++)
                {
                    if (matrix[i][j] == 1)
                    {
                        renderer.DrawRectSize(xx, yy, width * scale * 2, height * scale, color, false);
                    }

                    xx += width * scale * 2;
                }

                xx = x;
                yy += height * scale;
            }

            return true;
        }
    };

  private:
    const std::shared_ptr<GameData>& data;

    unsigned int score = 0;
    std::deque<Piece> pieces{};

    clock_t initialTime;
    const int pieceGeneration = 3; // seconds

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
