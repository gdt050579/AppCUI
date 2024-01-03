#include "Internal.hpp"

namespace AppCUI::Internal
{
using namespace Graphics;

AbstractTerminal::AbstractTerminal()
    : lastCursorVisibility(false), lastCursorX(0xFFFFFFFF), lastCursorY(0xFFFFFFFF), inited(false)
{
}
AbstractTerminal::~AbstractTerminal()
{
}

bool AbstractTerminal::Init(const Application::InitializationData& initData)
{
    return this->OnInit(initData);
}
void AbstractTerminal::UnInit()
{
    // restore the original screen settings before AppCUI was initialized
    RestoreOriginalConsoleSettings();

    // clear up current buffer
    screenCanvas.ClearEntireSurface(' ', ColorPair{ Color::Silver, Color::Black });

    // copy the original buffer
    screenCanvas.DrawCanvas(0, 0, this->originalScreenCanvas);

    // restore the original buffer
    OnFlushToScreen();

    // Copy cursor position
    if (originalScreenCanvas.GetCursorVisibility())
    {
        screenCanvas.SetCursor(originalScreenCanvas.GetCursorX(), originalScreenCanvas.GetCursorY());
    }
    else
    {
        screenCanvas.HideCursor();
    }

    // restore the original cursor position
    OnUpdateCursor();

    // OS specific On-unit
    OnUnInit();
}
void AbstractTerminal::Update()
{
    OnFlushToScreen();

    if ((screenCanvas.GetCursorVisibility() != lastCursorVisibility) || (screenCanvas.GetCursorX() != lastCursorX) ||
        (screenCanvas.GetCursorY() != lastCursorY))
    {
        if (this->OnUpdateCursor())
        {
            // update last cursor information
            lastCursorX          = screenCanvas.GetCursorX();
            lastCursorY          = screenCanvas.GetCursorY();
            lastCursorVisibility = screenCanvas.GetCursorVisibility();
        }
    }
}
} // namespace AppCUI::Internal