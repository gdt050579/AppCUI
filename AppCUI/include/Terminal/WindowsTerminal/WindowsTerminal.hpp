#ifndef _WINDOWS_TERMINAL_INCLUDED_
#define _WINDOWS_TERMINAL_INCLUDED_

#include "Internal.hpp"

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
        std::unique_ptr<CHAR_INFO> ConsoleBuffer;
        unsigned int ConsoleBufferCount;
        AppCUI::Input::Key KeyTranslationMatrix[KEYTRANSLATION_MATRIX_SIZE];
        AppCUI::Input::Key shiftState;

        bool ResizeConsoleBuffer(unsigned int width, unsigned int height);
        bool CopyOriginalScreenBuffer(
              unsigned int width, unsigned int height, unsigned int mouseX, unsigned int mouseY);
        void BuildKeyTranslationMatrix();
        bool ResizeConsoleScreenBufferSize(unsigned int width, unsigned int height);
        bool ResizeConsoleWindowSize(unsigned int width, unsigned int height);
        AppCUI::Graphics::Size FullScreenTerminal();
        AppCUI::Graphics::Size MaximizeTerminal();
        AppCUI::Graphics::Size ResizeTerminal(
              const AppCUI::Application::InitializationData& initData, 
              const AppCUI::Graphics::Size& currentSize);
        AppCUI::Graphics::Size UpdateTerminalSize(
              const AppCUI::Application::InitializationData& initData,
              const AppCUI::Graphics::Size& currentSize);
        bool ComputeCharacterSize(const AppCUI::Application::InitializationData& initData);

      public:
        WindowsTerminal();
        virtual bool OnInit(const AppCUI::Application::InitializationData& initData) override;
        virtual void RestoreOriginalConsoleSettings() override;
        virtual void OnUninit() override;
        virtual void OnFlushToScreen() override;
        virtual bool OnUpdateCursor() override;
        virtual void GetSystemEvent(AppCUI::Internal::SystemEvent& evnt) override;
        virtual bool IsEventAvailable() override;
        virtual ~WindowsTerminal();
    };
} // namespace Internal
} // namespace AppCUI

#endif // _WINDOWS_TERMINAL_INCLUDED_
