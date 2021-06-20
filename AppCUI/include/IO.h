#ifndef __APPCUI_IO_HEADER__
#define __APPCUI_IO_HEADER__

#include "AppCUI.h"
#include <cstdint>

#ifdef BUILD_FOR_WINDOWS
#   include <Windows.h>
#   define CHARACTER_INFORMATION	                    CHAR_INFO
#   define SET_CHARACTER_VALUE(ptrCharInfo,value)	    { (ptrCharInfo)->Char.UnicodeChar = (value); }
#   define SET_CHARACTER_COLOR(ptrCharInfo,color)	    { (ptrCharInfo)->Attributes = (color); }
#   define SET_CHARACTER(ptrCharInfo,value,color)       { (ptrCharInfo)->Char.UnicodeChar = (value);(ptrCharInfo)->Attributes = (color); }
#   define GET_CHARACTER_COLOR(ptrCharInfo)             ((ptrCharInfo)->Attributes)
#else 
// dummy replacements for other systems
typedef struct { int characterCode, characterColor; } CHAR_INFO, CHARACTER_INFORMATION;
#   define SET_CHARACTER_VALUE(ptrCharInfo,value)	    { (ptrCharInfo)->characterCode = (value); }
#   define SET_CHARACTER_COLOR(ptrCharInfo,color)	    { (ptrCharInfo)->characterColor = (color); }
#   define SET_CHARACTER(ptrCharInfo,value,color)       { (ptrCharInfo)->characterCode = (value);(ptrCharInfo)->characterColor = (color); }
#   define GET_CHARACTER_COLOR(ptrCharInfo)             ((ptrCharInfo)->characterColor)
#endif


#define SET_CHARACTER_EX(ptrCharInfo,value,color) {\
    if (value>=0) { SET_CHARACTER_VALUE(ptrCharInfo,value); } \
    if (color != AppCUI::Console::Color::NoColor) { \
        if (color & 256) color = (GET_CHARACTER_COLOR(ptrCharInfo) & 0x0F)|(color & 0xFFFFF0); \
        if (color & (256<<4)) color = (GET_CHARACTER_COLOR(ptrCharInfo) & 0xF0)|(color & 0xFFFF0F); \
        SET_CHARACTER_COLOR(ptrCharInfo,(color & 0xFF));\
    } \
}

#define KEYTRANSLATION_MATRIX_SIZE          256

#define BOX_TOP_LEFT        0
#define BOX_TOP_RIGHT       1
#define BOX_BOTTOM_RIGHT    2
#define BOX_BOTTOM_LEFT     3
#define BOX_HORIZONTAL      4
#define BOX_VERTICAL        5

namespace AppCUI
{
    namespace Internal
    {
        namespace SystemEvents
        {
            enum Type : unsigned int
            {
                NONE = 0,
                MOUSE_DOWN,
                MOUSE_UP,
                MOUSE_MOVE,
                APP_CLOSE,
                APP_RESIZED,
                KEY_PRESSED,
                SHIFT_STATE_CHANGED
            };
            struct Event
            {
                SystemEvents::Type      eventType;
                int                     mouseX, mouseY;
                unsigned int            newWidth, newHeight;
                unsigned int            mouseButtonState;
                AppCUI::Input::Key::Type keyCode;
                char                    asciiCode;
            };
        }

        class ConsoleRenderer
        {
            int                         TranslateX, TranslateY;
            AppCUI::Console::Size       ConsoleSize;
            CHARACTER_INFORMATION*      WorkingBuffer;
            CHARACTER_INFORMATION**     OffsetRows;
            struct {
                int                     Left, Top, Right, Bottom;
                bool                    Visible;
            } Clip;                        

#       ifdef BUILD_FOR_WINDOWS
            HANDLE			            hstdOut;
            struct {
                DWORD                   stdMode;
                AppCUI::Console::Size   consoleSize;
                CHAR_INFO*              screenBuffer;
            } BeforeInitConfig;
#       else 
            struct {
                AppCUI::Console::Size   consoleSize;
                CHAR_INFO*              screenBuffer;
            } BeforeInitConfig;
#       endif

            bool    CreateScreenBuffers(unsigned int width, unsigned int height);
        public:
            int                         *SpecialCharacters;
        public:
            ConsoleRenderer();
            bool    Init();
            bool    FillRect(int left, int top, int right, int bottom, int charCode, unsigned int color);
            bool    FillHorizontalLine(int left, int y, int right, int charCode, unsigned int color);
            bool    FillVerticalLine(int x, int top, int bottom, int charCode, unsigned int color);
            bool    DrawRect(int left, int top, int right, int bottom, unsigned int color, bool doubleLine);
            bool    ClearClipRectangle(int charCode, unsigned int color);
            bool    WriteSingleLineText(int x, int y, const char * text, unsigned int color, int textSize = -1);
            bool    WriteMultiLineText(int x, int y, const char * text, unsigned int color, int textSize = -1);
            bool    WriteCharacter(int x, int y, int charCode, unsigned int color);
            void    Close();
            void    SetClip(const AppCUI::Console::Clip & clip);
            void    ResetClip();
            void    SetTranslate(int offX, int offY);
            bool    SetSize(unsigned int width, unsigned int height);
            void    FlushToScreen();
            void    Prepare();

            // inlines
            inline const AppCUI::Console::Size& GetConsoleSize() const { return ConsoleSize; }
        };

        class InputReader
        {
            AppCUI::Input::Key::Type    KeyTranslationMatrix[KEYTRANSLATION_MATRIX_SIZE];
#       ifdef BUILD_FOR_WINDOWS
            HANDLE			            hstdIn;
            DWORD                       originalStdMode;
            AppCUI::Input::Key::Type    shiftState;
#       endif
        public:
            bool                    Init();
            void                    GetSystemEvent(AppCUI::Internal::SystemEvents::Event & evnt);
        };
    }
}

#endif
