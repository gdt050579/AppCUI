#include "TestTerminal.hpp"

namespace AppCUI::Internal
{
using namespace Application;
using namespace Graphics;

TestTerminal::TestTerminal()
{
}
TestTerminal::~TestTerminal()
{
}
bool TestTerminal::OnInit(const Application::InitializationData& initData)
{
    // computer terminal size
    uint32 termWidth, termHeight;
    if ((initData.Width > 0) && (initData.Height > 0))
    {
        termWidth  = initData.Width;
        termHeight = initData.Height;
    }
    else
    {
        termWidth  = 100; // default values for terminal width
        termHeight = 60;  // default values for terminal height
    }

    // create canvases
    CHECK(this->ScreenCanvas.Create(termWidth, termHeight),
          false,
          "Fail to create an internal canvas of %u x %u size",
          termWidth,
          termHeight);

    return true;
}
void TestTerminal::RestoreOriginalConsoleSettings()
{
}
void TestTerminal::OnUninit()
{
}
void TestTerminal::OnFlushToScreen()
{
}
void TestTerminal::OnFlushToScreen(const Graphics::Rect& rect)
{
}
bool TestTerminal::OnUpdateCursor()
{
    return true;
}
void TestTerminal::GetSystemEvent(Internal::SystemEvent& evnt)
{
    evnt.eventType    = SystemEventType::None;
    evnt.updateFrames = false;
}

bool TestTerminal::IsEventAvailable()
{
    return false;
}
bool TestTerminal::HasSupportFor(Application::SpecialCharacterSetType type)
{
    return true;
}
} // namespace AppCUI::Internal