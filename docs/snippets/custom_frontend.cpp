// Sample of docs/source/terminals.rst - compiled with the examples (docs/snippets/CMakeLists.txt).
// Renders an AppCUI application without a terminal and prints the resulting screen (UTF-8) on stdout.
#include "AppCUI.hpp"

#include <iostream>
#include <string>
#include <vector>

using namespace AppCUI;
using namespace AppCUI::Application;

// [frontend]
// A headless frontend: AppCUI draws into memory instead of a terminal (useful for tests, screenshots or for
// streaming the screen somewhere else).
class HeadlessScreen : public CustomFrontendInterface
{
    std::vector<Graphics::Character> screen;
    uint32 width{ 0 };
    uint32 height{ 0 };
    bool closed{ false };

  public:
    bool OnInit(uint32& w, uint32& h) override
    {
        w = 60; // the screen size is decided by the frontend
        h = 12;
        return true;
    }
    void OnUnInit() override
    {
    }
    void OnFlushToScreen(const Graphics::Character* characters, uint32 w, uint32 h) override
    {
        screen.assign(characters, characters + static_cast<size_t>(w) * h);
        width  = w;
        height = h;
    }
    void OnUpdateCursor(uint32, uint32, bool) override
    {
    }
    bool WaitForEvent(FrontendEvent& e, uint32) override
    {
        // a real frontend waits here for input (network, test script, ...); this one closes the application as
        // soon as the first screen has been drawn
        if (screen.empty() || closed)
            return false;
        e         = FrontendEvent{};
        e.Type    = FrontendEventType::Closed;
        closed    = true;
        return true;
    }
    std::string ToText() const
    {
        std::string text;
        for (uint32 y = 0; y < height; y++)
        {
            for (uint32 x = 0; x < width; x++)
            {
                const char16 ch = screen[static_cast<size_t>(y) * width + x].Code;
                if (ch < 0x80)
                    text.push_back(ch >= 0x20 ? static_cast<char>(ch) : ' ');
                else if (ch < 0x800)
                {
                    text.push_back(static_cast<char>(0xC0 | (ch >> 6)));
                    text.push_back(static_cast<char>(0x80 | (ch & 0x3F)));
                }
                else
                {
                    text.push_back(static_cast<char>(0xE0 | (ch >> 12)));
                    text.push_back(static_cast<char>(0x80 | ((ch >> 6) & 0x3F)));
                    text.push_back(static_cast<char>(0x80 | (ch & 0x3F)));
                }
            }
            text.push_back('\n');
        }
        return text;
    }
};
// [/frontend]

int main()
{
    // [usage]
    HeadlessScreen headless; // must outlive the application

    InitializationData initData;
    initData.Frontend            = FrontendType::Custom;
    initData.CustomFrontend      = &headless;
    initData.SpecialCharacterSet = SpecialCharacterSetType::Ascii;
    if (!Application::Init(initData))
        return 1;

    auto window = Controls::Factory::Window::Create("Headless", "d:c,w:40,h:6");
    Controls::Factory::Label::Create(*window, "Rendered without a terminal", "x:1,y:1,w:36");
    Application::AddWindow(std::move(window));
    Application::Run(); // returns after the frontend sent FrontendEventType::Closed

    std::cout << headless.ToText();
    // [/usage]
    return 0;
}
