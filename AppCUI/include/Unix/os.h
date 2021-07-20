#ifndef __OS_SPECIFIC_IMPLEMENTATION_FOR_UNIX__
#define __OS_SPECIFIC_IMPLEMENTATION_FOR_UNIX__

#include <map>
#include <memory>
#include <iterator>
#include <string>

#include "Internal.h"
#include "SDL.h"
#include "SDL_ttf.h"

namespace AppCUI
{
    namespace Internal
    {
        constexpr static size_t fontSize = 16;
        const static std::string fontName = "CourierNew.ttf";

        class Terminal : public AbstractTerminal
        {
        private:
            std::map<SDL_Scancode, AppCUI::Input::Key> KeyTranslation;
            std::map<SDL_Scancode, AppCUI::Input::Key> AsciiTranslation
            ;
            AppCUI::Input::Key shiftState;

            SDL_Window* window;
            SDL_Renderer* renderer;
            TTF_Font* font;
            size_t charWidth;
            size_t charHeight;

        public:
            virtual bool OnInit() override;
            virtual void OnUninit() override;
            virtual void OnFlushToScreen() override;
            virtual bool OnUpdateCursor() override;
            virtual void GetSystemEvent(AppCUI::Internal::SystemEvents::Event & evnt) override;
            virtual bool IsEventAvailable() override;


        private:
            bool initScreen();
            bool initInput();

            bool initFont();

            void uninitScreen();
            void uninitInput();

            void handleMouse(SystemEvents::Event& evt, const SDL_Event& eSdl);
            void handleKey(SystemEvents::Event &evt, const SDL_Event& eSdl);
        };
    }
}

#endif