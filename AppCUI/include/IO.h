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
    if (color<256) { \
        SET_CHARACTER_COLOR(ptrCharInfo, color); \
    } else { \
        if (color != AppCUI::Console::Color::NoColor) { \
            unsigned int temp_color = color; \
            if (color & 256) temp_color = (GET_CHARACTER_COLOR(ptrCharInfo) & 0x0F)|(temp_color & 0xFFFFF0); \
            if (color & (256<<4)) temp_color = (GET_CHARACTER_COLOR(ptrCharInfo) & 0xF0)|(temp_color & 0xFFFF0F); \
            SET_CHARACTER_COLOR(ptrCharInfo,(temp_color & 0xFF));\
        } \
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
        protected:
            bool                        Inited;
            int                         TranslateX, TranslateY;
            AppCUI::Console::Size       ConsoleSize;
            CHARACTER_INFORMATION*      WorkingBuffer;
            CHARACTER_INFORMATION**     OffsetRows;
            struct {
                int                     Left, Top, Right, Bottom;
                bool                    Visible;
            } Clip;      
            struct {
                unsigned int            X, Y;
                bool                    Visible;
            } Cursor,LastUpdateCursor;
            struct {
                AppCUI::Console::Size   consoleSize;
                CHAR_INFO*              screenBuffer;
                unsigned int            CursorX, CursorY, CursorVisible;
            } BeforeInitConfig;

            bool            CreateScreenBuffers(unsigned int width, unsigned int height);
            bool            WriteCharacterBuffer_SingleLine(int x, int y, const AppCUI::Console::CharacterBuffer & cb, const AppCUI::Console::WriteCharacterBufferParams& params, unsigned int start, unsigned int end);
            bool            WriteCharacterBuffer_MultiLine_WithWidth(int x, int y, const AppCUI::Console::CharacterBuffer & cb, const AppCUI::Console::WriteCharacterBufferParams& params, unsigned int start, unsigned int end);
            bool            WriteCharacterBuffer_MultiLine_ProcessNewLine(int x, int y, const AppCUI::Console::CharacterBuffer & cb, const AppCUI::Console::WriteCharacterBufferParams& params, unsigned int start, unsigned int end);

            virtual bool    OnInit() = 0;
            virtual void    OnUninit() = 0;
            virtual void    OnFlushToScreen() = 0;
            virtual bool    OnUpdateCursor() = 0;

        public:
            int                         *SpecialCharacters;
        public:
            ConsoleRenderer();
            ~ConsoleRenderer();
            bool    Init();
            void    Uninit();

            // Generic methods
            bool    FillRect(int left, int top, int right, int bottom, int charCode, unsigned int color);
            bool    FillHorizontalLine(int left, int y, int right, int charCode, unsigned int color);
            bool    FillVerticalLine(int x, int top, int bottom, int charCode, unsigned int color);
            bool    DrawRect(int left, int top, int right, int bottom, unsigned int color, bool doubleLine);
            bool    ClearClipRectangle(int charCode, unsigned int color);
            bool    WriteSingleLineText(int x, int y, const char * text, unsigned int color, int textSize = -1);
            bool    WriteSingleLineTextWithHotKey(int x, int y, const char * text, unsigned int color, unsigned int hotKeyColor, int textSize = -1);
            bool    WriteMultiLineText(int x, int y, const char * text, unsigned int color, int textSize = -1);
            bool    WriteMultiLineTextWithHotKey(int x, int y, const char * text, unsigned int color, unsigned int hotKeyColor, int textSize = -1);
            bool    WriteCharacter(int x, int y, int charCode, unsigned int color);
            bool    WriteCharacterBuffer(int x, int y, const AppCUI::Console::CharacterBuffer & cb, const AppCUI::Console::WriteCharacterBufferParams& params);
            void    HideCursor();
            bool    ShowCursor(int x, int y);
            void    SetClip(const AppCUI::Console::Clip & clip);
            void    ResetClip();
            void    SetTranslate(int offX, int offY);
            bool    SetSize(unsigned int width, unsigned int height);            
            void    Prepare();
            void    Update();

            // inlines
            inline const AppCUI::Console::Size& GetConsoleSize() const { return ConsoleSize; }
        };

#       ifdef BUILD_FOR_WINDOWS
        class WindowsConsoleRenderer : public ConsoleRenderer
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
#       else
        class MACConsoleRenderer : public ConsoleRenderer
        {
            //struct {
            //    HANDLE			        hstdOut;
            //    DWORD                   stdMode;
            //} OSSpecific;
        protected:
            virtual bool    OnInit() override;
            virtual void    OnUninit() override;
            virtual void    OnFlushToScreen() override;
            virtual bool    OnUpdateCursor() override;
        };
#       endif

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
            void                    Uninit();
            void                    GetSystemEvent(AppCUI::Internal::SystemEvents::Event & evnt);
        };
    }
}

#endif
