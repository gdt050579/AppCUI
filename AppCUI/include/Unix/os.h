#ifndef __OS_SPECIFIC_IMPLEMENTATION_FOR_UNIX__
#define __OS_SPECIFIC_IMPLEMENTATION_FOR_UNIX__

#include <array>
#include "Internal.h"
#include "Color.h"

namespace AppCUI
{
    namespace Internal
    {
        constexpr size_t KEY_TRANSLATION_MATRIX_SIZE = 65536;

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
            AppCUI::Terminal::ColorManager colors;
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
            void handleMouse(SystemEvents::Event &evt, const int c);
            void handleKey(SystemEvents::Event &evt, const int c);

        protected:

            std::array<AppCUI::Input::Key::Type, KEY_TRANSLATION_MATRIX_SIZE> KeyTranslationMatrix;
            AppCUI::Input::Key::Type shiftState;
        };
    }
}


#endif
