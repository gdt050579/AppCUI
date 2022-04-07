#pragma once

#include "../../Internal.hpp"
#include <array>
#include <map>

/*
    AppCUI uses 16 colors, 16 for background and 16 for foreground.
    Not all terminals can display colors and not all of them can display 16.

    The solution is to find a suitable mapping for each terminal configuration.
    If the terminal can display >=16 colors -> it will be a direct mapping between AppCUI and terminal colors
    If the terminan can display >=8 colors -> bright colors will be displayed as normal (without the light bit)
    If the terminal can't display colors or there are <8 colors -> no colors will be used
*/

namespace AppCUI
{
namespace Internal
{
    using AppColor = Graphics::Color;

    constexpr size_t NR_APPCUI_COLORS = 16;

    class ColorManager
    {
      public:
        ColorManager();

        // Should be called before anything can work
        void Init();
        void SetColor(const AppColor fg, const AppColor bg);
        void UnsetColor(const AppColor fg, const AppColor bg);
        void ResetColor();

      private:
        void initColorPairs();
        int getPairId(const AppColor fg, const AppColor bg);
        char mapColor(const AppColor color);

      private:
        size_t nrColors;
        std::array<int, NR_APPCUI_COLORS> appcuiColorMapping;
        std::array<int, NR_APPCUI_COLORS * NR_APPCUI_COLORS> pairMapping;
    };

    enum class TerminalMode : std::size_t
    {
        TerminalNormal = 0,
        TerminalInsert = 1,
    };

    class NcursesTerminal : public AbstractTerminal
    {
      private:
        std::map<int, Input::Key> keyTranslationMatrix;
        ColorManager colors;
        TerminalMode mode;

      public:
        virtual bool OnInit(const Application::InitializationData& initData) override;
        virtual void OnUninit() override;
        virtual void OnFlushToScreen() override;
	virtual void OnFlushToScreen(const Graphics::Rect& r) override;
        virtual bool OnUpdateCursor() override;
        virtual void GetSystemEvent(Internal::SystemEvent& evnt) override;
        virtual bool IsEventAvailable() override;
        virtual void RestoreOriginalConsoleSettings() override;
        virtual bool HasSupportFor(Application::SpecialCharacterSetType type) override;

      private:
        bool initScreen();
        bool initInput();

        void uninitScreen();
        void uninitInput();

        void handleMouse(SystemEvent& evt, const int c);
        void handleKey(SystemEvent& evt, const int c);
        void handleKeyNormalMode(SystemEvent& evt, const int c);
        void handleKeyInsertMode(SystemEvent& evt, const int c);
    };
} // namespace Internal
} // namespace AppCUI