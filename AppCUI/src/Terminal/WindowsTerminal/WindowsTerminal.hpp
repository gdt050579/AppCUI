#pragma once

#include "../../Internal.hpp"

namespace AppCUI
{
namespace Internal
{
    constexpr uint32 KEYTRANSLATION_MATRIX_SIZE = 256;
    class WindowsTerminal : public AbstractTerminal
    {
        HANDLE hstdOut;
        HANDLE hstdIn;
        DWORD originalStdMode;
        DWORD stdMode;
        DWORD startTime;
        unique_ptr<CHAR_INFO> ConsoleBuffer;
        uint32 ConsoleBufferCount;
        struct
        {
            uint32 x, y;
        } lastMousePosition;
        Input::Key KeyTranslationMatrix[KEYTRANSLATION_MATRIX_SIZE];
        Input::Key shiftState;
        bool fpsMode;

        bool ResizeConsoleBuffer(uint32 width, uint32 height);
        bool CopyOriginalScreenBuffer(
              uint32 width, uint32 height, uint32 mouseX, uint32 mouseY);
        void BuildKeyTranslationMatrix();
        bool ResizeConsoleScreenBufferSize(uint32 width, uint32 height);
        bool ResizeConsoleWindowSize(uint32 width, uint32 height);
        Graphics::Size FullScreenTerminal();
        Graphics::Size MaximizeTerminal();
        Graphics::Size ResizeTerminal(
              const Application::InitializationData& initData, const Graphics::Size& currentSize);
        Graphics::Size UpdateTerminalSize(
              const Application::InitializationData& initData, const Graphics::Size& currentSize);
        bool ComputeCharacterSize(const Application::InitializationData& initData);

      public:
        WindowsTerminal();
        virtual bool OnInit(const Application::InitializationData& initData) override;
        virtual void RestoreOriginalConsoleSettings() override;
        virtual void OnUninit() override;
        virtual void OnFlushToScreen() override;
        virtual void OnFlushToScreen(const Graphics::Rect& r) override;
        virtual bool OnUpdateCursor() override;
        virtual void GetSystemEvent(Internal::SystemEvent& evnt) override;
        virtual bool IsEventAvailable() override;
        virtual bool HasSupportFor(Application::SpecialCharacterSetType type) override;
        virtual ~WindowsTerminal();
    };
} // namespace Internal
} // namespace AppCUI
