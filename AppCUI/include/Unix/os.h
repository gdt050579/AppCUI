#ifndef __OS_SPECIFIC_IMPLEMENTATION_FOR_UNIX__
#define __OS_SPECIFIC_IMPLEMENTATION_FOR_UNIX__

#include "../Internal.h"

namespace AppCUI
{
    namespace Internal
    {
        class Console : public AbstractConsole
        {
        protected:
            virtual bool    OnInit() override;
            virtual void    OnUninit() override;
            virtual void    OnFlushToScreen() override;
            virtual bool    OnUpdateCursor() override;
        };
        class Input : public AbstractInput
        {
        public:
            virtual bool  Init() override;
            virtual void  Uninit() override;
            virtual void  GetSystemEvent(AppCUI::Internal::SystemEvents::Event & evnt) override;
        };
    }
}


#endif
