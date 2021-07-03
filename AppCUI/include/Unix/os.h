#ifndef __OS_SPECIFIC_IMPLEMENTATION_FOR_UNIX__
#define __OS_SPECIFIC_IMPLEMENTATION_FOR_UNIX__

#include "../Internal.h"
#include <vector>

namespace AppCUI
{
    namespace Internal
    {
        class Console : public AbstractConsole
        {
        public:
            Console();
        protected:
            virtual bool    OnInit() override;
            virtual void    OnUninit() override;
            virtual void    OnFlushToScreen() override;
            virtual bool    OnUpdateCursor() override;
            virtual ~Console();

        protected:
            bool term_has_colors;
        };

        class Input : public AbstractInput
        {
        public:
            Input();
            virtual bool  Init() override;
            virtual void  Uninit() override;
            virtual void  GetSystemEvent(AppCUI::Internal::SystemEvents::Event & evnt) override;
            virtual ~Input();
        protected:

            std::vector<AppCUI::Input::Key::Type> KeyTranslationMatrix;
            AppCUI::Input::Key::Type shiftState;
        };
    }
}


#endif
