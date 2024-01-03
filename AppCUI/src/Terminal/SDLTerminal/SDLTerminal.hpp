#pragma once

#include "Internal.hpp"
#include "SDL.h"
#include "SDL_ttf.h"
#include <iterator>
#include <unordered_map>

namespace AppCUI
{
namespace Internal
{
    class SDLTerminal : public AbstractTerminal
    {
      private:
        std::map<SDL_Scancode, Input::Key> KeyTranslation;
        std::map<SDL_Scancode, Input::Key> AsciiTranslation;
        Input::Key oldShiftState;
        std::chrono::time_point<std::chrono::high_resolution_clock> lastFramesUpdate;

        SDL_Window* window;
        Uint32 windowID;
        SDL_Renderer* renderer;
        std::unique_ptr<char[]> fontBuffer;
        TTF_Font* font;
        size_t charWidth;
        size_t charHeight;
        bool autoRedraw;

        std::unordered_map<uint32, SDL_Texture*> characterCache;

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
        bool InitScreen(const Application::InitializationData& initData);
        bool InitInput(const Application::InitializationData& initData);

        bool InitFont(const Application::InitializationData& initData);

        void UnInitScreen();
        void UnInitInput();

        void HandleMouse(SystemEvent& evt, const SDL_Event& eSdl);
        void HandleKeyUp(SystemEvent& evt, const SDL_Event& eSdl);
        void HandleKeyDown(SystemEvent& evt, const SDL_Event& eSdl);

        SDL_Texture* RenderCharacter(
              const uint32 charPacked, const char16_t charCode, const SDL_Color& fg, const SDL_Color& bg);
    };
} // namespace Internal
} // namespace AppCUI
