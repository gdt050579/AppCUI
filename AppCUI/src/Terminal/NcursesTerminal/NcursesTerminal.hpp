#pragma once

#include "../../Internal.hpp"
#include <array>
#include <ncursesw/ncurses.h>

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
    using Graphics::Character;
    using Graphics::ColorPair;
    using Input::Key;
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
        void InitColorPairs();
        int GetPairId(const AppColor fg, const AppColor bg);
        char MapColor(const AppColor color);

      private:
        size_t nrColors;
        std::array<int, NR_APPCUI_COLORS> appcuiColorMapping;
        std::array<int, NR_APPCUI_COLORS * NR_APPCUI_COLORS> pairMapping;
    };

    enum class TerminalMode : std::size_t
    {
        TerminalNormal = 0,
        TerminalCombo  = 1,
    };

    class NcursesTerminal : public AbstractTerminal
    {
      private:
        const static size_t COMBO_DLG_COL = 8;
        const static size_t COMBO_DLG_ROW = 3;
        Character* canvasState;
        std::map<int, Input::Key> keyTranslationMatrix;
        ColorManager colors;
        TerminalMode mode;
        uint32 comboKeysMask   = 0;
        bool isComboModeLocked = false;

      public:
        virtual bool OnInit(const Application::InitializationData& initData) override;
        virtual void OnUnInit() override;
        virtual void OnFlushToScreen() override;
        virtual void OnFlushToScreen(const Graphics::Rect& r) override;
        virtual bool OnUpdateCursor() override;
        virtual void GetSystemEvent(Internal::SystemEvent& evnt) override;
        virtual bool IsEventAvailable() override;
        virtual void RestoreOriginalConsoleSettings() override;
        virtual bool HasSupportFor(Application::SpecialCharacterSetType type) override;

      private:
        bool InitScreen();
        bool InitInput();

        void UnInitScreen();
        void UnInitInput();

        void HandleMouse(SystemEvent& evt, const int c);
        void HandleKey(SystemEvent& evt, const int c);
        void HandleKeyNormalMode(SystemEvent& evt, const int c);
        void HandleKeyComboMode(SystemEvent& evt, const int c);

        void DrawModifiers(
              const size_t left,
              const size_t top,
              const size_t right,
              const size_t bottom,
              const ColorPair& defaultColorPair,
              const ColorPair& pressedColorPair);
        void DrawModifier(
              const size_t x,
              const size_t y,
              const ColorPair& defaultColorPair,
              const ColorPair& pressedColorPair,
              const Key& modifier,
              const char printedModifier);
    };
} // namespace Internal
} // namespace AppCUI
