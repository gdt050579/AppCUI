#include "AppCUI.hpp"
#include <string>
#include <cstring>

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;
using namespace AppCUI::Graphics;
using namespace AppCUI::Dialogs;

constexpr unsigned int CELL_WITH_X = 1;
constexpr unsigned int CELL_WITH_O = 2;

constexpr unsigned int X_HAS_WON_EVENT = 123;
constexpr unsigned int O_HAS_WON_EVENT = 321;
constexpr unsigned int DRAW_GAME       = 444;

class TicTacToeTable : public UserControl
{
    int table[3][3];
    int currentPiece;
    int totalPieces;

  public:
    TicTacToeTable(const std::string_view& layout) : UserControl(layout)
    {
        Start();
    }
    void Start()
    {
        memset(table, 0, sizeof(int) * 9);
        currentPiece = CELL_WITH_X;
        totalPieces  = 0;
    }
    void ValidateResult(int x1, int y1, int x2, int y2, int x3, int y3, int& res)
    {
        if (res != 0)
            return;
        if ((table[y1][x1] == table[y2][x2]) && (table[y1][x1] == table[y3][x3]) && (table[y1][x1] != 0))
            res = table[y1][x1];
    }
    void DrawX(Graphics::Renderer& renderer, int x, int y)
    {
        int px = x * 5;
        int py = y * 5;
        for (int tr = 0; tr < 4; tr++)
        {
            renderer.WriteCharacter(px + tr, py + tr, '\\', ColorPair{ Color::Aqua, Color::Black });
            renderer.WriteCharacter(px + 3 - tr, py + tr, '/', ColorPair{ Color::Aqua, Color::Black });
        }
    }
    void DrawO(Graphics::Renderer& renderer, int x, int y)
    {
        int px = x * 5;
        int py = y * 5;
        renderer.WriteSingleLineText(px, py + 0, "/--\\", ColorPair{ Color::Red, Color::Black });
        renderer.WriteSingleLineText(px, py + 1, "|  |", ColorPair{ Color::Red, Color::Black });
        renderer.WriteSingleLineText(px, py + 2, "|  |", ColorPair{ Color::Red, Color::Black });
        renderer.WriteSingleLineText(px, py + 3, "\\--/", ColorPair{ Color::Red, Color::Black });
    }
    void Paint(Graphics::Renderer& renderer) override
    {
        renderer.Clear(' ', ColorPair{ Color::White, Color::Black });
        for (int y = 0; y < 3; y++)
            for (int x = 0; x < 3; x++)
            {
                if (table[y][x] == CELL_WITH_X)
                    DrawX(renderer, x, y);
                if (table[y][x] == CELL_WITH_O)
                    DrawO(renderer, x, y);
            }
        renderer.FillHorizontalLineWithSpecialChar(
              0, 4, 13, SpecialChars::BoxHorizontalSingleLine, ColorPair{ Color::White, Color::Black });
        renderer.FillHorizontalLineWithSpecialChar(
              0, 9, 13, SpecialChars::BoxHorizontalSingleLine, ColorPair{ Color::White, Color::Black });
        renderer.FillVerticalLineWithSpecialChar(
              4, 0, 13, SpecialChars::BoxVerticalSingleLine, ColorPair{ Color::White, Color::Black });
        renderer.FillVerticalLineWithSpecialChar(
              9, 0, 13, SpecialChars::BoxVerticalSingleLine, ColorPair{ Color::White, Color::Black });
    }
    void OnMousePressed(int x, int y, AppCUI::Input::MouseButton) override
    {
        const int cell_x = std::min<>(x / 5, 2);
        const int cell_y = std::min<>(y / 5, 2);
        if (table[cell_y][cell_x] != 0)
            return;
        table[cell_y][cell_x] = currentPiece;
        currentPiece          = 3 - currentPiece;
        totalPieces++;
        // validate win
        int res = 0;
        ValidateResult(0, 0, 0, 1, 0, 2, res);
        ValidateResult(1, 0, 1, 1, 1, 2, res);
        ValidateResult(2, 0, 2, 1, 2, 2, res);
        ValidateResult(0, 0, 1, 0, 2, 0, res);
        ValidateResult(0, 1, 1, 1, 2, 1, res);
        ValidateResult(0, 2, 1, 2, 2, 2, res);
        ValidateResult(0, 0, 1, 1, 2, 2, res);
        ValidateResult(0, 2, 1, 1, 2, 0, res);
        if (res == CELL_WITH_X)
            this->RaiseEvent(Event::Custom, X_HAS_WON_EVENT);
        if (res == CELL_WITH_O)
            this->RaiseEvent(Event::Custom, O_HAS_WON_EVENT);
        // check for draw
        if ((res == 0) && (totalPieces >= 9))
            this->RaiseEvent(Event::Custom, DRAW_GAME);
    }
};

class TicTacToeWin : public AppCUI::Controls::Window
{
    TicTacToeTable *game;

  public:
    TicTacToeWin() : Window("TicTacToe", "d:c,w:20,h:18",WindowFlags::None)
    {
        game = this->AddControl<TicTacToeTable>(std::make_unique<TicTacToeTable>("x:2,y:1,w:14,h:14"));        
    }
    bool OnEvent(Control*, Event eventType, int controlID) override
    {
        if (eventType == Event::WindowClose)
        {
            Application::Close();
            return true;
        }
        if (eventType == Event::Custom)
        {
            switch (controlID)
            {
            case X_HAS_WON_EVENT:
                MessageBox::ShowNotification("Tic Tac Toe", "X has won !");
                break;
            case O_HAS_WON_EVENT:
                MessageBox::ShowNotification("Tic Tac Toe", "O has won !");
                break;
            case DRAW_GAME:
                MessageBox::ShowNotification("Tic Tac Toe", "Draw Game !");
                break;
            }
            game->Start(); // restart a new game
            return true;
        }
        return false;
    }
};
int main()
{
    if (!Application::Init(Application::InitializationFlags::CommandBar))
        return 1;
    Application::AddWindow(std::make_unique<TicTacToeWin>());
    Application::Run();
    return 0;
}
