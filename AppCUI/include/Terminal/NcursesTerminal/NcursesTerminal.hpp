#include <array>
#include "Internal.h"

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
        constexpr size_t NR_APPCUI_COLORS = 16;
        using AppColor = AppCUI::Console::Color;
        
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

        constexpr size_t KEY_TRANSLATION_MATRIX_SIZE = 65536;

        class NcursesTerminal : public AbstractTerminal
        {
        private:
            std::array<AppCUI::Input::Key, KEY_TRANSLATION_MATRIX_SIZE> KeyTranslationMatrix;
            AppCUI::Input::Key shiftState;
            ColorManager colors;

        public:
            virtual bool OnInit(const InitializationData& initData) override;
            virtual void OnUninit() override;
            virtual void OnFlushToScreen() override;
            virtual bool OnUpdateCursor() override;
            virtual void GetSystemEvent(AppCUI::Internal::SystemEvents::Event & evnt) override;
            virtual bool IsEventAvailable() override;
            
        private:
            bool initScreen();
            bool initInput();

            void uninitScreen();
            void uninitInput();

            void handleMouse(SystemEvents::Event &evt, const int c);
            void handleKey(SystemEvents::Event &evt, const int c);

        };
    }
}