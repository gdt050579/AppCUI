#ifndef __OS_SPECIFIC_IMPLEMENTATION_FOR_WINDOWS__
#define __OS_SPECIFIC_IMPLEMENTATION_FOR_WINDOWS__

#include "../Internal.h"

namespace AppCUI
{
    namespace Internal
    {
        class Console : public AbstractConsole
        {
            struct {
                HANDLE			        hstdOut;
                DWORD                   stdMode;
            } OSSpecific;
        protected:
            virtual bool    OnInit() override;
            virtual void    OnUninit() override;
            virtual void    OnFlushToScreen() override;
            virtual bool    OnUpdateCursor() override;
        };
        class Input : public AbstractInput
        {
            AppCUI::Input::Key::Type    KeyTranslationMatrix[KEYTRANSLATION_MATRIX_SIZE];
            HANDLE			            hstdIn;
            DWORD                       originalStdMode;
            AppCUI::Input::Key::Type    shiftState;
        public:
            virtual bool  Init() override;
            virtual void  Uninit() override;
            virtual void  GetSystemEvent(AppCUI::Internal::SystemEvents::Event & evnt) override;
        };
    }
}


#endif
