#ifndef __APPCUI_MAIN_HEADER__
#define __APPCUI_MAIN_HEADER__

#ifdef BUILD_AS_DYNAMIC_LIB
#   ifdef BUILD_FOR_WINDOWS
#       define EXPORT __declspec(dllexport)
#   else
#       define EXPORT
#   endif
#else
#   define EXPORT
#endif

#if _DEBUG
#   define ENABLE_LOGGING
#endif

#ifdef ENABLE_LOGGING
#   define CHECK(c,returnValue,format,...) { \
        if (!(c)) { AppCUI::Log::Report(AppCUI::Log::Severity::Error, __FILE__, __FUNCTION__, #c, __LINE__, format, ##__VA_ARGS__); return (returnValue); } \
    }
#   define CHECKBK(c,format,...) { \
        if (!(c)) { AppCUI::Log::Report(AppCUI::Log::Severity::Error, __FILE__, __FUNCTION__, #c, __LINE__, format, ##__VA_ARGS__); break; } \
    }
#   define RETURNERROR(returnValue,format,...) { AppCUI::Log::Report(AppCUI::Log::Severity::Error, __FILE__, __FUNCTION__, "", __LINE__, format, ##__VA_ARGS__); return (returnValue); }
#   define NOT_IMPLEMENTED(returnValue) { AppCUI::Log::Report(AppCUI::Log::Severity::Warning, __FILE__, __FUNCTION__, "", __LINE__, "Current function/method is not implemented under current OS"); return (returnValue); }
#   define LOG_INFO(format,...) AppCUI::Log::Report(AppCUI::Log::Severity::Information, __FILE__, __FUNCTION__, "", __LINE__, format, ##__VA_ARGS__);
#   define LOG_WARNING(format,...) AppCUI::Log::Report(AppCUI::Log::Severity::Warning, __FILE__, __FUNCTION__, "", __LINE__, format, ##__VA_ARGS__);
#   define LOG_ERROR(format,...) AppCUI::Log::Report(AppCUI::Log::Severity::Error, __FILE__, __FUNCTION__, "", __LINE__, format, ##__VA_ARGS__);
#else
#   define CHECK(c,returnValue,format,...) { if (!(c)) return (returnValue); }
#   define RETURNERROR(returnValue,format,...) { return (returnValue);}
#   define CHECKBK(c,format,...) if (!(c)) break;
#   define NOT_IMPLEMENTED(returnValue) { return (returnValue); }
#   define LOG_INFO(format,...)
#   define LOG_WARNING(format,...) 
#   define LOG_ERROR(format,...)
#endif

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
        class EXPORT Array32
        {
            unsigned int*   Data;
            unsigned int    Count, Allocated;

            bool				 Grow(unsigned int newSize);
        public:
            Array32();
            ~Array32();
            void                 Destroy();

            bool			     Create(unsigned int allocatedCount = 64);
            bool			     Create(unsigned int * vector, unsigned int vectorSize, unsigned int elementsCount = 0);
            bool			     Create(int * vector, unsigned int vectorSize, unsigned int elementsCount = 0);

            inline unsigned int* GetUInt32Array() const { return Data; }
            inline int*          GetInt32Array() const { return (int*)Data; }
            inline unsigned int	 Len() const { return Count; }
            inline unsigned int	 GetAllocatedSize() const { return Allocated & 0x7FFFFFFF; }

            inline void          Clear() { Count = 0; }
            bool                 Reserve(unsigned int newSize);
            bool                 Resize(unsigned int newSize);
            bool                 Push(unsigned int value);
            bool                 Push(int value);
            bool                 Get(unsigned int index, unsigned int & value);
            bool                 Get(unsigned int index, int & value);

            bool                 Sort(int(*compare)(int elem1, int elem2, void* Context), bool ascendent, void * Context = nullptr);
            bool                 Sort(int(*compare)(unsigned int elem1, unsigned int elem2, void* Context), bool ascendent, void * Context = nullptr);
        };
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
            static bool         Contains    (const char *sir, const char *textToFind, bool ignoreCase = false);
            static int          Compare     (const char *sir1, const char * sir2, bool ignoreCase = false);
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
            bool			    Contains(const char *text, bool ignoreCase = false) const;
            bool			    Contains(const String &ss, bool ignoreCase = false) const;
            int                 CompareWith(const char * text, bool ignoreCase = false) const;

            void                ConvertToInternalNewLineFormat();

            
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
            virtual bool                ReadBuffer(void* buffer, unsigned int bufferSize, unsigned int & bytesRead);
            virtual bool                WriteBuffer(const void* buffer, unsigned int bufferSize, unsigned int & bytesWritten);
            virtual unsigned long long  GetSize();
            virtual unsigned long long  GetCurrentPos();
            virtual bool                SetSize(unsigned long long newSize);
            virtual bool                SetCurrentPos(unsigned long long newPosition);
            virtual void                Close();

            // other methods
            bool                        Read(void* buffer, unsigned int bufferSize, unsigned int & bytesRead);
            bool                        Write(const void* buffer, unsigned int bufferSize, unsigned int & bytesWritten);
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

            bool                ReadBuffer(void* buffer, unsigned int bufferSize, unsigned int & bytesRead) override;
            bool                WriteBuffer(const void* buffer, unsigned int bufferSize, unsigned int & bytesWritten) override;
            unsigned long long  GetSize() override;
            unsigned long long  GetCurrentPos() override;
            bool                SetSize(unsigned long long newSize) override;
            bool                SetCurrentPos(unsigned long long newPosition) override;
            void                Close() override;
        };

        typedef bool(*EnumerateFilesCallback) (const char *fullPath, const char* Name, unsigned long long size, bool Folder, void *Context);
        typedef bool(*CopyFileCallback)       (const char * source, const char* destination, unsigned long long copiedBytesSoFar, unsigned long long totalFileSize, void* Context);
        typedef bool(*DeleteFileCallback)     (const char * fileName, void *Context);


        namespace FileSystem
        {
            namespace Path
            {
                EXPORT bool		Join(AppCUI::Utils::String& path, const char * name);
                EXPORT bool		CopyDirectoryName(const char * path, AppCUI::Utils::String& result);
                EXPORT bool		IsRootPath(const char *path);
            };
            EXPORT bool			EnumerateFiles(AppCUI::Utils::String &path, bool recursive, EnumerateFilesCallback callback, void *Context, bool callFolderCallbackAfterProcessingAllOfItsFiles = false);
            EXPORT bool			EnumerateFiles(const char* path, bool recursive, EnumerateFilesCallback callback, void *Context, bool callFolderCallbackAfterProcessingAllOfItsFiles = false);
            EXPORT bool			FileExists(const char *path);
            EXPORT bool			DirectoryExists(const char *path);
            EXPORT bool			GetCurrentDir(AppCUI::Utils::String &path);
            EXPORT bool			CreateFolder(const char *name);
            EXPORT bool			DeleteFile(const char *name, bool failIfFileIsMissing = false);

            EXPORT bool         DeleteEmptyFoler(const char * name);
            EXPORT bool			DeleteFolder(AppCUI::Utils::String &name, bool failIfFileIsMissing = false, DeleteFileCallback callback = nullptr, void *Context = nullptr);
            EXPORT bool			DeleteFolder(const char *name, bool failIfFileIsMissing = false, DeleteFileCallback callback = nullptr, void *Context = nullptr);

            EXPORT bool			CopyFile(const char* source, const char* destination, CopyFileCallback callback = nullptr, void* Context = nullptr);
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
        enum class SpecialChars : unsigned int
        {
            BoxTopLeftCornerDoubleLine = 0,
            BoxTopRightCornerDoubleLine,
            BoxBottomRightCornerDoubleLine,
            BoxBottomLeftCornerDoubleLine,
            BoxHorizontalDoubleLine,
            BoxVerticalDoubleLine,
            BoxCrossDoubleLine,

            BoxTopLeftCornerSingleLine,
            BoxTopRightCornerSingleLine,
            BoxBottomRightCornerSingleLine,
            BoxBottomLeftCornerSingleLine,
            BoxHorizontalSingleLine,
            BoxVerticalSingleLine,
            BoxCrossSingleLine,

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
            BlockCentered,

            // Trangles
            TriangleUp,
            TriangleDown,
            TriangleLeft,
            TriangleRight,

            // symbols
            CircleFilled,
            CircleEmpty,
            CheckMark,

            // always last
            Count
        };
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
        enum class TextAlignament : unsigned int
        {
            Left        = 0x00,
            Center      = 0x01,
            Right       = 0x02,
            Padding     = 0x04
        };


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
            bool SetWithNewLines(const char * text, const ColorPair color = NoColorPair, unsigned int textSize = 0xFFFFFFFF);
            bool SetWithHotKey(const char * text, unsigned int & hotKeyCharacterPosition, const ColorPair color = NoColorPair, unsigned int textSize = 0xFFFFFFFF);
            bool Delete(unsigned int start, unsigned int end);
            bool DeleteChar(unsigned int position);
            bool Insert(const char * text, unsigned int position, const ColorPair color = NoColorPair, unsigned int textSize = 0xFFFFFFFF);
            bool Insert(const AppCUI::Utils::String& text, unsigned int position, const ColorPair color = NoColorPair);
            bool InsertChar(unsigned short characterCode, unsigned int position, const ColorPair color = NoColorPair);
            bool SetColor(unsigned int start, unsigned int end, const ColorPair color);
            void SetColor(const ColorPair color);
            bool CopyString(Utils::String & text, unsigned int start, unsigned int end);
            bool CopyString(Utils::String & text);
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
            } Clip, ClipCopy;
            bool                ClipHasBeenCopied;
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
            bool    DrawHorizontalLineWithSpecialChar(int left, int y, int right, SpecialChars charID, const ColorPair color);
            
            // Vertical lines
            bool    DrawVerticalLine(int x, int top, int bottom, int charCode, const ColorPair color);
            bool    DrawVerticalLineSize(int x, int y, unsigned int size, int charCode, const ColorPair color);
            bool    DrawVerticalLineWithSpecialChar(int x, int top, int bottom, SpecialChars charID, const ColorPair color);

            // Rectangle
            bool    FillRect(int left, int top, int right, int bottom, int charCode, const ColorPair color);  
            bool    FillRectSize(int x, int y, unsigned int width, unsigned int height, int charCode, const ColorPair color);
            bool    DrawRect(int left, int top, int right, int bottom, const ColorPair color, bool doubleLine);
            bool    DrawRectSize(int x, int y, unsigned int width, unsigned int height, const ColorPair color, bool doubleLine);

            // Texts
            bool    WriteSingleLineText(int x, int y, const char * text, const ColorPair color, int textSize = -1);
            bool    WriteSingleLineText(int x, int y, const char * text, unsigned int width, const ColorPair color, TextAlignament align = TextAlignament::Left, int textSize = -1);
            bool    WriteSingleLineTextWithHotKey(int x, int y, const char * text, const ColorPair color, const ColorPair hotKeyColor, int textSize = -1);
            bool    WriteSingleLineTextWithHotKey(int x, int y, const char * text, unsigned int width, const ColorPair color, const ColorPair hotKeyColor, unsigned int hotKeyOffset, TextAlignament align = TextAlignament::Left, int textSize = -1);
            bool    WriteMultiLineText(int x, int y, const char * text, const ColorPair color, int textSize = -1);
            bool    WriteMultiLineTextWithHotKey(int x, int y, const char * text, const ColorPair color, const ColorPair hotKeyColor, int textSize = -1);

            // Characters
            bool    WriteCharacter(int x, int y, int charCode, const ColorPair color);
            bool    WriteSpecialCharacter(int x, int y, SpecialChars charID, const ColorPair color);

            // Character Buffer
            bool    WriteCharacterBuffer(int x, int y, const AppCUI::Console::CharacterBuffer & cb, const AppCUI::Console::WriteCharacterBufferParams& params);
            bool    WriteCharacterBuffer(int x, int y, unsigned int width, const AppCUI::Console::CharacterBuffer & cb, const ColorPair textColor, TextAlignament align);
            bool    WriteCharacterBuffer(int x, int y, unsigned int width, const AppCUI::Console::CharacterBuffer & cb, const ColorPair textColor, const ColorPair hotKeyColor, unsigned int hotKeyOffset, TextAlignament align);

            // Canvas & Images
            bool    DrawCanvas(int x, int y, const Canvas& canvas, const ColorPair overwriteColor = NoColorPair);            


            // Clear
            bool    ClearWithSpecialChar(SpecialChars charID, const ColorPair color);
            bool    Clear(int charCode, const ColorPair color);
            
            // Clipping
            bool    SetClipMargins(int leftMargin, int topMargin, int rightMargin, int bottomMargin);
            bool    ResetClip();

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
            void    SetAbsoluteClip(const AppCUI::Console::Clip & clip);
            void    ExtendAbsoluteCliptToRightBottomCorner();
            void    ClearClip();
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
                EVENT_LISTVIEW_CURRENTITEM_CHANGED,
                EVENT_LISTVIEW_SELECTION_CHANGED,
                EVENT_LISTVIEW_ITEM_CHECKED,
                EVENT_LISTVIEW_ITEM_CLICKED,
                EVENT_TERMINATE_APPLICATION,
                EVENT_COMMAND,
                EVENT_CUSTOM,
            };
        }
        typedef unsigned int ItemHandle;
        constexpr ItemHandle InvalidItemHandle = 0xFFFFFFFF;
        class EXPORT Control;
        class EXPORT TextField;
        class EXPORT ListView;
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
            typedef void(*SyntaxHighlightHandler) (AppCUI::Controls::Control * control, AppCUI::Console::Character* characters, unsigned int charactersCount, void* Context);
            typedef void(*TextASyntaxHighlightHandler)(const char *ptrLine, unsigned char *ptrColors, unsigned int LineSize, unsigned int ColorVectSize, void *Context);
            typedef int (*ListViewItemComparer)(AppCUI::Controls::ListView *control, ItemHandle item1, ItemHandle item2, unsigned int columnIndex, void *Context);

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
            
            // Scroll bars
            void            UpdateHScrollBar(unsigned long long value, unsigned long long maxValue);
            void            UpdateVScrollBar(unsigned long long value, unsigned long long maxValue);

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
            virtual void    OnUpdateScrollBars();

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
            bool	Create(Control *parent, const char * text, const char * layout, TextFieldFlags::Type flags = TextFieldFlags::NONE, Handlers::SyntaxHighlightHandler handler = nullptr, void* Context = nullptr);
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
        enum class TextAreaFlags : unsigned int
        {
            NONE                = 0x000000,
            BORDER              = 0x000100,
            SHOW_LINE_NUMBERS   = 0x000200,
            PROCESS_TAB         = 0x000400,
            READONLY            = 0x000800,
            SCROLLBARS          = 0x001000,
            SYNTAX_HIGHLIGHTING = 0x002000,
        };

        class EXPORT TextArea : public Control
        {
        public:
            bool		Create(Control *parent, const char * text,  const char * layout, TextAreaFlags flags = TextAreaFlags::NONE, Handlers::SyntaxHighlightHandler handler = nullptr, void* handlerContext = nullptr);
            void	    Paint(Console::Renderer & renderer) override;
            bool	    OnKeyEvent(AppCUI::Input::Key::Type keyCode, char AsciiCode) override;
            void        OnUpdateScrollBars() override;
            void		OnFocus() override;
            void		OnAfterResize(int newWidth, int newHeight) override;
            void		OnAfterSetText(const char* newText) override;
            void		SetReadOnly(bool value);
            bool		IsReadOnly();
            void		SetTabCharacter(char tabCharacter);
            virtual ~TextArea();
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
        namespace ViewerFlags
        {
            enum Type : unsigned int
            {
                NONE        = 0,
                BORDER      = 0x000100,
            };
        }
        class EXPORT CanvasViewer : public Control {
        public:
            ~CanvasViewer();
            bool	Create(Control* parent, const char * layout, unsigned int canvasWidth, unsigned int canvasHeight, ViewerFlags::Type flags = ViewerFlags::NONE);
            bool	Create(Control* parent, const char * title, const char * layout, unsigned int canvasWidth, unsigned int canvasHeight, ViewerFlags::Type flags = ViewerFlags::NONE);
            void	Paint(Console::Renderer & renderer) override;
            bool	OnKeyEvent(AppCUI::Input::Key::Type keyCode, char AsciiCode) override;
            bool    OnMouseLeave() override;
            bool    OnMouseEnter() override;
            void    OnUpdateScrollBars() override;
            Console::Canvas*	GetCanvas();
        };
    
        enum class ListViewFlags: unsigned int
        {
            NONE                    = 0, 
            HIDE_COLUMNS            = 0x000100,
            HAS_CHECKBOX            = 0x000200,
            HIDE_COLUMNS_SEPARATORS = 0x000400,
            SORT_COLUMNS            = 0x000800,
            ITEM_SEPARATORS         = 0x001000,
            HIDECURRENTITEM         = 0x002000,
            ALLOWSELECTION          = 0x004000,
            SEARCHMODE              = 0x008000,
        };
        enum class ListViewItemType: unsigned int
        {
            REGULAR                 = 0,
            HIGHLIGHT               = 1,
            INACTIVE                = 2,
            ERROR_INFORMATION       = 3,
            WARNING_INFORMATION     = 4,
            COLOR_1                 = 5,
            COLOR_2                 = 6,
            COLOR_3                 = 7
        };
        union ItemData
        {
            void*               Pointer;
            unsigned int        UInt32Value;
            unsigned long long  UInt64Value;
        };
        class EXPORT ListView : public Control
        {
        public:
            bool			Create(Control *parent, const char * layout, ListViewFlags flags);
            bool            Reserve(unsigned int itemsCount);
            void	        Paint(Console::Renderer & renderer) override;
            bool	        OnKeyEvent(AppCUI::Input::Key::Type keyCode, char AsciiCode) override;
            void			OnMouseReleased(int x, int y, int butonState) override;
            void	        OnMousePressed(int x, int y, int Button) override;
            bool	        OnMouseDrag(int x, int y, int Button) override;
            bool            OnMouseOver(int x, int y) override;
            bool            OnMouseLeave() override;
            void			OnFocus() override;
            void            OnUpdateScrollBars() override;

            // coloane
            bool			AddColumn(const char *text, AppCUI::Console::TextAlignament Align, unsigned int Size = 10);
            bool			SetColumnText(unsigned int columnIndex, const char *text);
            bool			SetColumnAlignament(unsigned int columnIndex, AppCUI::Console::TextAlignament Align);
            bool			SetColumnWidth(unsigned int columnIndex, unsigned int width);
            bool			SetColumnClipboardCopyState(unsigned int columnIndex, bool allowCopy);
            bool			SetColumnFilterMode(unsigned int columnIndex, bool allowFilterForThisColumn);
            bool			DeleteColumn(unsigned int columnIndex);
            void			DeleteAllColumns();
            unsigned int	GetColumnsCount();

            // items add
            ItemHandle		AddItem(const char *text);
            ItemHandle		AddItem(const char *text, const char * subItem1);
            ItemHandle		AddItem(const char *text, const char * subItem1, const char * subItem2);
            ItemHandle		AddItem(const char *text, const char * subItem1, const char * subItem2, const char * subItem3);
            ItemHandle		AddItem(const char *text, const char * subItem1, const char * subItem2, const char * subItem3, const char * subItem4);
            ItemHandle		AddItem(const char *text, const char * subItem1, const char * subItem2, const char * subItem3, const char * subItem4, const char * subItem5);
            ItemHandle		AddItem(const char *text, const char * subItem1, const char * subItem2, const char * subItem3, const char * subItem4, const char * subItem5, const char * subItem6);
            ItemHandle		AddItem(const char *text, const char * subItem1, const char * subItem2, const char * subItem3, const char * subItem4, const char * subItem5, const char * subItem6, const char * subItem7);
            ItemHandle		AddItem(const char *text, const char * subItem1, const char * subItem2, const char * subItem3, const char * subItem4, const char * subItem5, const char * subItem6, const char * subItem7, const char * subItem8);

            // items properties
            bool			SetItemText(ItemHandle item, unsigned int subItemIndex, const char *text);
            const char*		GetItemText(ItemHandle item, unsigned int subItemIndex);
            bool			SetItemCheck(ItemHandle item, bool check);
            bool			SetItemSelect(ItemHandle item, bool select);
            bool			SetItemColor(ItemHandle item, AppCUI::Console::ColorPair color);
            bool			SetItemType(ItemHandle item, ListViewItemType type);
            bool			IsItemChecked(ItemHandle item);
            bool			IsItemSelected(ItemHandle item);
            bool			SetItemData(ItemHandle item, ItemData Data);
            ItemData*	    GetItemData(ItemHandle item);
            bool			SetItemXOffset(ItemHandle item, unsigned int XOffset);
            unsigned int	GetItemXOffset(ItemHandle item);
            bool			SetItemHeight(ItemHandle item, unsigned int Height);
            unsigned int	GetItemHeight(ItemHandle item);
            void			DeleteAllItems();
            unsigned int	GetItemsCount();
            ItemHandle		GetCurrentItem();
            bool			SetCurrentItem(ItemHandle item);
            void			SelectAllItems();
            void			UnSelectAllItems();
            void			CheckAllItems();
            void			UncheckAllItems();
            unsigned int	GetCheckedItemsCount();

            // misc
            void			SetClipboardSeparator(char ch);

            // sort
            void			SetItemCompareFunction(Handlers::ListViewItemComparer fnc, void *Context = nullptr);
            bool			Sort();
            bool			Sort(unsigned int columnIndex, bool ascendent);
            bool			Sort(unsigned int columnIndex, bool ascendent, Handlers::ListViewItemComparer fnc, void *Context = nullptr);


            virtual ~ListView();
        };


    };
    namespace Dialogs
    {
        enum class DialogResult : int
        {
            RESULT_NONE = 0,
            RESULT_OK = 1,
            RESULT_CANCEL = 2,
            RESULT_YES = 3,
            RESULT_NO = 4,
        };
        class EXPORT MessageBox
        {
            MessageBox() = delete;
        public:
            static void                 ShowError(const char * title, const char * message);
            static void                 ShowNotification(const char *title, const char *message);
            static void                 ShowWarning(const char *title, const char *message);
            static DialogResult         ShowOkCancel(const char *title, const char *message);
            static DialogResult         ShowYesNoCancel(const char *title, const char *message);
        };

    }
    namespace Log
    {
        enum class Severity: unsigned int
        {
            InternalError   = 3,
            Error           = 2,
            Warning         = 1,
            Information     = 0,
        };
        struct Message
        {
            Severity        Type;
            const char*     Content;
            const char*     FileName;
            const char*     Function;
            const char*     Condition;
            int             LineNumber;
        };
        void EXPORT Report(Severity type, const char* fileName, const char *function, const char *condition, int line, const char *format, ...);
        void EXPORT SetLogCallback(void(*callback)(const Message &));
        bool EXPORT ToFile(const char * fileName);
        bool EXPORT ToOutputDebugString();
        bool EXPORT ToStdErr();
    }
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
                struct {
                    Console::ColorPair  Text, Border, LineNumbers, CurrentLineNumber;
                } Normal, Focus, Inactive, Hover;
                Console::ColorPair SelectionColor;                
            } Text;
            struct {
                Console::ColorPair PageColor, TabBarColor, HoverColor, PageHotKeyColor, TabBarHotKeyColor, HoverHotKeyColor;
                Console::ColorPair ListSelectedPageColor, ListSelectedPageHotKey;
            } Tab;
            struct {
                struct {
                    Console::ColorPair Border, Text, Hotkey;
                } Normal, Focused, Inactive, Hover;
                Console::ColorPair InactiveCanvasColor;
            } View;
            struct {
                Console::ColorPair Bar, Arrows, Position;
            } ScrollBar;
            struct {
                struct {                    
                    Console::ColorPair Border, LineSparators;
                } Normal, Focused, Inactive, Hover;
                struct {
                    Console::ColorPair Text, HotKey, Separator;
                } ColumnNormal, ColumnHover, ColumnInactive, ColumnSort;
                struct {
                    Console::ColorPair Regular, Highligheted, Inactive, Error, Warning;
                } Item;
                Console::ColorPair CheckedSymbol, UncheckedSymbol;
                Console::ColorPair InactiveColor;
                Console::ColorPair FocusColor;
                Console::ColorPair SelectionColor;
                Console::ColorPair FocusAndSelectedColor;
                Console::ColorPair FilterText;
            } ListView;

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
inline constexpr AppCUI::Console::TextAlignament operator| (AppCUI::Console::TextAlignament f1, AppCUI::Console::TextAlignament f2)
{
    return static_cast<AppCUI::Console::TextAlignament>(static_cast<unsigned int>(f1) | static_cast<unsigned int>(f2));
}
inline constexpr AppCUI::Console::TextAlignament operator& (AppCUI::Console::TextAlignament f1, AppCUI::Console::TextAlignament f2)
{
    return static_cast<AppCUI::Console::TextAlignament>(static_cast<unsigned int>(f1) & static_cast<unsigned int>(f2));
}
inline constexpr void operator|=(AppCUI::Console::WriteCharacterBufferFlags::Type & f1, AppCUI::Console::WriteCharacterBufferFlags::Type f2)
{
    f1 = static_cast<AppCUI::Console::WriteCharacterBufferFlags::Type>(static_cast<unsigned int>(f1) | static_cast<unsigned int>(f2));
}
inline constexpr AppCUI::Controls::TabFlags::Type operator|(AppCUI::Controls::TabFlags::Type f1, AppCUI::Controls::TabFlags::Type f2)
{
    return static_cast<AppCUI::Controls::TabFlags::Type>(static_cast<unsigned int>(f1) | static_cast<unsigned int>(f2));
}
inline constexpr AppCUI::Controls::TextAreaFlags operator|(AppCUI::Controls::TextAreaFlags f1, AppCUI::Controls::TextAreaFlags f2)
{
    return static_cast<AppCUI::Controls::TextAreaFlags>(static_cast<unsigned int>(f1) | static_cast<unsigned int>(f2));
}
inline constexpr unsigned int operator&(unsigned int f1, AppCUI::Controls::ListViewFlags f2)
{
    return (static_cast<unsigned int>(f1) & static_cast<unsigned int>(f2));
}
inline constexpr AppCUI::Controls::ListViewFlags operator|(AppCUI::Controls::ListViewFlags f1, AppCUI::Controls::ListViewFlags f2)
{
    return static_cast<AppCUI::Controls::ListViewFlags>(static_cast<unsigned int>(f1) | static_cast<unsigned int>(f2));
}
#endif
