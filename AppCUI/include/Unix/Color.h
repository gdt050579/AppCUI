#ifndef _COLOR_H_INCLUDED_
#define _COLOR_H_INCLUDED_
#include <unistd.h>
#include <array>
#include <map>

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
    namespace Terminal
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


            private:
                size_t nrColors;
                std::map<AppColor, int> appcuiColorMapping;
                std::array<int, NR_APPCUI_COLORS * NR_APPCUI_COLORS> pairMapping;
        };
    }
}

#endif // _COLOR_H_INCLUDED_