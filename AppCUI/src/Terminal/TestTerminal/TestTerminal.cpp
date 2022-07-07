#include "TestTerminal.hpp"

namespace AppCUI::Internal
{
using namespace Application;
using namespace Graphics;

TestTerminal::TestTerminal()
{
    this->lastMousePosition = { 0xFFFFFFFFu, 0xFFFFFFFFu };
}
TestTerminal::~TestTerminal()
{
}
bool TestTerminal::OnInit(const Application::InitializationData& initData)
{
    
    // computer terminal size
    Size terminalSize = { 80, 40 };
    CHECK((terminalSize.Width > 0) && (terminalSize.Height > 0), false, "Fail to update terminal size !");

    // create canvases
    CHECK(this->ScreenCanvas.Create(terminalSize.Width, terminalSize.Height),
          false,
          "Fail to create an internal canvas of %u x %u size",
          terminalSize.Width,
          terminalSize.Height);

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