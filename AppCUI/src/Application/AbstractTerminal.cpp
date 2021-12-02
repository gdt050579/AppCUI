#include "Internal.hpp"

using namespace AppCUI::Internal;
using namespace AppCUI::Graphics;

AbstractTerminal::AbstractTerminal()
{
    this->LastCursorVisibility = false;
    this->LastCursorX          = 0xFFFFFFFF;
    this->LastCursorY          = 0xFFFFFFFF;
    this->Inited               = false;
}
AbstractTerminal::~AbstractTerminal()
{
}

bool AbstractTerminal::Init(const AppCUI::Application::InitializationData& initData)
{
    return this->OnInit(initData);
}
void AbstractTerminal::Uninit()
{
    // restore the original screen settings before AppCUI was initialized
    this->RestoreOriginalConsoleSettings();
    // clear up current buffer
    this->ScreenCanvas.ClearEntireSurface(' ', ColorPair{ Color::Silver, Color::Black });
    // copy the original buffer
    this->ScreenCanvas.DrawCanvas(0, 0, this->OriginalScreenCanvas);
    // restore the original buffer
    this->OnFlushToScreen();
    // Copy cursor position
    if (this->OriginalScreenCanvas.GetCursorVisibility())
        this->ScreenCanvas.SetCursor(this->OriginalScreenCanvas.GetCursorX(), this->OriginalScreenCanvas.GetCursorY());
    else
        this->ScreenCanvas.HideCursor();
    // restore the original cursor position
    this->OnUpdateCursor();
    // OS specific On-unit
    this->OnUninit();
}
void AbstractTerminal::Update()
{
    this->OnFlushToScreen();

    if ((this->ScreenCanvas.GetCursorVisibility() != this->LastCursorVisibility) ||
        (this->ScreenCanvas.GetCursorX() != this->LastCursorX) ||
        (this->ScreenCanvas.GetCursorY() != this->LastCursorY))
    {
        if (this->OnUpdateCursor())
        {
            // update last cursor information
            this->LastCursorX          = this->ScreenCanvas.GetCursorX();
            this->LastCursorY          = this->ScreenCanvas.GetCursorY();
            this->LastCursorVisibility = this->ScreenCanvas.GetCursorVisibility();
        }
    }
}
