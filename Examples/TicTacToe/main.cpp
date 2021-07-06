#include "AppCUI.h"
#include <string>
#include <cstring>

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;
using namespace AppCUI::Console;

#define CELL_WITH_X 1
#define CELL_WITH_O 2

#define X_HAS_WON_EVENT 123
#define O_HAS_WON_EVENT 321
#define DRAW_GAME       444

class TicTacToeTable : public UserControl
{
    int table[3][3];
    int currentPiece;
    int totalPieces;
public:
    TicTacToeTable() { Start();  }
    void Start()
    {
        memset(table, 0, sizeof(int) * 9);
        currentPiece = CELL_WITH_X;
        totalPieces = 0;
    }
    void ValidateResult(int x1, int y1, int x2, int y2, int x3, int y3, int & res)
    {
        if (res != 0)
            return;
        if ((table[y1][x1] == table[y2][x2]) && (table[y1][x1] == table[y3][x3]) && (table[y1][x1] != 0))
            res = table[y1][x1];
    }
    void DrawX(Console::Renderer & renderer, int x, int y)
    {
        int px = x * 5;
        int py = y * 5;
        for (int tr = 0; tr < 4; tr++)
        {
            renderer.WriteCharacter(px + tr, py + tr, '\\', ColorPair{ Color::Aqua, Color::Black });
            renderer.WriteCharacter(px + 3 - tr, py + tr, '/', ColorPair{ Color::Aqua, Color::Black });
        }
    }
    void DrawO(Console::Renderer & renderer, int x, int y)
    {
        
        int px = x * 5;
        int py = y * 5;
        renderer.WriteSingleLineText(px, py + 0, "/--\\", ColorPair{ Color::Red, Color::Black });
        renderer.WriteSingleLineText(px, py + 1, "|  |", ColorPair{ Color::Red, Color::Black });
        renderer.WriteSingleLineText(px, py + 2, "|  |", ColorPair{ Color::Red, Color::Black });
        renderer.WriteSingleLineText(px, py + 3, "\\--/", ColorPair{ Color::Red, Color::Black });
    }
    void Paint(Console::Renderer & renderer) override
    {
        renderer.Clear(' ', ColorPair{ Color::White, Color::Black });
        for (int y=0;y<3;y++) 
            for (int x = 0; x < 3; x++)
            {
                if (table[y][x] == CELL_WITH_X)
                    DrawX(renderer, x, y);
                if (table[y][x] == CELL_WITH_O)
                    DrawO(renderer, x, y);
            }
        renderer.DrawHorizontalLineWithSpecialChar(0, 4, 13, SpecialChars::BoxHorizontalSingleLine, ColorPair{ Color::White, Color::Black });
        renderer.DrawHorizontalLineWithSpecialChar(0, 9, 13, SpecialChars::BoxHorizontalSingleLine, ColorPair{ Color::White, Color::Black });
        renderer.DrawVerticalLineWithSpecialChar(4, 0, 13, SpecialChars::BoxVerticalSingleLine, ColorPair{ Color::White, Color::Black });
        renderer.DrawVerticalLineWithSpecialChar(9, 0, 13, SpecialChars::BoxVerticalSingleLine, ColorPair{ Color::White, Color::Black });
    }
    void OnMousePressed(int x, int y, int Button) override
    {
        int cell_x = MINVALUE(x / 5, 2);
        int cell_y = MINVALUE(y / 5, 2);
        if (table[cell_y][cell_x] != 0)
            return;
        table[cell_y][cell_x] = currentPiece;
        currentPiece = 3 - currentPiece;
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
            this->RaiseEvent(Event::EVENT_CUSTOM, X_HAS_WON_EVENT);
        if (res == CELL_WITH_O)
            this->RaiseEvent(Event::EVENT_CUSTOM, O_HAS_WON_EVENT);
        // check for draw
        if ((res == 0) && (totalPieces>=9))
            this->RaiseEvent(Event::EVENT_CUSTOM, DRAW_GAME);
    }
};

class TicTacToeWin : public AppCUI::Controls::Window
{
    TicTacToeTable game;
public:
    TicTacToeWin()
    {
        this->Create("TicTacToe", "a:c,w:20,h:18");
        game.Create(this, "x:2,y:1,w:14,h:14");
    }
    bool OnEvent(const void* sender, Event::Type eventType, int controlID) override
    {
        if (eventType == Event::EVENT_WINDOW_CLOSE)
        {
            Application::Close();
            return true;
        }
        if (eventType == Event::EVENT_CUSTOM)
        {
            switch(controlID)
            {
                case X_HAS_WON_EVENT: MessageBox::ShowNotification("Tic Tac Toe", "X has won !"); break;
                case O_HAS_WON_EVENT: MessageBox::ShowNotification("Tic Tac Toe", "O has won !"); break;
                case DRAW_GAME: MessageBox::ShowNotification("Tic Tac Toe", "Draw Game !"); break;
            }
            game.Start(); // restart a new game
            return true;
        }
        return false;
    }
};
int main()
{
    Application::Init(Application::Flags::HAS_COMMANDBAR);
    Application::AddWindow(new TicTacToeWin());
    Application::Run();
    return 0;
}
