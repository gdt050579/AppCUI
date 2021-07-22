#ifndef __OS_SPECIFIC_IMPLEMENTATION_FOR_WINDOWS__
#define __OS_SPECIFIC_IMPLEMENTATION_FOR_WINDOWS__

#include "../Internal.h"

namespace AppCUI
{
    namespace Internal
    {
        class WindowsTerminal : public AbstractTerminal
        {
            HANDLE			            hstdOut;
            HANDLE			            hstdIn;
            DWORD                       originalStdMode;
            DWORD                       stdMode;
            CHAR_INFO*                  ConsoleBuffer;
            unsigned int                ConsoleBufferCount;
            AppCUI::Input::Key          KeyTranslationMatrix[KEYTRANSLATION_MATRIX_SIZE];
            AppCUI::Input::Key          shiftState;

            bool                        ResizeConsoleBuffer(unsigned int width, unsigned int height);
            bool                        CopyOriginalScreenBuffer(unsigned int width, unsigned int height, unsigned int mouseX, unsigned int mouseY);
        public:
            WindowsTerminal();
            virtual bool                OnInit(const InitializationData & initData) override;
            virtual void                OnUninit() override;
            virtual void                OnFlushToScreen() override;
            virtual bool                OnUpdateCursor() override;
            virtual void                GetSystemEvent(AppCUI::Internal::SystemEvents::Event & evnt) override;
            virtual bool                IsEventAvailable() override;
            virtual ~WindowsTerminal();
        };
    }
}


#endif
