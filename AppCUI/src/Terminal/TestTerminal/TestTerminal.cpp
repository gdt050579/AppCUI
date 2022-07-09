#include "TestTerminal.hpp"

namespace AppCUI::Internal
{
using namespace Application;
using namespace Graphics;
using namespace AppCUI::Utils;

struct
{
    std::string_view commandName;
    TestTerminal::CommandID id;
    uint8 paramsCount;
} SupporttedCommandsFormat[] = {
    { "Mouse.Press", TestTerminal::CommandID::MousePress, 3 /* x,y,button(Left,Right,Middle) */ },
    { "Mouse.Release", TestTerminal::CommandID::MouseRelease, 2 /* x,y */ },
};

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
std::optional<AppCUI::Input::MouseButton> StringToMouseButton(std::string_view txt)
{
    LocalString<64> temp;
    CHECK(temp.Set(txt), std::nullopt, "");
    if (temp.Equals("left", true))
        return AppCUI::Input::MouseButton::Left;
    if (temp.Equals("right", true))
        return AppCUI::Input::MouseButton::Right;
    if (temp.Equals("middle", true))
        return AppCUI::Input::MouseButton::Center;
    if (temp.Equals("center", true))
        return AppCUI::Input::MouseButton::Center;
    return std::nullopt;
}

TestTerminal::TestTerminal()
{
}
TestTerminal::~TestTerminal()
{
}
void TestTerminal::PrintCurrentScreen()
{
    LocalString<512> temp;
    for (auto y = 0u; y < this->ScreenCanvas.GetHeight();y++)
    {
        temp.Clear();
        for (auto x = 0u; x < this->ScreenCanvas.GetWidth();x++)
        {
            const auto ch = *(this->ScreenCanvas.GetCharactersBuffer() + y * this->ScreenCanvas.GetWidth() + x);
            if (ch.Code < 32)
                temp.AddChar(' ');
            else if (ch.Code > 127)
                temp.AddChar('?');
            else
                temp.AddChar((char) ch.Code);
        }
        std::cout << temp << std::endl;
    }
}
void TestTerminal::AddMousePressCommand(const std::string_view* params)
{
    Command cmd(CommandID::MousePress);
    auto x = Number::ToInt32(params[0]);
    auto y = Number::ToInt32(params[1]);
    auto b = StringToMouseButton(params[2]);
    ASSERT(x.has_value(), "First parameter (x) must be a valid int32 value -> (in Mouse.Press(x,y,button)");
    ASSERT(y.has_value(), "Second parameter (y) must be a valid int32 value -> (in Mouse.Press(x,y,button)");
    ASSERT(
          b.has_value(),
          "Third parameter (button) must be a valid value (one of Left,Right,Middle) -> (in Mouse.Press(x,y,button)");
    cmd.Params[0].i32Value         = x.value();
    cmd.Params[1].i32Value         = y.value();
    cmd.Params[2].mouseButtonValue = b.value();
    this->commandsQueue.push(cmd);
}
void TestTerminal::AddMouseReleaseCommand(const std::string_view* params)
{
    Command cmd(CommandID::MouseRelease);
    auto x = Number::ToInt32(params[0]);
    auto y = Number::ToInt32(params[1]);
    ASSERT(x.has_value(), "First parameter (x) must be a valid int32 value -> (in Mouse.Press(x,y,button)");
    ASSERT(y.has_value(), "Second parameter (y) must be a valid int32 value -> (in Mouse.Press(x,y,button)");
    cmd.Params[0].i32Value = x.value();
    cmd.Params[1].i32Value = y.value();
    this->commandsQueue.push(cmd);
}
void TestTerminal::CreateEventsQueue(std::string_view commandsScript)
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
        cmd   = { start, (size_t)(next - start) };
        start = SkipSpaces(next, end);
        ASSERT(start < end, "Premature end of command -> Expecting a '(' after the comand name !");
        ASSERT(*start == '(', "Expecting a '(' after the comand name !");
        start           = SkipSpaces(start + 1, end);
        auto paramIndex = 0;
        while ((start < end) && ((*start) != ')'))
        {
            next                 = ParseWord(start, end);
            params[paramIndex++] = { start, (size_t)(next - start) };
            start                = SkipSpaces(next, end);
            if ((start < end) && ((*start) == ','))
                start = SkipSpaces(start + 1, end);
            ASSERT(paramIndex < 10, "Too many parameters --> max allowed are 9 !");
        }
        if ((start < end) && ((*start) == ')'))
            start = SkipSpaces(start + 1, end);
        // check the command ID
        auto cmdID = TestTerminal::CommandID::None;
        for (auto idx = 0; idx < ARRAY_LEN(SupporttedCommandsFormat); idx++)
        {
            if (cmd == SupporttedCommandsFormat[idx].commandName)
            {
                ASSERT(paramIndex != SupporttedCommandsFormat[idx].paramsCount, "Invalid number of parameters");
                cmdID = SupporttedCommandsFormat[idx].id;
                break;
            }
        }
        ASSERT(cmdID != TestTerminal::CommandID::None, "Unknown command ID");
        switch (cmdID)
        {
        case TestTerminal::CommandID::MousePress:
            AddMousePressCommand(params);
            break;
        case TestTerminal::CommandID::MouseRelease:
            AddMouseReleaseCommand(params);
            break;
        case TestTerminal::CommandID::Print:
            this->commandsQueue.emplace(CommandID::Print);
            break;
        default:
            ASSERT(false, "Internal error (code path to a command ID was not treated !");
            break;
        }
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
    if (this->commandsQueue.empty())
    {
        // if no events are in the que --> then close the application
        evnt.eventType    = SystemEventType::AppClosed;
        evnt.updateFrames = false;
    }
    else
    {
        auto cmd = this->commandsQueue.front();
        this->commandsQueue.pop();
        evnt.updateFrames = false;
        switch (cmd.id)
        {
        case CommandID::MousePress:
            evnt.eventType   = SystemEventType::MouseDown;
            evnt.mouseX      = cmd.Params[0].i32Value;
            evnt.mouseY      = cmd.Params[1].i32Value;
            evnt.mouseButton = cmd.Params[2].mouseButtonValue;
            break;
        case CommandID::MouseRelease:
            evnt.eventType   = SystemEventType::MouseUp;
            evnt.mouseX      = cmd.Params[0].i32Value;
            evnt.mouseY      = cmd.Params[1].i32Value;
            evnt.mouseButton = Input::MouseButton::None;
            break;
        case CommandID::Print:
            evnt.eventType = SystemEventType::None;
            PrintCurrentScreen();
            break;
        default:
            ASSERT(false, "Internal flow error -> command ID without a code path !");
        }
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