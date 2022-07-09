#include "TestTerminal.hpp"

namespace AppCUI::Internal
{
using namespace Application;
using namespace Graphics;

const char* SkipSpaces(const char* start, const char* end)
{
    while ((start < end) && (((*start) == ' ') || ((*start) == '\t') || ((*start) == '\n') || ((*start) == '\r')))
        start++;
    return start;
}
const char* SkipCharacter(char ch, const char* start, const char* end)
{
    // skips a character possible surrounded by spaces
    start = SkipSpaces(start, end);
    if ((start < end) && ((*start) == ch))
    {
        start = SkipSpaces(start + 1, end);
        return start;
    }
    return nullptr;
}
const char* ParseWord(const char* start, const char* end)
{
    while ((start < end) && ((*start) > 32) && ((*start) != ',') && ((*start) != '(') && ((*start) != ')'))
        start++;
    return start;
}

TestTerminal::TestTerminal()
{
}
TestTerminal::~TestTerminal()
{
}
bool TestTerminal::CreateEventsQueue(std::string_view commandsScript)
{
    const char* start = commandsScript.data();
    const char* end   = start + commandsScript.size();
    const char* next  = nullptr;
    std::string_view cmd;
    std::string_view params[10];
    while (start < end)
    {
        // format = WORD ( param1, param2, ... paramn);
        start = SkipSpaces(start, end);
        next  = ParseWord(start, end);
        cmd   = { start, next - start };
        start = SkipSpaces(next, end);
        CHECK(start < end, false, "Premature end of command -> Expecting a '(' after the comand name !");
        CHECK(*start == '(', false, "Expecting a '(' after the comand name !");
        start           = SkipSpaces(start + 1, end);
        auto paramIndex = 0;
        while ((start < end) && ((*start) != ')'))
        {
            next = ParseWord(start, end);
            params[paramIndex++] = { start, next - start };
            start                = SkipSpaces(next, end);
            if ((start < end) && ((*start) == ','))
                start = SkipSpaces(start+1, end);
            CHECK(paramIndex < 10, false, "Too many parameters --> max allowed are 9 !");
        }
        if ((start < end) && ((*start) == ')'))
            start = SkipSpaces(start + 1, end);
    }
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
    if (this->eventsQueue.empty())
    {
        // if no events are in the que --> then close the application
        evnt.eventType    = SystemEventType::AppClosed;
        evnt.updateFrames = false;
    }
    else
    {
        evnt = this->eventsQueue.front();
        this->eventsQueue.pop();
    }
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