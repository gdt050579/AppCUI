#pragma once

#include "../../Internal.hpp"

namespace AppCUI
{
namespace Internal
{
    class WindowsTerminal : public AbstractTerminal
    {
        HANDLE hstdOut;
        HANDLE hstdIn;
        DWORD originalStdMode;
        DWORD stdMode;
        DWORD startTime;
        unique_ptr<CHAR_INFO> ConsoleBuffer;
        unsigned int ConsoleBufferCount;
        struct
        {
            unsigned int x, y;
        } lastMousePosition;
        Input::Key KeyTranslationMatrix[KEYTRANSLATION_MATRIX_SIZE];
        Input::Key shiftState;
        bool fpsMode;

        bool ResizeConsoleBuffer(unsigned int width, unsigned int height);
        bool CopyOriginalScreenBuffer(
              unsigned int width, unsigned int height, unsigned int mouseX, unsigned int mouseY);
        void BuildKeyTranslationMatrix();
        bool ResizeConsoleScreenBufferSize(unsigned int width, unsigned int height);
        bool ResizeConsoleWindowSize(unsigned int width, unsigned int height);
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
        virtual bool OnUpdateCursor() override;
        virtual void GetSystemEvent(Internal::SystemEvent& evnt) override;
        virtual bool IsEventAvailable() override;
        virtual ~WindowsTerminal();
    };
} // namespace Internal
} // namespace AppCUI
