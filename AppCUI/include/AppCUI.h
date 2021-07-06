#ifndef __APPCUI_MAIN_HEADER__
#define __APPCUI_MAIN_HEADER__
#include <array>

#ifdef BUILD_AS_DYNAMIC_LIB
#   ifdef BUILD_FOR_WINDOWS
#       define EXPORT __declspec(dllexport)
#   else
#       define EXPORT
#   endif
#else
#   define EXPORT
#endif

#define CHECK(c,returnValue,format,...) { if (!(c)) return (returnValue); }
#define RETURNERROR(returnValue,format,...) { return (returnValue);}
#define CHECKBK(c,format,...) if (!(c)) break;

#define NOT_IMPLEMENTED(returnValue) { return (returnValue); }
#define LOG_INFO(format,...)
#define LOG_ERROR(format,...)

#define PATH_SEPARATOR '\\'

#define MAXVALUE(x,y)	((x)>(y)?(x):(y))
#define MINVALUE(x,y)	((x)<(y)?(x):(y))

namespace AppCUI
{
    namespace Application
    {
        struct Config;
        class CommandBar;
    };
    namespace Input
    {
        namespace Key
        {
            enum Type : unsigned int
            {
                None = 0,
                F1 = 1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
                Enter, Escape, Insert, Delete, Backspace, Tab,
                Left, Up, Down, Right, PageUp, PageDown, Home, End, Space,
                A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
                N0, N1, N2, N3, N4, N5, N6, N7, N8, N9,
                Count, // must be the last
                Alt = 0x1000,
                Ctrl = 0x2000,
                Shift = 0x4000
            };
        }
    };
    namespace Utils
    {
        class EXPORT String 
        {
            char	            *Text;
            unsigned int		Size;
            unsigned int	    Allocated;

            bool				Grow(unsigned int newSize);
        public:
            String(void);
            String(const String &s);
            ~String(void);

            // Static functions
            static unsigned int	Len         (const char *string);
            static bool		    Add         (char *destination, const char *source, unsigned int maxDestinationSize, unsigned int destinationSize = 0xFFFFFFFF, unsigned int sourceSize = 0xFFFFFFFF, unsigned int * resultedDestinationSize = nullptr);
            static bool		    Set         (char *destination, const char *source, unsigned int maxDestinationSize, unsigned int sourceSize = 0xFFFFFFFF, unsigned int * resultedDestinationSize = nullptr);
            static bool         Equals      (const char *sir1, const char *sir2, bool ignoreCase = false);
            static bool		    StartsWith  (const char *sir, const char *text, bool ignoreCase = false);
            static bool		    EndsWith    (const char *sir, const char *text, bool ignoreCase = false, unsigned int sirTextSize = 0xFFFFFFFF, unsigned int textSize = 0xFFFFFFFF);

            // Create string object
            bool			    Create(unsigned int initialAllocatedBuffer = 64);
            bool			    Create(const char* text);
            bool			    Create(char* buffer, unsigned int bufferSize, bool emptyString = false);

            const char*		    GetText() const { return Text; }
            unsigned int	    Len() const { return Size; }
            unsigned int	    GetAllocatedSize() const { return Allocated & 0x7FFFFFFF; }

            int				    GetChar(int index) const;
            bool			    SetChar(int index, char value);

            bool			    Add(const char *text, unsigned int size = 0xFFFFFFFF);
            bool			    Add(const String& text);
            bool			    Add(const String* text);
            bool			    AddChar(char ch);
            bool			    AddChars(char ch, unsigned int count);

            bool                InsertChar(char character, unsigned int position);
            bool                DeleteChar(unsigned int position);
            bool                Delete(unsigned int start, unsigned int end);

            bool			    Set(const char *text, unsigned int size = 0xFFFFFFFF);
            bool			    Set(const String& text);
            bool			    Set(const String* text);
            bool			    SetChars(char ch, unsigned int count);

            bool			    SetFormat(const char *format, ...);
            bool			    AddFormat(const char *format, ...);
            const char*		    Format(const char *format, ...);

            bool			    Realloc(unsigned int newSize);
            void			    Destroy();
            bool			    Truncate(unsigned int newSize);
            void			    Clear();

            bool			    StartsWith(const char *text, bool ignoreCase = false) const;
            bool			    StartsWith(const String *text, bool ignoreCase = false) const;
            bool			    StartsWith(const String &text, bool ignoreCase = false) const;
            bool			    EndsWith(const char *text, bool ignoreCase = false) const;
            bool			    EndsWith(const String *text, bool ignoreCase = false) const;
            bool			    EndsWith(const String &text, bool ignoreCase = false) const;
            bool			    Equals(const char *text, bool ignoreCase = false) const;
            bool			    Equals(const String &ss, bool ignoreCase = false) const;

            
            inline String&	    operator=  (const String &s) { this->Set(s); return *this; }
            inline String&      operator=  (const char * text) { this->Set(text); return *this; }
            inline              operator char *() const { return Text; }
            inline              operator const char *() const  { return Text; }
            inline String&      operator+= (const String &s) { this->Add(s); return *this; }
            inline String&      operator+= (const char* text) { this->Add(text); return *this; }

            inline bool         operator== (const String &s) const { return this->Equals(s); }
            inline bool		    operator!= (const String &s) const { return !this->Equals(s); }
            char&		        operator[] (int poz);
            
        };
        
        template <int size>
        class LocalString : public String
        {
            char tempBuffer[size];
        public:
            LocalString() { Create(tempBuffer, size, true); }
        };

        class EXPORT KeyUtils
        {
        public:
            enum { KEY_SHIFT_MASK = 0x7000, KEY_SHIFT_BITS = 12, KEY_CODE_MASK = 0xFF };

            // Returns the name of the Key without modifiers
            static const char*	            GetKeyName(AppCUI::Input::Key::Type keyCode);
            static const char*	            GetKeyNamePadded(AppCUI::Input::Key::Type keyCode, unsigned int * nameSize = nullptr);
            static const char*	            GetKeyModifierName(AppCUI::Input::Key::Type keyCode, unsigned int * nameSize = nullptr);
            static bool			            ToString(AppCUI::Input::Key::Type keyCode, char* text, int maxTextSize);
            static bool			            ToString(AppCUI::Input::Key::Type keyCode, AppCUI::Utils::String *text);
            static bool			            ToString(AppCUI::Input::Key::Type keyCode, AppCUI::Utils::String &text);
            static AppCUI::Input::Key::Type	FromString(const char * stringRepresentation);
            static AppCUI::Input::Key::Type	FromString(AppCUI::Utils::String *text);
            static AppCUI::Input::Key::Type	FromString(AppCUI::Utils::String &text);
        };
    };
    namespace OS
    {
        class EXPORT Clipboard
        {
            Clipboard() = delete;
        public:
            static bool		SetText(const char *text, unsigned int textLen = 0xFFFFFFFF);
            static bool		SetText(const AppCUI::Utils::String &text);
            static bool		GetText(AppCUI::Utils::String &text);
            static bool		Clear();
        };
        class EXPORT IFile 
        {
        public:
            virtual ~IFile();

            // virtual methods
            virtual bool                Read(void* buffer, unsigned int bufferSize, unsigned int & bytesRead);
            virtual bool                Write(const void* buffer, unsigned int bufferSize, unsigned int & bytesWritten);
            virtual unsigned long long  GetSize();
            virtual unsigned long long  GetCurrentPos();
            virtual bool                SetSize(unsigned long long newSize);
            virtual bool                SetCurrentPos(unsigned long long newPosition);
            virtual void                Close();

            // other methods
            bool                        Read(void* buffer, unsigned int bufferSize);
            bool                        Write(const void* buffer, unsigned int bufferSize);
            bool                        Read(unsigned long long offset, void* buffer, unsigned int bufferSize, unsigned int & bytesRead);
            bool                        Write(unsigned long long offset, const void* buffer, unsigned int bufferSize, unsigned int & bytesWritten);
        };

        class EXPORT File : public IFile
        {
            union {
                void*               Handle;
                unsigned long long  u64Value;
                unsigned int        u32Value;
                int                 fid;
            } FileID;
        public:
            File();
            ~File();
            
            /**
             * Opens a file for Read/Write. The file MUST exists. The file pointer will be set at the end of the file.
             * @param[in] filePath is the full path to an existing file.
             */
            bool                OpenWrite(const char * filePath);
            /**
             * Opens a file for Read. The file MUST exists. The file pointer will be set at the begining of the file.
             * @param[in] filePath is the full path to an existing file.
             */
            bool                OpenRead(const char * filePath);
            /**
             * Creates a new file. If the file exists and overwriteExisting parameter is set to true, it will be overwritten.
             * @param[in] filePath is the full path to an existing file.
             * @param[in] overwriteExisting - if set to true and the file exists it will overwrite the file. If set to false and a file exists, will fail to overwrite and the method will return false.
             */
            bool                Create(const char * filePath, bool overwriteExisting = true);

            bool                Read(void* buffer, unsigned int bufferSize, unsigned int & bytesRead) override;
            bool                Write(const void* buffer, unsigned int bufferSize, unsigned int & bytesWritten) override;
            unsigned long long  GetSize() override;
            unsigned long long  GetCurrentPos() override;
            bool                SetSize(unsigned long long newSize) override;
            bool                SetCurrentPos(unsigned long long newPosition) override;
            void                Close() override;
        };

    }
    namespace Console
    {
        enum class Color: unsigned char
        {
            Black       = 0x00,
            DarkBlue    = 0x01,
            DarkGreen   = 0x02,
            Teal        = 0x03,
            DarkRed     = 0x04,
            Magenta     = 0x05,
            Olive       = 0x06,
            Silver      = 0x07,
            Gray        = 0x08,
            Blue        = 0x09,
            Green       = 0x0A,
            Aqua        = 0x0B,
            Red         = 0x0C,
            Pink        = 0x0D,
            Yellow      = 0x0E,
            White       = 0x0F,
            Transparent = 0x10,
        };

        namespace Alignament
        {
            enum Type : unsigned int
            {
                TopLeft = 0,
                Top,
                TopRight,
                Right,
                BottomRight,
                Bottom,
                BottomLeft,
                Left,
                Center
            };
        };
        namespace SpecialChars
        {
            enum Type : unsigned int
            {
                BoxTopLeftCornerDoubleLine = 0,
                BoxTopRightCornerDoubleLine,
                BoxBottomRightCornerDoubleLine,
                BoxBottomLeftCornerDoubleLine,
                BoxHorizontalDoubleLine,
                BoxVerticalDoubleLine,

                BoxTopLeftCornerSingleLine,
                BoxTopRightCornerSingleLine,
                BoxBottomRightCornerSingleLine,
                BoxBottomLeftCornerSingleLine,
                BoxHorizontalSingleLine,
                BoxVerticalSingleLine,

                // arrows
                ArrowUp,
                ArrowDown,
                ArrowLeft,
                ArrowRight,
                ArrowUpDown,
                ArrowLeftRight,

                // Blocks
                Block0,
                Block25,
                Block50,
                Block75,
                Block100,
                BlockUpperHalf,
                BlockLowerHalf,
                BlockLeftHalf,
                BlockRightHalf,

                // symbols
                CircleFilled,
                CircleEmpty,
                CheckMark,

                // always last
                Count
            };
        }
        namespace WriteCharacterBufferFlags
        {
            enum Type : unsigned int
            {
                NONE                = 0,
                SINGLE_LINE         = 0x0000001,
                MULTIPLE_LINES      = 0x0000002,
                OVERWRITE_COLORS    = 0x0000004,
                WRAP_TO_WIDTH       = 0x0000008,
                HIGHLIGHT_HOTKEY    = 0x0000010,
                BUFFER_RANGE        = 0x0000020,
                PROCESS_NEW_LINE    = 0x0000040,
            };
        }

        struct Size
        {
            unsigned int Width, Height;
            inline Size() : Width(0), Height(0) { }
            inline Size(unsigned int width, unsigned int height) : Width(width), Height(height) { }
            inline void Set(unsigned int width, unsigned int height) { Width = width; Height = height; }
        };
        struct Point
        {
            int X, Y;
            inline Point(): X(0), Y(0) {} 
            inline Point(int x, int y) : X(x), Y(y) { }
            inline void Set(int x, int y) { X = x; Y = y; }
        };
        struct ColorPair
        {
            Color Forenground;
            Color Background;
        };
        constexpr ColorPair NoColorPair = ColorPair{ Color::Transparent, Color::Transparent };
        constexpr ColorPair DefaultColorPair = ColorPair{ Color::White, Color::Black };
        struct Character
        {
            union {
                struct {
                    char16_t    Code;
                    ColorPair   Color;
                };
                unsigned int    PackedValue;
            };
        };

        struct WriteCharacterBufferParams
        {
            WriteCharacterBufferFlags::Type Flags;
            ColorPair    Color;
            ColorPair    HotKeyColor;
            unsigned int HotKeyPosition;
            unsigned int Start, End;
            unsigned int Width;
            WriteCharacterBufferParams(): Flags(WriteCharacterBufferFlags::NONE) { }
            WriteCharacterBufferParams(WriteCharacterBufferFlags::Type _flg) : Flags(_flg) { }
        };
        
        class EXPORT Rect
        {
            int X, Y, Width, Height;
        public:
            Rect() : X(0), Y(0), Width(0), Height(0) { }
            bool                 Create(int x, int y, int width, int height, Alignament::Type align);
            void                 Create(int left, int top, int right, int bottom);
            inline bool          Contains(int x, int y) const { return (x >= this->X) && (y >= this->Y) && (x < (this->X + this->Width)) && (y < (this->Y + this->Height)); }
            inline int           GetCenterX() const { return X + (Width >> 1); }
            inline int           GetCenterY() const { return Y + (Height >> 1); }
            inline int           GetX() const { return X; }
            inline int           GetY() const { return Y; }
            inline unsigned int  GetWidth() const { return (unsigned int)Width; }
            inline unsigned int  GetHeight() const { return (unsigned int)Height; }
            inline int           GetLeft() const { return X; }
            inline int           GetTop() const { return Y; }
            inline int           GetRight() const { return X + Width - 1; }
            inline int           GetBotom() const { return X + Height - 1; }
        };

        class EXPORT Clip
        {
        public:
            struct
            {
                int				X, Y, Width, Height;
            } ClipRect;
            struct
            {
                int             X, Y;
            } ScreenPosition;            
            bool			Visible;

        public:
            Clip();
            Clip(const Clip &parent, int relative_x, int relative_y, int width, int height);
            Clip(int x, int y, int width, int height);
            void			Reset();
            void			Set(const Clip &parent, int relativeLeft, int relativeTop, int relativeRight, int relativeBottom);
            void			Set(int x, int y, int width, int height);
        };

        class EXPORT CharacterBuffer
        {
            Character *     Buffer;
            unsigned int    Count;
            unsigned int    Allocated;
            bool            Grow(unsigned int newSize);
        public:
            CharacterBuffer();
            ~CharacterBuffer();

            void Destroy();
            void Clear();

            inline unsigned int     Len() const { return Count; }
            inline Character*       GetBuffer() const { return Buffer; }

            bool Add(const char * text, const ColorPair color = NoColorPair, unsigned int textSize = 0xFFFFFFFF);
            bool Set(const char * text, const ColorPair color = NoColorPair, unsigned int textSize = 0xFFFFFFFF);
            bool SetWithHotKey(const char * text, unsigned int & hotKeyCharacterPosition, const ColorPair color = NoColorPair, unsigned int textSize = 0xFFFFFFFF);
            bool Delete(unsigned int start, unsigned int end);
            bool DeleteChar(unsigned int position);
            bool InsertChar(unsigned short characterCode, unsigned int position, const ColorPair color = NoColorPair);
            bool SetColor(unsigned int start, unsigned int end, const ColorPair color);
            void SetColor(const ColorPair color);
        };
        
        class EXPORT Image
        {
            unsigned int*	Pixels;
            unsigned int	Width;
            unsigned int	Height;
        public:
            Image();
            ~Image();
            bool			        Create(unsigned int width, unsigned int height);
            bool                    SetPixel(unsigned int x, unsigned int y, const Color color);
            bool			        SetPixel(unsigned int x, unsigned int y, unsigned int colorRGB);
            bool			        SetPixel(unsigned int x, unsigned int y, unsigned char Red, unsigned char Green, unsigned char Blue, unsigned char Alpha = 255);
            unsigned int	        GetPixel(unsigned int x, unsigned int y, unsigned int invalidIndexValue = 0);
            bool			        GetPixel(unsigned int x, unsigned int y, unsigned int &color);
            bool                    Clear(unsigned int color);
            bool                    Clear(const Color color);
            inline unsigned int	    GetWidth() const { return Width; }
            inline unsigned int	    GetHeight() const { return Height; }
            inline unsigned int*	GetPixelsBuffer() const { return Pixels; }
        };
        
        class EXPORT Canvas;
        class EXPORT Renderer
        {
        protected:
            Character*          Characters;
            Character**         OffsetRows;
            unsigned int        Width, Height;
            int                 TranslateX, TranslateY;            
            struct {
                int             Left, Top, Right, Bottom;
                bool            Visible;
            } Clip;
            struct {
                unsigned int    X, Y;
                bool            Visible;
            } Cursor;

        
            Renderer();
            ~Renderer();

            void            _Destroy();
            bool            _ClearEntireSurface(int character, const ColorPair color);
            bool            _WriteCharacterBuffer_SingleLine(int x, int y, const CharacterBuffer & cb, const WriteCharacterBufferParams& params, unsigned int start, unsigned int end);
            bool            _WriteCharacterBuffer_MultiLine_WithWidth(int x, int y, const CharacterBuffer & cb, const WriteCharacterBufferParams& params, unsigned int start, unsigned int end);
            bool            _WriteCharacterBuffer_MultiLine_ProcessNewLine(int x, int y, const CharacterBuffer & cb, const WriteCharacterBufferParams& params, unsigned int start, unsigned int end);
        public:
            
            // Horizontal lines
            bool    DrawHorizontalLine(int left, int y, int right, int charCode, const ColorPair color);
            bool    DrawHorizontalLineSize(int x, int y, unsigned int size, int charCode, const ColorPair color);
            bool    DrawHorizontalLineWithSpecialChar(int left, int y, int right, SpecialChars::Type charID, const ColorPair color);
            
            // Vertical lines
            bool    DrawVerticalLine(int x, int top, int bottom, int charCode, const ColorPair color);
            bool    DrawVerticalLineSize(int x, int y, unsigned int size, int charCode, const ColorPair color);
            bool    DrawVerticalLineWithSpecialChar(int x, int top, int bottom, SpecialChars::Type charID, const ColorPair color);

            // Rectangle
            bool    FillRect(int left, int top, int right, int bottom, int charCode, const ColorPair color);  
            bool    FillRectSize(int x, int y, unsigned int width, unsigned int height, int charCode, const ColorPair color);
            bool    DrawRect(int left, int top, int right, int bottom, const ColorPair color, bool doubleLine);
            bool    DrawRectSize(int x, int y, unsigned int width, unsigned int height, const ColorPair color, bool doubleLine);

            // Texts & Characters
            bool    WriteSingleLineText(int x, int y, const char * text, const ColorPair color, int textSize = -1);
            bool    WriteSingleLineTextWithHotKey(int x, int y, const char * text, const ColorPair color, const ColorPair hotKeyColor, int textSize = -1);
            bool    WriteMultiLineText(int x, int y, const char * text, const ColorPair color, int textSize = -1);
            bool    WriteMultiLineTextWithHotKey(int x, int y, const char * text, const ColorPair color, const ColorPair hotKeyColor, int textSize = -1);
            bool    WriteCharacter(int x, int y, int charCode, const ColorPair color);
            bool    WriteSpecialCharacter(int x, int y, SpecialChars::Type charID, const ColorPair color);
            bool    WriteCharacterBuffer(int x, int y, const AppCUI::Console::CharacterBuffer & cb, const AppCUI::Console::WriteCharacterBufferParams& params);

            // Canvas & Images
            bool    DrawCanvas(int x, int y, const Canvas& canvas);

            bool    ClearWithSpecialChar(SpecialChars::Type charID, const ColorPair color);
            bool    Clear(int charCode, const ColorPair color);
            

            // Cursor
            void    HideCursor();
            bool    SetCursor(int x, int y);


        };

        class EXPORT Canvas: public Renderer
        {
        public:
            Canvas();
            ~Canvas();
            bool    Create(unsigned int width, unsigned int height, int fillCharacter = ' ', const ColorPair color = DefaultColorPair);
            bool    Resize(unsigned int width, unsigned int height, int fillCharacter = ' ', const ColorPair color = DefaultColorPair);

            // Clipping & Translate
            void    SetClip(const AppCUI::Console::Clip & clip);
            void    ResetClip();
            void    SetTranslate(int offX, int offY);

            bool    SetSize(unsigned int width, unsigned int height);
            void    Reset();
            void    Update();
            void    DarkenScreen();
            bool    ClearEntireSurface(int character, const ColorPair color);

            // inlines
            inline unsigned int GetWidth() const            { return this->Width; }
            inline unsigned int GetHeight() const           { return this->Height; }
            inline bool         GetCursorVisibility() const { return this->Cursor.Visible; }
            inline unsigned int GetCursorX() const          { return this->Cursor.X; }
            inline unsigned int GetCursorY() const          { return this->Cursor.Y; }
            inline Character*   GetCharactersBuffer() const { return this->Characters; }
        };

        

    };
    namespace Controls
    {
        namespace Event
        {
            enum Type: unsigned int
            {
                EVENT_WINDOW_CLOSE,
                EVENT_WINDOW_ACCEPT,
                EVENT_BUTTON_CLICKED,
                EVENT_CHECKED_STATUS_CHANGED,
                EVENT_TEXT_CHANGED,
                EVENT_TEXTFIELD_VALIDATE,
                EVENT_TAB_CHANGED,
                EVENT_TERMINATE_APPLICATION,
                EVENT_COMMAND,
                EVENT_CUSTOM,
            };
        }
        class EXPORT Control;
        class EXPORT TextField;
        namespace Handlers
        {
            typedef void(*AfterResizeHandler) (AppCUI::Controls::Control *control, int newWidth, int newHeight, void *Context);
            typedef bool(*BeforeResizeHandler) (AppCUI::Controls::Control *control, int newWidth, int newHeight, void *Context);
            typedef void(*AfterMoveHandler) (AppCUI::Controls::Control *control, int newX, int newY, void *Context);
            typedef bool(*UpdateCommandBarHandler)(AppCUI::Controls::Control *control, void* Context);
            typedef bool(*KeyEventHandler)(AppCUI::Controls::Control *control, int KeyCode, int AsciiCode, void *Context);
            typedef void(*PaintHandler)(AppCUI::Controls::Control *control, void *Context);
            typedef void(*OnFocusHandler)(AppCUI::Controls::Control *control, void *Context);
            typedef bool(*EventHandler)(AppCUI::Controls::Control *control, const void* sender, AppCUI::Controls::Event::Type eventType, int controlID, void *Context);
            typedef void(*MousePressedHandler) (AppCUI::Controls::Control *control, int x, int y, int buttonState, void *Context);
            typedef void(*MouseReleasedHandler) (AppCUI::Controls::Control *control, int x, int y, int buttonState, void *Context);
            typedef void(*TextFieldSyntaxHighlightHandler) (AppCUI::Controls::TextField * textField, AppCUI::Console::Character* characters, unsigned int charactersCount, void* Context);
        }

        class EXPORT Control
        {
        public:
            void*			Context;
        protected:
            bool			IsMouseInControl(int x, int y);
            bool			SetMargins(int left, int top, int right, int bottom);
            bool			Init(Control *parent, const char *text, const char * layout, bool computeHotKey = false);
        public:
            Control();
            bool			AddControl(Control* control);
            bool			RemoveControl(Control * control);
            bool			RemoveControl(unsigned int index);

            bool			IsInitialized();

            // coordonates
            int				GetX();
            int				GetY();
            int				GetWidth();
            int				GetHeight();
            void            GetSize(AppCUI::Console::Size & size);
            void            GetClientSize(AppCUI::Console::Size & size);
            void			MoveTo(int newX, int newY);
            bool			Resize(int newWidth, int newHeight);
            void			RecomputeLayout();

            // groups
            int				GetGroup();
            void			SetGroup(int newGroupID);
            void			ClearGroup();

            // hot key
            bool			SetHotKey(char hotKey);
            Input::Key::Type GetHotKey();
            void			ClearHotKey();

            // status
            void			SetEnabled(bool value);
            void			SetVisible(bool value);
            void			SetChecked(bool value);
            bool			IsEnabled();
            bool			IsVisible();
            bool			IsChecked();
            bool			HasFocus();
            bool            IsMouseOver();

            // childern and parent
            Control*		GetParent();
            Control**		GetChildrenList();
            Control*		GetChild(unsigned int index);
            unsigned int	GetChildernCount();
            bool			GetChildIndex(Control *control, unsigned int &index);

            // Events
            void			RaiseEvent(Event::Type eventType);
            void			RaiseEvent(Event::Type eventType, int ID);
            // focus
            bool			SetFocus();
            void			SetControlID(int newID);

            // Text
            bool			SetText(const char * text, bool updateHotKey = false);
            bool			SetText(AppCUI::Utils::String *text, bool updateHotKey = false);
            bool			SetText(AppCUI::Utils::String &text, bool updateHotKey = false);
            //const char*		GetText();
            //bool			GetText(AppCUI::Utils::String *text);
            //bool			GetText(AppCUI::Utils::String &text);

            // handlere
            void			SetOnBeforeResizeHandler(Handlers::BeforeResizeHandler handler, void *Context = nullptr);
            void			SetOnAfterResizeHandler(Handlers::AfterResizeHandler handler, void *Context = nullptr);
            void			SetOnAfterMoveHandler(Handlers::AfterMoveHandler handler, void *Context = nullptr);
            void			SetOnUpdateCommandBarHandler(Handlers::UpdateCommandBarHandler handler, void* Context = nullptr);
            void			SetOnKeyEventHandler(Handlers::KeyEventHandler handler, void* Context = nullptr);
            void			SetPaintHandler(Handlers::PaintHandler handler, void* Context = nullptr);
            void			SetOnFocusHandler(Handlers::OnFocusHandler handler, void* Context = nullptr);
            void			SetOnLoseFocusHandler(Handlers::OnFocusHandler handler, void* Context = nullptr);
            void			SetEventHandler(Handlers::EventHandler handler, void* Context = nullptr);
            void			SetMousePressedHandler(Handlers::MousePressedHandler handler, void* Context = nullptr);
            void			SetMouseReleasedHandler(Handlers::MouseReleasedHandler handler, void* Context = nullptr);
            void			SetMouseHandler(Handlers::MousePressedHandler mousePressedHandler, Handlers::MouseReleasedHandler mouseReleasedHandler, void *Context = nullptr);

            // paint
            virtual void	Paint(Console::Renderer & renderer);

            // Evenimente
            virtual bool	OnKeyEvent(AppCUI::Input::Key::Type keyCode, char AsciiCode);
            virtual void	OnHotKey();
            virtual void	OnFocus();
            virtual void	OnLoseFocus();

            virtual void	OnMousePressed(int x, int y, int Button);
            virtual void	OnMouseReleased(int x, int y, int Button);
            virtual bool	OnMouseDrag(int x, int y, int Button);
            
            virtual bool    OnMouseEnter();
            virtual bool    OnMouseOver(int x, int y);
            virtual bool    OnMouseLeave();

            virtual void	OnMouseWheel(int direction);
            virtual bool	OnEvent(const void* sender, Event::Type eventType, int controlID);
            virtual bool	OnUpdateCommandBar(AppCUI::Application::CommandBar & commandBar);

            virtual bool	OnBeforeResize(int newWidth, int newHeight);
            virtual void	OnAfterResize(int newWidth, int newHeight);
            virtual bool	OnBeforeAddControl(Control *ctrl);
            virtual void	OnAfterAddControl(Control *ctrl);
            virtual bool	OnBeforeSetText(const char * text);
            virtual void	OnAfterSetText(const char * text);

            virtual ~Control();

        };

        namespace WindowFlags
        {
            enum Type : unsigned int
            {
                NONE = 0,
                SIZEABLE = 0x000100,
                NOTIFYBOX = 0x000200,
                ERRORBOX = 0x000400,
                WARNINGBOX = 0x000800,
                NOCLOSEBUTTON = 0x001000,
                FIXED = 0x004000,
                CENTERED = 0x008000,
                MAXIMIZED = 0x010000
            };
        }
        class EXPORT Window : public Control 
        {
        public:
            bool	Create(const char* text, const char * layout, WindowFlags::Type windowsFlags = WindowFlags::NONE);
            void	Paint(Console::Renderer & renderer) override;
            void	OnMousePressed(int x, int y, int Button) override;
            void	OnMouseReleased(int x, int y, int Button) override;
            bool	OnMouseDrag(int x, int y, int Button) override;
            bool    OnMouseOver(int x, int y) override;
            bool    OnMouseLeave() override;
            int		Show();
            int		GetDialogResult();
            bool	MaximizeRestore();
            bool	OnBeforeResize(int newWidth, int newHeight) override;
            void	OnAfterResize(int newWidth, int newHeight) override;
            bool	CenterScreen();
            bool	OnKeyEvent(AppCUI::Input::Key::Type keyCode, char AsciiCode) override;
            bool	Exit(int dialogResult);
            bool	IsWindowInResizeMode();

            virtual ~Window();
        };
        class EXPORT Label : public Control
        {
        public:
            bool	Create(Control *parent, const char * text, const char * layout);
            void	Paint(Console::Renderer & renderer) override;
        };
        class EXPORT Button : public Control
        {
        public:
            bool	Create(Control *parent, const char * text, const char * layout, int controlID = 0);
            void	OnMousePressed(int x, int y, int Button) override;
            void	OnMouseReleased(int x, int y, int Button) override;
            bool	OnMouseDrag(int x, int y, int Button) override;
            void	Paint(Console::Renderer & renderer) override;
            bool	OnKeyEvent(AppCUI::Input::Key::Type keyCode, char AsciiCode) override;
            void	OnHotKey() override;
            bool    OnMouseEnter() override;
            bool    OnMouseLeave() override;
        };
        class EXPORT CheckBox : public Control
        {
        public:
            bool	Create(Control *parent, const char * text, const char * layout, int controlID = 0);
            void	OnMouseReleased(int x, int y, int Button) override;
            void	Paint(Console::Renderer & renderer) override;
            bool	OnKeyEvent(AppCUI::Input::Key::Type keyCode, char AsciiCode) override;
            void	OnHotKey() override;
            bool    OnMouseEnter() override;
            bool    OnMouseLeave() override;
        };
        class EXPORT RadioBox : public Control
        {
        public:
            bool	Create(Control *parent, const char * text, const char * layout, int groupID, int controlID = 0);
            void	OnMouseReleased(int x, int y, int Button) override;
            void	Paint(Console::Renderer & renderer) override;
            bool	OnKeyEvent(AppCUI::Input::Key::Type keyCode, char AsciiCode) override;
            void	OnHotKey() override;
            bool    OnMouseEnter() override;
            bool    OnMouseLeave() override;
        };
        class EXPORT Splitter : public Control
        {
        public:
            bool	Create(Control *parent, const char * layout, bool vertical);
            void	Paint(Console::Renderer & renderer) override;
            bool	OnKeyEvent(AppCUI::Input::Key::Type keyCode, char AsciiCode) override;
            bool	SetSecondPanelSize(int newSize);
            bool	HideSecondPanel();
            bool	MaximizeSecondPanel();
            void	OnAfterResize(int newWidth, int newHeight) override;
            void	OnFocus() override;
            bool	OnBeforeAddControl(Control *ctrl) override;
            void	OnAfterAddControl(Control *ctrl) override;
            void	OnMousePressed(int x, int y, int Button) override;
            void	OnMouseReleased(int x, int y, int Button) override;
            bool	OnMouseDrag(int x, int y, int Button) override;
            bool    OnMouseEnter() override;
            bool    OnMouseLeave() override;
            int		GetSplitterPosition();
            virtual ~Splitter();
        };
        class EXPORT Panel : public Control
        {
        public:
            bool	Create(Control *parent, const char * text, const char * layout);
            bool	Create(Control *parent, const char * layout);
            void	Paint(Console::Renderer & renderer) override;
        };
        namespace TextFieldFlags {
            enum Type : unsigned int {
                NONE                = 0,
                PROCESS_ENTER       = 0x000100,
                READONLY_TEXT       = 0x000200,
                SYNTAX_HIGHLIGHTING = 0x000400,
            };
        }
        class EXPORT TextField : public Control
        {
        public:
            bool	Create(Control *parent, const char * text, const char * layout, TextFieldFlags::Type flags = TextFieldFlags::NONE, Handlers::TextFieldSyntaxHighlightHandler handler = nullptr, void* Context = nullptr);
            bool	OnKeyEvent(AppCUI::Input::Key::Type keyCode, char AsciiCode) override;
            void	OnAfterSetText(const char *text) override;
            void	Paint(Console::Renderer & renderer) override;
            void	OnFocus() override;
            bool    OnMouseEnter() override;
            bool    OnMouseLeave() override;

            void	SelectAll();
            void	ClearSelection();

            virtual ~TextField();
        };

        namespace TabFlags
        {
            enum Type : unsigned int
            {
                TOP_TABS                = 0x000000, // default mode
                BOTTOM_TABS             = 0x000100,
                LEFT_TABS               = 0x000200,
                LIST                    = 0x000300,
                TRANSPARENT_BACKGROUND  = 0x001000,
                HAS_TABBAR              = 0x002000,
            };
        }
        class EXPORT TabPage : public Control
        {
        public:
            bool	Create(Control *parent, const char * text);
            bool	OnBeforeResize(int newWidth, int newHeight);
        };
        class EXPORT Tab : public Control
        {
        public:
            bool	Create(Control *parent, const char * layout, TabFlags::Type flags = TabFlags::TOP_TABS, unsigned int tabPageSize = 16);
            bool	SetCurrentTabPage(unsigned int index);
            bool	SetTabPageTitleSize(unsigned int newSize);
            bool	SetTabPageName(unsigned int index, const char* name);
            bool	SetTabPageName(unsigned int index, AppCUI::Utils::String* name);
            bool	SetTabPageName(unsigned int index, AppCUI::Utils::String& name);
            void	OnAfterResize(int newWidth, int newHeight) override;
            void	OnFocus() override;
            void	OnMouseReleased(int x, int y, int buttonState) override;
            bool    OnMouseLeave() override;
            bool    OnMouseOver(int x, int y) override;
            bool	OnKeyEvent(AppCUI::Input::Key::Type keyCode, char AsciiCode) override;
            void	OnAfterAddControl(Control *ctrl) override;
            void	Paint(Console::Renderer & renderer) override;
            Control* GetCurrentTab();
        };
        class EXPORT UserControl : public Control
        {
        public:
            bool    Create(Control *parent, const char * text, const char * layout);
            bool    Create(Control *parent, const char * layout);
        };

        namespace DialogResult
        {
            enum Type : int
            {
                RESULT_NONE = 0,
                RESULT_OK = 1,
                RESULT_CANCEL = 2,
                RESULT_YES = 3,
                RESULT_NO = 4,
            };
        }
        class EXPORT MessageBox
        {
            MessageBox() = delete;
        public:
            static void                 ShowError(const char * title, const char * message);
            static void                 ShowNotification(const char *title, const char *message);
            static void                 ShowWarning(const char *title, const char *message);
            static DialogResult::Type   ShowOkCancel(const char *title, const char *message);
            static DialogResult::Type   ShowYesNoCancel(const char *title, const char *message);
        };

    };
    namespace Application
    {
        namespace Flags
        {
            enum Type : unsigned int
            {
                NONE            = 0,
                HAS_COMMANDBAR  = 0x00000001,
            };
        }

        class EXPORT CommandBar
        {
            void* Controller;
        public:
            CommandBar();
            void  Init(void* controller);
            bool  SetCommand(AppCUI::Input::Key::Type keyCode, const char* Name, int CommandID);
        };

        struct Config
        {
            struct {
                int             DesktopFillCharacterCode;
                Console::ColorPair    Color;
            } Desktop;
            struct {
                Console::ColorPair BackgroundColor;
                Console::ColorPair ShiftKeysColor;
                struct {
                    Console::ColorPair KeyColor;
                    Console::ColorPair NameColor;
                } Normal, Hover, Pressed;
            } CommandBar;
            struct {
                Console::ColorPair    ActiveColor;
                Console::ColorPair    InactiveColor;
                Console::ColorPair    TitleActiveColor;
                Console::ColorPair    TitleInactiveColor;
                Console::ColorPair    ControlButtonColor;
                Console::ColorPair    ControlButtonInactiveColor;
                Console::ColorPair    ControlButtonHoverColor;
                Console::ColorPair    ControlButtonPressedColor;
            } Window, DialogError, DialogNotify, DialogWarning;
            struct {
                Console::ColorPair    NormalColor;
                Console::ColorPair    HotKeyColor;
            } Label;
            struct {
                struct {
                    Console::ColorPair TextColor, HotKeyColor;
                } Normal, Focused, Inactive, Hover;
            } Button;
            struct {
                struct {
                    Console::ColorPair TextColor, HotKeyColor,StateSymbolColor;
                } Normal, Focused, Inactive, Hover;
            } StateControl;
            struct {
                Console::ColorPair NormalColor, ClickColor, HoverColor;
            } Splitter;
            struct {
                Console::ColorPair NormalColor, TextColor;
            } Panel;
            struct {
                Console::ColorPair NormalColor, FocusColor, InactiveColor, HoverColor, SelectionColor;
            } TextField;
            struct {
                Console::ColorPair PageColor, TabBarColor, HoverColor, PageHotKeyColor, TabBarHotKeyColor, HoverHotKeyColor;
                Console::ColorPair ListSelectedPageColor, ListSelectedPageHotKey;
            } Tab;
            void SetDarkTheme();
        };
        typedef             void(*EventHandler)(const void* sender, AppCUI::Controls::Event::Type eventType, int controlID);

        EXPORT Config*      GetAppConfig();
        EXPORT bool         Init(Application::Flags::Type flags = Application::Flags::NONE, EventHandler eventCallback = nullptr);
        EXPORT bool         Run();
        EXPORT bool         AddWindow(AppCUI::Controls::Window * wnd);
        EXPORT bool         GetApplicationSize(AppCUI::Console::Size & size);
        EXPORT bool         GetDesktopSize(AppCUI::Console::Size & size);
        EXPORT void         Repaint();
        EXPORT void         RecomputeControlsLayout();
        EXPORT void         RaiseEvent(AppCUI::Controls::Control *control, AppCUI::Controls::Control *sourceControl, AppCUI::Controls::Event::Type eventType, int controlID);
        EXPORT void         Close();
    };
}

// inline OR operator for flags
inline constexpr AppCUI::Application::Flags::Type operator|(AppCUI::Application::Flags::Type f1, AppCUI::Application::Flags::Type f2)
{
    return static_cast<AppCUI::Application::Flags::Type>(static_cast<unsigned int>(f1) | static_cast<unsigned int>(f2));
} 
inline constexpr AppCUI::Input::Key::Type operator|(AppCUI::Input::Key::Type f1, AppCUI::Input::Key::Type f2)
{
    return static_cast<AppCUI::Input::Key::Type>(static_cast<unsigned int>(f1) | static_cast<unsigned int>(f2));
}
inline constexpr void operator|=(AppCUI::Input::Key::Type & f1, AppCUI::Input::Key::Type f2)
{
    f1 = static_cast<AppCUI::Input::Key::Type>(static_cast<unsigned int>(f1) | static_cast<unsigned int>(f2));
}
inline constexpr AppCUI::Controls::WindowFlags::Type operator|(AppCUI::Controls::WindowFlags::Type f1, AppCUI::Controls::WindowFlags::Type f2)
{
    return static_cast<AppCUI::Controls::WindowFlags::Type>(static_cast<unsigned int>(f1) | static_cast<unsigned int>(f2));
}
inline constexpr AppCUI::Console::WriteCharacterBufferFlags::Type operator|(AppCUI::Console::WriteCharacterBufferFlags::Type f1, AppCUI::Console::WriteCharacterBufferFlags::Type f2)
{
    return static_cast<AppCUI::Console::WriteCharacterBufferFlags::Type>(static_cast<unsigned int>(f1) | static_cast<unsigned int>(f2));
}
inline constexpr void operator|=(AppCUI::Console::WriteCharacterBufferFlags::Type & f1, AppCUI::Console::WriteCharacterBufferFlags::Type f2)
{
    f1 = static_cast<AppCUI::Console::WriteCharacterBufferFlags::Type>(static_cast<unsigned int>(f1) | static_cast<unsigned int>(f2));
}
inline constexpr AppCUI::Controls::TabFlags::Type operator|(AppCUI::Controls::TabFlags::Type f1, AppCUI::Controls::TabFlags::Type f2)
{
    return static_cast<AppCUI::Controls::TabFlags::Type>(static_cast<unsigned int>(f1) | static_cast<unsigned int>(f2));
}

#endif
