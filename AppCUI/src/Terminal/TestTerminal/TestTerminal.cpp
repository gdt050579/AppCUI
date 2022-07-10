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
    { "Mouse.Hold", TestTerminal::CommandID::MouseHold, 3 /* x,y,button(Left,Right,Middle) */ },
    { "Mouse.Release", TestTerminal::CommandID::MouseRelease, 2 /* x,y */ },
    { "Mouse.Click", TestTerminal::CommandID::MouseClick, 3 /* x,y,button(Left,Right,Middle) */ },
    { "Mouse.Move", TestTerminal::CommandID::MouseMove, 2 /* x,y */ },
    { "Mouse.Drag", TestTerminal::CommandID::MouseDrag, 4 /* x1,y1,x2,y2 */ },
    { "Key.Press", TestTerminal::CommandID::KeyPress, 1 /* key */ },
    { "Key.PressMultipleTimes", TestTerminal::CommandID::KeyPressMultipleTimes, 2 /* key, times */ },
    { "Key.Type", TestTerminal::CommandID::KeyType, 1 /* string with keys */ },
    { "Key.Hold", TestTerminal::CommandID::KeyHold, 1 /* shift state */ },
    { "Key.Release", TestTerminal::CommandID::KeyRelease, 0 /**/ },
    { "Print", TestTerminal::CommandID::Print, 0 /**/ },
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
    auto w = this->ScreenCanvas.GetWidth();
    auto p = this->ScreenCanvas.GetCharactersBuffer();
    auto e = p + ((size_t) w) * ((size_t) this->ScreenCanvas.GetHeight());
    auto x = 0U;

    std::cout << std::endl;
    temp.Clear();

    while (p<e)
    {
        if (p->Code < 32)
            temp.AddChar(' ');
        else if (p->Code > 127)
            temp.AddChar('?');
        else
            temp.AddChar((char) p->Code);
        x++;
        p++;
        if (x==w)
        {
            x = 0;
            std::cout << temp.GetText() << std::endl;
            temp.Clear();
        }
    }
}
void TestTerminal::AddMouseHoldCommand(const std::string_view* params)
{
    Command cmd(CommandID::MouseHold);
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
    ASSERT(x.has_value(), "First parameter (x) must be a valid int32 value -> (in Mouse.Release(x,y)");
    ASSERT(y.has_value(), "Second parameter (y) must be a valid int32 value -> (in Mouse.Release(x,y)");
    cmd.Params[0].i32Value = x.value();
    cmd.Params[1].i32Value = y.value();
    this->commandsQueue.push(cmd);
}
void TestTerminal::AddMouseMoveCommand(const std::string_view* params)
{
    Command cmd(CommandID::MouseMove);
    auto x = Number::ToInt32(params[0]);
    auto y = Number::ToInt32(params[1]);
    ASSERT(x.has_value(), "First parameter (x) must be a valid int32 value -> (in Mouse.Move(x,y)");
    ASSERT(y.has_value(), "Second parameter (y) must be a valid int32 value -> (in Mouse.Move(x,y)");
    cmd.Params[0].i32Value         = x.value();
    cmd.Params[1].i32Value         = y.value();
    cmd.Params[2].mouseButtonValue = Input::MouseButton::None;
    this->commandsQueue.push(cmd);
}
void TestTerminal::AddMouseDragCommand(const std::string_view* params)
{
    auto x1 = Number::ToInt32(params[0]);
    auto y1 = Number::ToInt32(params[1]);
    auto x2 = Number::ToInt32(params[2]);
    auto y2 = Number::ToInt32(params[3]);
    ASSERT(x1.has_value(), "First parameter (x1) must be a valid int32 value -> (in Mouse.Drag(x1,y1,x2,y2)");
    ASSERT(y1.has_value(), "Second parameter (y1) must be a valid int32 value -> (in Mouse.Drag(x1,y1,x2,y2)");
    ASSERT(x2.has_value(), "Third parameter (x2) must be a valid int32 value -> (in Mouse.Drag(x1,y1,x2,y2)");
    ASSERT(y2.has_value(), "Fourth parameter (y2) must be a valid int32 value -> (in Mouse.Drag(x1,y1,x2,y2)");

    Command cmd(CommandID::MouseHold);
    cmd.Params[0].i32Value         = x1.value();
    cmd.Params[1].i32Value         = y1.value();
    cmd.Params[2].mouseButtonValue = Input::MouseButton::Left;
    this->commandsQueue.push(cmd);

    cmd.id                         = CommandID::MouseMove;
    cmd.Params[0].i32Value         = x2.value();
    cmd.Params[1].i32Value         = y2.value();
    cmd.Params[2].mouseButtonValue = Input::MouseButton::Left;
    this->commandsQueue.push(cmd);

    cmd.id                 = CommandID::MouseRelease;
    cmd.Params[0].i32Value = x2.value();
    cmd.Params[1].i32Value = y2.value();
    this->commandsQueue.push(cmd);
}
void TestTerminal::AddKeyPressMultipleTimesCommand(const std::string_view* params)
{
    auto times = Number::ToUInt32(params[1]);
    ASSERT(
          times.has_value(),
          "Second parameter (times) must be a valid uint32 value -> (in Key.PressMultipleTimes(key,times)");
    ASSERT(
          times.value() > 0, "Second parameter (times) must be bigger than 0 -> (in Key.PressMultipleTimes(key,times)");
    auto val = times.value();
    while (val)
    {
        AddKeyPressCommand(params);
        val--;
    }
}
void TestTerminal::AddKeyPressCommand(const std::string_view* params)
{
    Command cmd(CommandID::KeyPress);
    auto k = KeyUtils::FromString(params[0]);
    ASSERT(k != Input::Key::None, "Invalid key code");
    cmd.Params[0].keyValue = k;
    if (k == Input::Key::Space)
        cmd.Params[1].charValue = ' ';
    else if (((uint32) k >= (uint32) Input::Key::A) && ((uint32) k <= (uint32) Input::Key::Z))
        cmd.Params[1].charValue = 'a' + (uint32) k - (uint32) Input::Key::A;
    else if (((uint32) k >= (uint32) Input::Key::N0) && ((uint32) k <= (uint32) Input::Key::N9))
        cmd.Params[1].charValue = '0' + (uint32) k - (uint32) Input::Key::N0;
    else
        cmd.Params[1].charValue = 0;

    this->commandsQueue.push(cmd);
}
void TestTerminal::AddKeyHoldCommand(const std::string_view* params)
{
    Command cmd(CommandID::KeyHold);
    auto k = KeyUtils::KeyModifiersFromString(params[0]);
    ASSERT(k != Input::Key::None, "Expected shift keys (Shift,Ctrl or Alt)");
    cmd.Params[0].keyValue = k;
    this->commandsQueue.push(cmd);
}
void TestTerminal::AddKeyTypeCommand(const std::string_view* params)
{
    Command cmd(CommandID::KeyPress);
    for (auto ch : params[0])
    {
        cmd.Params[0].keyValue  = KeyUtils::FromString({ &ch, 1 });
        cmd.Params[1].charValue = ch;
        this->commandsQueue.push(cmd);
    }
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
        if (start >= end)
            break;
        if ((*start) == '#')
        {
            // we have a comment --> skip till end of line
            while ((start < end) && ((*start) != '\n') && ((*start) != '\r'))
                start++;
            continue;
        }

        next  = ParseWord(start, end);
        cmd   = { start, (size_t) (next - start) };
        start = SkipSpaces(next, end);
        ASSERT(start < end, "Premature end of command -> Expecting a '(' after the comand name !");
        ASSERT(*start == '(', "Expecting a '(' after the comand name !");
        start           = SkipSpaces(start + 1, end);
        auto paramIndex = 0;
        while ((start < end) && ((*start) != ')'))
        {
            next                 = ParseWord(start, end);
            params[paramIndex++] = { start, (size_t) (next - start) };
            start                = SkipSpaces(next, end);
            if ((start < end) && ((*start) == ','))
                start = SkipSpaces(start + 1, end);
            ASSERT(paramIndex < 10, "Too many parameters --> max allowed are 9 !");
        }
        if ((start < end) && ((*start) == ')'))
            start = SkipSpaces(start + 1, end);
        // check the command ID
        auto cmdID = TestTerminal::CommandID::None;
        for (auto idx = 0U; idx < ARRAY_LEN(SupporttedCommandsFormat); idx++)
        {
            if (cmd == SupporttedCommandsFormat[idx].commandName)
            {
                ASSERT(paramIndex == SupporttedCommandsFormat[idx].paramsCount, "Invalid number of parameters");
                cmdID = SupporttedCommandsFormat[idx].id;
                break;
            }
        }
        ASSERT(cmdID != TestTerminal::CommandID::None, "Unknown command ID");
        switch (cmdID)
        {
        case TestTerminal::CommandID::MouseHold:
            AddMouseHoldCommand(params);
            break;
        case TestTerminal::CommandID::MouseRelease:
            AddMouseReleaseCommand(params);
            break;
        case TestTerminal::CommandID::MouseMove:
            AddMouseReleaseCommand(params);
            break;
        case TestTerminal::CommandID::MouseDrag:
            AddMouseDragCommand(params);
            break;
        case TestTerminal::CommandID::MouseClick:
            AddMouseHoldCommand(params);
            AddMouseReleaseCommand(params);
            break;
        case TestTerminal::CommandID::KeyPress:
            AddKeyPressCommand(params);
            break;
        case TestTerminal::CommandID::KeyPressMultipleTimes:
            AddKeyPressMultipleTimesCommand(params);
            break;
        case TestTerminal::CommandID::KeyType:
            AddKeyTypeCommand(params);
            break;
        case TestTerminal::CommandID::KeyHold:
            AddKeyHoldCommand(params);
            break;
        case TestTerminal::CommandID::KeyRelease:
            this->commandsQueue.emplace(CommandID::KeyRelease);
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
void TestTerminal::OnFlushToScreen(const Graphics::Rect& /*rect*/)
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
        auto& cmd         = this->commandsQueue.front();
        evnt.updateFrames = false;
        switch (cmd.id)
        {
        case CommandID::MouseHold:
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
        case CommandID::MouseMove:
            evnt.eventType   = SystemEventType::MouseMove;
            evnt.mouseX      = cmd.Params[0].i32Value;
            evnt.mouseY      = cmd.Params[1].i32Value;
            evnt.mouseButton = cmd.Params[2].mouseButtonValue;
            break;
        case CommandID::KeyPress:
            evnt.eventType        = SystemEventType::KeyPressed;
            evnt.keyCode          = cmd.Params[0].keyValue;
            evnt.unicodeCharacter = cmd.Params[1].charValue;
            break;
        case CommandID::KeyHold:
            evnt.eventType        = SystemEventType::ShiftStateChanged;
            evnt.keyCode          = cmd.Params[0].keyValue;
            evnt.unicodeCharacter = 0;
            break;
        case CommandID::KeyRelease:
            evnt.eventType        = SystemEventType::ShiftStateChanged;
            evnt.keyCode          = Input::Key::None;
            evnt.unicodeCharacter = 0;
            break;
        case CommandID::Print:
            evnt.eventType = SystemEventType::None;
            PrintCurrentScreen();
            break;
        case CommandID::MouseClick:
        case CommandID::KeyType:
        case CommandID::KeyPressMultipleTimes:
        case CommandID::MouseDrag:
            ASSERT(false, "Internal flow error -> cthese are composed events (should be treated at the perser side)");
            break;
        default:
            ASSERT(false, "Internal flow error -> command ID without a code path !");
            break;
        }
        // remove the command from queue
        this->commandsQueue.pop();
    }
}

bool TestTerminal::IsEventAvailable()
{
    return false;
}
bool TestTerminal::HasSupportFor(Application::SpecialCharacterSetType /*type*/)
{
    return true;
}
} // namespace AppCUI::Internal