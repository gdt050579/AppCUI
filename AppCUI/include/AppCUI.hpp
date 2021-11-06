#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <functional>

// https://en.cppreference.com/w/cpp/feature_test
#if defined(__has_cpp_attribute)
#    define HAS_CPP_ATTR(attr) __has_cpp_attribute(attr)
#else
#    define HAS_CPP_ATTR (0)
#endif

#define NODISCARD_CPP17 201603L
#define NODISCARD_CPP19 201907L
#if HAS_CPP_ATTR(nodiscard) == NODISCARD_CPP17
#    define NODISCARD(msg) [[nodiscard]]
#elif HAS_CPP_ATTR(nodiscard) == NODISCARD_CPP19
#    define NODISCARD(msg) [[nodiscard(msg)]]
#else
#    define NODISCARD(msg)
#endif

#ifdef BUILD_AS_DYNAMIC_LIB
#    ifdef BUILD_FOR_WINDOWS
#        define EXPORT __declspec(dllexport)
#    else
#        define EXPORT
#    endif
#else
#    define EXPORT
#endif

#if _DEBUG
#    define ENABLE_LOGGING
#endif

#ifdef ENABLE_LOGGING
#    define ASSERT(c, error)                                                                                           \
        {                                                                                                              \
            if (!(c))                                                                                                  \
            {                                                                                                          \
                AppCUI::Log::Report(AppCUI::Log::Severity::Fatal, __FILE__, __FUNCTION__, #c, __LINE__, "%s", error);  \
                throw error;                                                                                           \
            }                                                                                                          \
        }
#    define CHECK(c, returnValue, format, ...)                                                                         \
        {                                                                                                              \
            if (!(c))                                                                                                  \
            {                                                                                                          \
                AppCUI::Log::Report(                                                                                   \
                      AppCUI::Log::Severity::Error, __FILE__, __FUNCTION__, #c, __LINE__, format, ##__VA_ARGS__);      \
                return (returnValue);                                                                                  \
            }                                                                                                          \
        }
#    define CHECKRET(c, format, ...)                                                                                   \
        {                                                                                                              \
            if (!(c))                                                                                                  \
            {                                                                                                          \
                AppCUI::Log::Report(                                                                                   \
                      AppCUI::Log::Severity::Error, __FILE__, __FUNCTION__, #c, __LINE__, format, ##__VA_ARGS__);      \
                return;                                                                                                \
            }                                                                                                          \
        }
#    define CHECKBK(c, format, ...)                                                                                    \
        {                                                                                                              \
            if (!(c))                                                                                                  \
            {                                                                                                          \
                AppCUI::Log::Report(                                                                                   \
                      AppCUI::Log::Severity::Error, __FILE__, __FUNCTION__, #c, __LINE__, format, ##__VA_ARGS__);      \
                break;                                                                                                 \
            }                                                                                                          \
        }
#    define RETURNERROR(returnValue, format, ...)                                                                      \
        {                                                                                                              \
            AppCUI::Log::Report(                                                                                       \
                  AppCUI::Log::Severity::Error, __FILE__, __FUNCTION__, "", __LINE__, format, ##__VA_ARGS__);          \
            return (returnValue);                                                                                      \
        }
#    define NOT_IMPLEMENTED(returnValue)                                                                               \
        {                                                                                                              \
            AppCUI::Log::Report(                                                                                       \
                  AppCUI::Log::Severity::Warning,                                                                      \
                  __FILE__,                                                                                            \
                  __FUNCTION__,                                                                                        \
                  "",                                                                                                  \
                  __LINE__,                                                                                            \
                  "Current function/method is not implemented under current OS");                                      \
            return (returnValue);                                                                                      \
        }
#    define LOG_INFO(format, ...)                                                                                      \
        AppCUI::Log::Report(                                                                                           \
              AppCUI::Log::Severity::Information, __FILE__, __FUNCTION__, "", __LINE__, format, ##__VA_ARGS__);
#    define LOG_WARNING(format, ...)                                                                                   \
        AppCUI::Log::Report(                                                                                           \
              AppCUI::Log::Severity::Warning, __FILE__, __FUNCTION__, "", __LINE__, format, ##__VA_ARGS__);
#    define LOG_ERROR(format, ...)                                                                                     \
        AppCUI::Log::Report(AppCUI::Log::Severity::Error, __FILE__, __FUNCTION__, "", __LINE__, format, ##__VA_ARGS__);
#else
inline void Unused(...)
{
}
#    define ASSERT(c, error)                                                                                           \
        {                                                                                                              \
            if (!(c))                                                                                                  \
            {                                                                                                          \
                throw(error);                                                                                          \
            }                                                                                                          \
        }
#    define CHECK(c, returnValue, format, ...)                                                                         \
        {                                                                                                              \
            if (!(c))                                                                                                  \
            {                                                                                                          \
                return (returnValue);                                                                                  \
            }                                                                                                          \
        }
#    define CHECKRET(c, format, ...)                                                                                   \
        {                                                                                                              \
            if (!(c))                                                                                                  \
                return;                                                                                                \
        }
#    define RETURNERROR(returnValue, format, ...)                                                                      \
        {                                                                                                              \
            return (returnValue);                                                                                      \
        }
#    define CHECKBK(c, format, ...)                                                                                    \
        if (!(c))                                                                                                      \
            break;
#    define NOT_IMPLEMENTED(returnValue)                                                                               \
        {                                                                                                              \
            return (returnValue);                                                                                      \
        }
#    define LOG_INFO(format, ...) Unused(format, ##__VA_ARGS__);
#    define LOG_WARNING(format, ...)
#    define LOG_ERROR(format, ...)
#endif

#define ADD_FLAG_OPERATORS(type, basic_type)                                                                           \
    inline constexpr type operator|(const type f1, const type f2)                                                      \
    {                                                                                                                  \
        return static_cast<type>(static_cast<basic_type>(f1) | static_cast<basic_type>(f2));                           \
    }                                                                                                                  \
    inline constexpr basic_type operator|(const basic_type f1, const type f2)                                          \
    {                                                                                                                  \
        return static_cast<basic_type>(f1) | static_cast<basic_type>(f2);                                              \
    }                                                                                                                  \
    inline constexpr basic_type operator|(const type f1, const basic_type f2)                                          \
    {                                                                                                                  \
        return static_cast<basic_type>(f1) | static_cast<basic_type>(f2);                                              \
    }                                                                                                                  \
    inline constexpr type operator&(const type f1, const type f2)                                                      \
    {                                                                                                                  \
        return static_cast<type>(static_cast<basic_type>(f1) & static_cast<basic_type>(f2));                           \
    }                                                                                                                  \
    inline constexpr type operator&(const basic_type f1, const type f2)                                                \
    {                                                                                                                  \
        return static_cast<type>(static_cast<basic_type>(f1) & static_cast<basic_type>(f2));                           \
    }                                                                                                                  \
    inline constexpr type operator&(const type f1, const basic_type f2)                                                \
    {                                                                                                                  \
        return static_cast<type>(static_cast<basic_type>(f1) & static_cast<basic_type>(f2));                           \
    }                                                                                                                  \
    inline constexpr bool operator&&(const type f1, const basic_type f2)                                               \
    {                                                                                                                  \
        return ((static_cast<basic_type>(f1) & static_cast<basic_type>(f2)) != 0);                                     \
    }                                                                                                                  \
    inline constexpr bool operator&&(const basic_type f1, const type f2)                                               \
    {                                                                                                                  \
        return ((static_cast<basic_type>(f1) & static_cast<basic_type>(f2)) != 0);                                     \
    }                                                                                                                  \
    inline constexpr type operator|=(type& f1, const type f2)                                                          \
    {                                                                                                                  \
        return f1 = static_cast<type>(static_cast<basic_type>(f1) | static_cast<basic_type>(f2));                      \
    }

#define ARRAY_LEN(x) std::extent<decltype(x)>::value

namespace AppCUI
{
namespace Graphics
{
    enum class Color : unsigned char
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
    struct Size
    {
        unsigned int Width, Height;
        inline Size() : Width(0), Height(0)
        {
        }
        inline Size(unsigned int width, unsigned int height) : Width(width), Height(height)
        {
        }
        inline void Set(unsigned int width, unsigned int height)
        {
            Width  = width;
            Height = height;
        }
    };
    struct Point
    {
        int X, Y;
        inline Point() : X(0), Y(0)
        {
        }
        inline Point(int x, int y) : X(x), Y(y)
        {
        }
        inline void Set(int x, int y)
        {
            X = x;
            Y = y;
        }
    };
    struct ColorPair
    {
        Color Foreground;
        Color Background;
    };
    constexpr ColorPair NoColorPair      = ColorPair{ Color::Transparent, Color::Transparent };
    constexpr ColorPair DefaultColorPair = ColorPair{ Color::White, Color::Black };
    struct Character
    {
        union
        {
            struct
            {
                char16_t Code;
                ColorPair Color;
            };
            unsigned int PackedValue;
        };
        inline constexpr bool operator==(char value) const
        {
            return Code == value;
        }
        inline constexpr bool operator!=(char value) const
        {
            return Code != value;
        }
        inline constexpr bool operator==(char16_t value) const
        {
            return Code == value;
        }
        inline constexpr bool operator!=(char16_t value) const
        {
            return Code != value;
        }
        inline constexpr bool operator>(char value) const
        {
            return Code > value;
        }
        inline constexpr bool operator>(char16_t value) const
        {
            return Code > value;
        }
        inline constexpr bool operator<(char value) const
        {
            return Code < value;
        }
        inline constexpr bool operator<(char16_t value) const
        {
            return Code < value;
        }
        inline constexpr bool operator>=(char value) const
        {
            return Code >= value;
        }
        inline constexpr bool operator>=(char16_t value) const
        {
            return Code >= value;
        }
        inline constexpr bool operator<=(char value) const
        {
            return Code <= value;
        }
        inline constexpr bool operator<=(char16_t value) const
        {
            return Code <= value;
        }
        inline constexpr operator char16_t() const
        {
            return Code;
        }
    };
    class EXPORT CharacterBuffer;
}; // namespace Graphics
namespace Utils
{
    using CharacterView = std::basic_string_view<AppCUI::Graphics::Character>;
    using ConstString   = std::variant<std::string_view, std::u8string_view, std::u16string_view, CharacterView>;
    template <typename T>
    class Pointer : public std::unique_ptr<T>
    {
      public:
        Pointer(T* obj) : std::unique_ptr<T>(obj)
        {
        }
        operator T*()
        {
            return this->get();
        }
    };
    template <typename T>
    class Reference;

    class GenericRef
    {
        void* ptr;

      public:
        GenericRef(void* p) : ptr(p)
        {
        }
        template <typename T>
        Reference<T> ToReference()
        {
            return Reference<T>((T*) ptr);
        }
    };
    template <typename T>
    class Reference
    {
        T* ptr;

      public:
        Reference() : ptr(nullptr)
        {
        }
        Reference(T* obj) : ptr(obj)
        {
        }

        constexpr inline T* operator->()
        {
            return ptr;
        }
        constexpr inline operator T&()
        {
            return *ptr;
        }
        constexpr inline bool operator==(const void* obj)
        {
            return ptr == obj;
        }
        constexpr inline bool operator==(const Reference<T>& obj)
        {
            return ptr == obj.ptr;
        }
        constexpr inline bool operator!=(const void* obj)
        {
            return ptr != obj;
        }
        constexpr inline bool operator!=(const Reference<T>& obj)
        {
            return ptr != obj.ptr;
        }
        constexpr inline void Reset()
        {
            ptr = nullptr;
        }
        constexpr inline bool IsValid() const
        {
            return ptr != nullptr;
        }
        template <typename C>
        constexpr inline Reference<C> To()
        {
            return Reference<C>(this->ptr);
        }
        template <typename C>
        constexpr inline Reference<C> UpCast()
        {
            return Reference<C>(this->ptr);
        }
        template <typename C>
        constexpr inline Reference<C> DownCast()
        {
            return Reference<C>((C*) (this->ptr));
        }
        constexpr inline operator size_t()
        {
            return (size_t) (this->ptr);
        }
        inline GenericRef ToGenericRef() const
        {
            return GenericRef(this->ptr);
        }
    };
} // namespace Utils
namespace Application
{
    struct Config;
    class CommandBar;
}; // namespace Application
namespace Dialogs
{
    enum class Result : int
    {
        None   = 0,
        Ok     = 1,
        Cancel = 2,
        Yes    = 3,
        No     = 4,
    };
}

namespace Input
{
    enum class Key : unsigned int
    {
        None = 0,
        F1   = 1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        Enter,
        Escape,
        Insert,
        Delete,
        Backspace,
        Tab,
        Left,
        Up,
        Down,
        Right,
        PageUp,
        PageDown,
        Home,
        End,
        Space,
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        N0,
        N1,
        N2,
        N3,
        N4,
        N5,
        N6,
        N7,
        N8,
        N9,
        Count, // must be the last

        // Modifier flas
        Alt   = 0x1000,
        Ctrl  = 0x2000,
        Shift = 0x4000
    };
    enum class MouseButton : unsigned int
    {
        None          = 0,
        Left          = 0x01,
        Center        = 0x02,
        Right         = 0x04,
        DoubleClicked = 0x08,
    };
    enum class MouseWheel : unsigned int
    {
        None = 0,
        Up,
        Down,
        Left,
        Right
    };
}; // namespace Input
namespace Utils
{
    class EXPORT Array32
    {
        unsigned int* Data;
        unsigned int Count, Allocated;

        bool Grow(unsigned int newSize);

      public:
        Array32();
        ~Array32();
        void Destroy();

        bool Create(unsigned int allocatedCount = 64);
        bool Create(unsigned int* vector, unsigned int vectorSize, unsigned int elementsCount = 0);
        bool Create(int* vector, unsigned int vectorSize, unsigned int elementsCount = 0);

        inline unsigned int* GetUInt32Array() const
        {
            return Data;
        }
        inline int* GetInt32Array() const
        {
            return (int*) Data;
        }
        inline unsigned int Len() const
        {
            return Count;
        }
        inline unsigned int GetAllocatedSize() const
        {
            return Allocated & 0x7FFFFFFF;
        }

        inline void Clear()
        {
            Count = 0;
        }
        bool Reserve(unsigned int newSize);
        bool Resize(unsigned int newSize);
        bool Push(unsigned int value);
        bool Push(int value);
        bool Get(unsigned int index, unsigned int& value);
        bool Get(unsigned int index, int& value);

        bool Sort(int (*compare)(int elem1, int elem2, void* Context), bool ascendent, void* Context = nullptr);
        bool Sort(
              int (*compare)(unsigned int elem1, unsigned int elem2, void* Context),
              bool ascendent,
              void* Context = nullptr);
    };
    class EXPORT String
    {
        char* Text;
        unsigned int Size;
        unsigned int Allocated;

        bool Grow(unsigned int newSize);

      public:
        String(void);
        String(const String& s);
        ~String(void);

        // Static functions
        static unsigned int Len(const char* string);
        static bool Add(
              char* destination,
              const char* source,
              unsigned int maxDestinationSize,
              unsigned int destinationSize          = 0xFFFFFFFF,
              unsigned int sourceSize               = 0xFFFFFFFF,
              unsigned int* resultedDestinationSize = nullptr);
        static bool Set(
              char* destination,
              const char* source,
              unsigned int maxDestinationSize,
              unsigned int sourceSize               = 0xFFFFFFFF,
              unsigned int* resultedDestinationSize = nullptr);
        static bool Equals(const char* sir1, const char* sir2, bool ignoreCase = false);
        static bool StartsWith(const char* sir, const char* text, bool ignoreCase = false);
        static bool StartsWith(std::string_view sir1, std::string_view sir2, bool ignoreCase = false);
        static bool EndsWith(
              const char* sir,
              const char* text,
              bool ignoreCase          = false,
              unsigned int sirTextSize = 0xFFFFFFFF,
              unsigned int textSize    = 0xFFFFFFFF);
        static bool Contains(const char* sir, const char* textToFind, bool ignoreCase = false);
        static int Compare(const char* sir1, const char* sir2, bool ignoreCase = false);
        // Create string object
        bool Create(unsigned int initialAllocatedBuffer = 64);
        bool Create(const char* text);
        bool Create(char* buffer, unsigned int bufferSize, bool emptyString = false);

        const char* GetText() const
        {
            return Text;
        }
        unsigned int Len() const
        {
            return Size;
        }
        unsigned int GetAllocatedSize() const
        {
            return Allocated & 0x7FFFFFFF;
        }

        int GetChar(int index) const;
        bool SetChar(int index, char value);

        bool Add(const char* text, unsigned int size = 0xFFFFFFFF);
        bool Add(const String& text);
        bool Add(const String* text);
        bool AddChar(char ch);
        bool AddChars(char ch, unsigned int count);

        bool InsertChar(char character, unsigned int position);
        bool DeleteChar(unsigned int position);
        bool Delete(unsigned int start, unsigned int end);

        bool Set(const char* text, unsigned int size = 0xFFFFFFFF);
        bool Set(const String& text);
        bool Set(const String* text);
        bool SetChars(char ch, unsigned int count);

        bool SetFormat(const char* format, ...);
        bool AddFormat(const char* format, ...);
        std::string_view Format(const char* format, ...);

        bool Realloc(unsigned int newSize);
        void Destroy();
        bool Truncate(unsigned int newSize);
        void Clear();

        bool StartsWith(const char* text, bool ignoreCase = false) const;
        bool StartsWith(const String* text, bool ignoreCase = false) const;
        bool StartsWith(const String& text, bool ignoreCase = false) const;
        bool EndsWith(const char* text, bool ignoreCase = false) const;
        bool EndsWith(const String* text, bool ignoreCase = false) const;
        bool EndsWith(const String& text, bool ignoreCase = false) const;
        bool Equals(const char* text, bool ignoreCase = false) const;
        bool Equals(const String& ss, bool ignoreCase = false) const;
        bool Contains(const char* text, bool ignoreCase = false) const;
        bool Contains(const String& ss, bool ignoreCase = false) const;
        int CompareWith(const char* text, bool ignoreCase = false) const;

        void ConvertToInternalNewLineFormat();

        inline String& operator=(const String& s)
        {
            this->Set(s);
            return *this;
        }
        inline String& operator=(const char* text)
        {
            this->Set(text);
            return *this;
        }
        inline operator char*() const
        {
            return Text;
        }
        inline operator const char*() const
        {
            return Text;
        }
        inline String& operator+=(const String& s)
        {
            this->Add(s);
            return *this;
        }
        inline String& operator+=(const char* text)
        {
            this->Add(text);
            return *this;
        }

        inline bool operator==(const String& s) const
        {
            return this->Equals(s);
        }
        inline bool operator!=(const String& s) const
        {
            return !this->Equals(s);
        }
        inline operator std::string_view() const
        {
            return std::string_view{ this->Text, this->Size };
        }
        inline std::string_view ToStringView() const
        {
            return std::string_view{ this->Text, this->Size };
        }
        char& operator[](int poz);
    };
    class EXPORT UnicodeStringBuilder
    {
        char16_t* chars;
        unsigned int length;
        unsigned int allocated;

        void Create(char16_t* localBuffer, size_t localBufferSize);

      public:
        UnicodeStringBuilder();
        UnicodeStringBuilder(char16_t* localBuffer, size_t localBufferSize);
        UnicodeStringBuilder(const AppCUI::Utils::ConstString& text);
        UnicodeStringBuilder(char16_t* localBuffer, size_t localBufferSize, const AppCUI::Utils::ConstString& text);
        UnicodeStringBuilder(const AppCUI::Graphics::CharacterBuffer& charBuffer);
        UnicodeStringBuilder(
              char16_t* localBuffer, size_t localBufferSize, const AppCUI::Graphics::CharacterBuffer& charBuffer);

        ~UnicodeStringBuilder();
        void Destroy();

        bool Set(const AppCUI::Utils::ConstString& text);
        bool Set(const AppCUI::Graphics::CharacterBuffer& charBuffer);
        bool Add(const AppCUI::Utils::ConstString& text);
        bool Add(const AppCUI::Graphics::CharacterBuffer& charBuffer);
        bool AddChar(char16_t ch);
        bool Resize(size_t size);

        void ToString(std::string& output) const;
        void ToString(std::u16string& output) const;
        void ToPath(std::filesystem::path& output) const;

        inline void Clear()
        {
            length = 0;
        }
        inline void Truncate(unsigned int newSize)
        {
            if (newSize < length)
                length = newSize;
        }
        inline unsigned int Len() const
        {
            return length;
        }
        inline const char16_t* GetString() const
        {
            return chars;
        }
        inline std::u16string_view ToStringView() const
        {
            return std::u16string_view{ chars, (size_t) length };
        }
        inline operator std::string() const
        {
            std::string temp;
            ToString(temp);
            return temp;
        }
        inline operator std::u16string() const
        {
            std::u16string temp;
            ToString(temp);
            return temp;
        }
        inline operator std::filesystem::path() const
        {
            std::filesystem::path temp;
            ToPath(temp);
            return temp;
        }
        inline operator std::u16string_view() const
        {
            return std::u16string_view{ chars, (size_t) length };
        }
        inline UnicodeStringBuilder& operator+=(const AppCUI::Utils::ConstString& text)
        {
            Add(text);
            return *this;
        }
        inline UnicodeStringBuilder& operator+=(const AppCUI::Graphics::CharacterBuffer& charBuffer)
        {
            Add(charBuffer);
            return *this;
        }
        inline UnicodeStringBuilder& operator=(const AppCUI::Utils::ConstString& text)
        {
            Set(text);
            return *this;
        }
        inline UnicodeStringBuilder& operator=(const AppCUI::Graphics::CharacterBuffer& charBuffer)
        {
            Set(charBuffer);
            return *this;
        }
    };

    enum class NumberParseFlags : unsigned int
    {
        None       = 0,
        BaseAuto   = 0,
        Base10     = 0x00000001,
        Base8      = 0x00000002,
        Base16     = 0x00000003,
        Base2      = 0x00000004,
        TrimSpaces = 0x00000010,
    };
    namespace Number
    {
        EXPORT std::optional<unsigned long long> ToUInt64(
              std::string_view text, NumberParseFlags flags = NumberParseFlags::None, unsigned int* size = nullptr);
        EXPORT std::optional<unsigned int> ToUInt32(
              std::string_view text, NumberParseFlags flags = NumberParseFlags::None, unsigned int* size = nullptr);
        EXPORT std::optional<unsigned short> ToUInt16(
              std::string_view text, NumberParseFlags flags = NumberParseFlags::None, unsigned int* size = nullptr);
        EXPORT std::optional<unsigned char> ToUInt8(
              std::string_view text, NumberParseFlags flags = NumberParseFlags::None, unsigned int* size = nullptr);

        EXPORT std::optional<long long> ToInt64(
              std::string_view text, NumberParseFlags flags = NumberParseFlags::None, unsigned int* size = nullptr);
        EXPORT std::optional<int> ToInt32(
              std::string_view text, NumberParseFlags flags = NumberParseFlags::None, unsigned int* size = nullptr);
        EXPORT std::optional<short> ToInt16(
              std::string_view text, NumberParseFlags flags = NumberParseFlags::None, unsigned int* size = nullptr);
        EXPORT std::optional<char> ToInt8(
              std::string_view text, NumberParseFlags flags = NumberParseFlags::None, unsigned int* size = nullptr);

        EXPORT std::optional<float> ToFloat(
              std::string_view text, NumberParseFlags flags = NumberParseFlags::None, unsigned int* size = nullptr);
        EXPORT std::optional<double> ToDouble(
              std::string_view text, NumberParseFlags flags = NumberParseFlags::None, unsigned int* size = nullptr);

    }; // namespace Number

    enum class NumericFormatFlags : unsigned short
    {
        None      = 0,
        LowerCase = 0x0001,
        PlusSign  = 0x0002,
        MinusSign = 0x0004,
        HexPrefix = 0x0008,
        HexSuffix = 0x0010,
        BinPrefix = 0x0020,
        OctPrefix = 0x0040,
    };
    struct NumericFormat
    {
        NumericFormatFlags Flags;
        unsigned char Base;
        unsigned char GroupSize;
        char GroupSeparator;
        unsigned char DigitsCount;
        NumericFormat(NumericFormatFlags flags)
            : Flags(flags), Base(10), GroupSize(0), GroupSeparator(0), DigitsCount(0)
        {
        }
        NumericFormat(NumericFormatFlags flags, unsigned char base)
            : Flags(flags), Base(base), GroupSize(0), GroupSeparator(0), DigitsCount(0)
        {
        }
        NumericFormat(NumericFormatFlags flags, unsigned char base, unsigned char groupSize, char groupSeparator)
            : Flags(flags), Base(base), GroupSize(groupSize), GroupSeparator(groupSeparator), DigitsCount(0)
        {
        }
        NumericFormat(
              NumericFormatFlags flags,
              unsigned char base,
              unsigned char groupSize,
              char groupSeparator,
              unsigned char digitsCount)
            : Flags(flags), Base(base), GroupSize(groupSize), GroupSeparator(groupSeparator), DigitsCount(digitsCount)
        {
        }
    };
    class EXPORT NumericFormatter
    {
        char temp[72]{}; // a minimum of 65 chars must be allocated to support 64 bits for binary translation
        char* heapBuffer;
        std::string_view ToHexString(unsigned long long value);
        std::string_view ToOctString(unsigned long long value);
        std::string_view ToBinString(unsigned long long value);
        std::string_view ToDecStringUnsigned(unsigned long long value);
        std::string_view ToDecStringSigned(long long value);
        std::string_view ToBaseUnsigned(unsigned long long value, int base);
        std::string_view ToBaseSigned(long long value, int base);
        std::string_view ToGenericBase(unsigned long long value, unsigned long long base);
        std::string_view ToStringUnsigned(unsigned long long value, NumericFormat fmt);
        std::string_view ToStringSigned(long long value, NumericFormat fmt);

      public:
        NumericFormatter() : heapBuffer(nullptr)
        {
        }
        ~NumericFormatter()
        {
            if (heapBuffer)
                delete[] heapBuffer;
            heapBuffer = nullptr;
        }
        // ToHex
        inline std::string_view ToHex(unsigned long long value)
        {
            return ToHexString(value);
        }
        inline std::string_view ToHex(unsigned int value)
        {
            return ToHexString((unsigned long long) value);
        }
        inline std::string_view ToHex(unsigned short value)
        {
            return ToHexString((unsigned long long) value);
        }
        inline std::string_view ToHex(unsigned char value)
        {
            return ToHexString((unsigned long long) value);
        }
        inline std::string_view ToHex(long long value)
        {
            return ToHexString(*(unsigned long long*) &value);
        }
        inline std::string_view ToHex(int value)
        {
            return ToHexString((unsigned long long) (*(unsigned int*) &value));
        }
        inline std::string_view ToHex(short value)
        {
            return ToHexString((unsigned long long) (*(unsigned short*) &value));
        }
        inline std::string_view ToHex(char value)
        {
            return ToHexString((unsigned long long) (*(unsigned char*) &value));
        }

        // ToDec
        inline std::string_view ToDec(unsigned long long value)
        {
            return ToDecStringUnsigned(value);
        }
        inline std::string_view ToDec(unsigned int value)
        {
            return ToDecStringUnsigned((unsigned long long) value);
        }
        inline std::string_view ToDec(unsigned short value)
        {
            return ToDecStringUnsigned((unsigned long long) value);
        }
        inline std::string_view ToDec(unsigned char value)
        {
            return ToDecStringUnsigned((unsigned long long) value);
        }
        inline std::string_view ToDec(long long value)
        {
            return ToDecStringSigned(value);
        }
        inline std::string_view ToDec(int value)
        {
            return ToDecStringSigned((long) value);
        }
        inline std::string_view ToDec(short value)
        {
            return ToDecStringSigned((long) value);
        }
        inline std::string_view ToDec(char value)
        {
            return ToDecStringSigned((long) value);
        }

        // ToOct
        inline std::string_view ToOct(unsigned long long value)
        {
            return ToOctString(value);
        }
        inline std::string_view ToOct(unsigned int value)
        {
            return ToOctString((unsigned long long) value);
        }
        inline std::string_view ToOct(unsigned short value)
        {
            return ToOctString((unsigned long long) value);
        }
        inline std::string_view ToOct(unsigned char value)
        {
            return ToOctString((unsigned long long) value);
        }
        inline std::string_view ToOct(long long value)
        {
            return ToOctString(*(unsigned long long*) &value);
        }
        inline std::string_view ToOct(int value)
        {
            return ToOctString((unsigned long long) (*(unsigned int*) &value));
        }
        inline std::string_view ToOct(short value)
        {
            return ToOctString((unsigned long long) (*(unsigned short*) &value));
        }
        inline std::string_view ToOct(char value)
        {
            return ToOctString((unsigned long long) (*(unsigned char*) &value));
        }

        // ToBin
        inline std::string_view ToBin(unsigned long long value)
        {
            return ToBinString(value);
        }
        inline std::string_view ToBin(unsigned int value)
        {
            return ToBinString((unsigned long long) value);
        }
        inline std::string_view ToBin(unsigned short value)
        {
            return ToBinString((unsigned long long) value);
        }
        inline std::string_view ToBin(unsigned char value)
        {
            return ToBinString((unsigned long long) value);
        }
        inline std::string_view ToBin(long long value)
        {
            return ToBinString(*(unsigned long long*) &value);
        }
        inline std::string_view ToBin(int value)
        {
            return ToBinString((unsigned long long) (*(unsigned int*) &value));
        }
        inline std::string_view ToBin(short value)
        {
            return ToBinString((unsigned long long) (*(unsigned short*) &value));
        }
        inline std::string_view ToBin(char value)
        {
            return ToBinString((unsigned long long) (*(unsigned char*) &value));
        }

        // ToBase
        inline std::string_view ToBase(unsigned long long value, int base)
        {
            return ToBaseUnsigned(value, base);
        }
        inline std::string_view ToBase(unsigned int value, int base)
        {
            return ToBaseUnsigned((unsigned long long) value, base);
        }
        inline std::string_view ToBase(unsigned short value, int base)
        {
            return ToBaseUnsigned((unsigned long long) value, base);
        }
        inline std::string_view ToBase(unsigned char value, int base)
        {
            return ToBaseUnsigned((unsigned long long) value, base);
        }
        inline std::string_view ToBase(long long value, int base)
        {
            return ToBaseSigned(value, base);
        }
        inline std::string_view ToBase(int value, int base)
        {
            return ToBaseSigned((long) value, base);
        }
        inline std::string_view ToBase(short value, int base)
        {
            return ToBaseSigned((long) value, base);
        }
        inline std::string_view ToBase(char value, int base)
        {
            return ToBaseSigned((long) value, base);
        }

        // ToString
        inline std::string_view ToString(unsigned long long value, NumericFormat fmt)
        {
            return ToStringUnsigned(value, fmt);
        }
        inline std::string_view ToString(unsigned int value, NumericFormat fmt)
        {
            return ToStringUnsigned((unsigned long long) value, fmt);
        }
        inline std::string_view ToString(unsigned short value, NumericFormat fmt)
        {
            return ToStringUnsigned((unsigned long long) value, fmt);
        }
        inline std::string_view ToString(unsigned char value, NumericFormat fmt)
        {
            return ToStringUnsigned((unsigned long long) value, fmt);
        }
        inline std::string_view ToString(long long value, NumericFormat fmt)
        {
            return ToStringSigned(value, fmt);
        }
        inline std::string_view ToString(int value, NumericFormat fmt)
        {
            return ToStringSigned((long) value, fmt);
        }
        inline std::string_view ToString(short value, NumericFormat fmt)
        {
            return ToStringSigned((long) value, fmt);
        }
        inline std::string_view ToString(char value, NumericFormat fmt)
        {
            return ToStringSigned((long) value, fmt);
        }
    };

    enum class StringEncoding : unsigned int
    {
        Ascii = 0,
        UTF8,
        Unicode16,
        CharacterBuffer,

        // must be the last member
        Count
    };

    static_assert(std::variant_size_v<ConstString> == static_cast<unsigned int>(StringEncoding::Count));

    struct ConstStringObject
    {
      private:
        template <typename T>
        inline void BuildFromAlternative(const ConstString& obj, StringEncoding encoding)
        {
            Data     = std::get<T>(obj).data();
            Length   = std::get<T>(obj).length();
            Encoding = encoding;
        }

      public:
        const void* Data;
        size_t Length;
        StringEncoding Encoding;
        inline ConstStringObject() : Data(nullptr), Length(0), Encoding(StringEncoding::Ascii)
        {
        }
        inline ConstStringObject(const ConstString& obj)
        {
            switch (obj.index())
            {
            case 0:
                BuildFromAlternative<std::string_view>(obj, StringEncoding::Ascii);
                break;
            case 1:
                BuildFromAlternative<std::u8string_view>(obj, StringEncoding::UTF8);
                break;
            case 2:
                BuildFromAlternative<std::u16string_view>(obj, StringEncoding::Unicode16);
                break;
            case 3:
                BuildFromAlternative<CharacterView>(obj, StringEncoding::CharacterBuffer);
                break;
            default:
                throw std::bad_variant_access();
            }
        }
    };

    template <int size>
    class LocalString : public String
    {
        char tempBuffer[size];

      public:
        LocalString()
        {
            Create(tempBuffer, size, true);
        }
    };
    template <size_t size>
    class LocalUnicodeStringBuilder : public UnicodeStringBuilder
    {
        char16_t tempBuffer[size];

      public:
        LocalUnicodeStringBuilder() : UnicodeStringBuilder(tempBuffer, size)
        {
        }
        LocalUnicodeStringBuilder(const AppCUI::Graphics::CharacterBuffer& charBuffer)
            : UnicodeStringBuilder(tempBuffer, size, charBuffer)
        {
        }
        LocalUnicodeStringBuilder(const AppCUI::Utils::ConstString& text) : UnicodeStringBuilder(tempBuffer, size, text)
        {
        }
    };
    template <unsigned short Size>
    class FixSizeString
    {
        static_assert(Size > 0);
        char data[Size + 1];
        unsigned short size;

      public:
        FixSizeString() : size(0)
        {
            data[0] = 0;
        }
        FixSizeString(std::string_view txt)
        {
            Set(txt);
        }
        constexpr inline operator std::string_view() const
        {
            return std::string_view{ data, size };
        }
        constexpr inline operator bool() const
        {
            return this->size != 0;
        }
        void Set(std::string_view txt)
        {
            size = (unsigned short) std::min((size_t) Size, txt.length());
            memcpy(data, txt.data(), size);
            data[size] = 0;
        }
        void Set(const char* text)
        {
            if (text)
            {
                const char* e = text;
                while (*e)
                    e++;
                Set(std::string_view{ text, static_cast<size_t>(e - text) });
            }
            else
            {
                Clear();
            }
        }
        constexpr inline unsigned short Len() const
        {
            return size;
        }
        constexpr inline const char* GetText() const
        {
            return data;
        }
        constexpr inline unsigned short MaxSize() const
        {
            return Size;
        }
        inline FixSizeString& operator=(std::string_view txt)
        {
            Set(txt);
            return *this;
        }
        inline void Clear()
        {
            this->data[0] = 0;
            this->size    = 0;
        }
        constexpr inline bool Empty() const
        {
            return this->size == 0;
        }
    };

    class EXPORT KeyUtils
    {
      public:
        constexpr static const unsigned int KEY_SHIFT_MASK = 0x7000;
        constexpr static const unsigned int KEY_SHIFT_BITS = 12;
        constexpr static const unsigned int KEY_CODE_MASK  = 0xFF;

        // Returns the name of the Key without modifiers
        static std::string_view GetKeyName(AppCUI::Input::Key keyCode);
        static std::string_view GetKeyModifierName(AppCUI::Input::Key keyCode);
        static std::string_view GetKeyNamePadded(AppCUI::Input::Key keyCode);
        static bool ToString(AppCUI::Input::Key keyCode, char* text, int maxTextSize);
        static bool ToString(AppCUI::Input::Key keyCode, AppCUI::Utils::String& text);
        static AppCUI::Input::Key FromString(std::string_view stringRepresentation);

        static AppCUI::Input::Key CreateHotKey(char16_t hotKey, AppCUI::Input::Key modifier = AppCUI::Input::Key::None);
    };

    class EXPORT IniValueArray
    {
        const char* text;
        unsigned int len;

      public:
        IniValueArray() : text(nullptr), len(0)
        {
        }
        IniValueArray(std::string_view obj) : text(obj.data()), len((unsigned int) obj.size())
        {
        }

        std::optional<unsigned long long> AsUInt64() const;
        std::optional<long long> AsInt64() const;
        std::optional<unsigned int> AsUInt32() const;
        std::optional<int> AsInt32() const;
        std::optional<bool> AsBool() const;
        std::optional<AppCUI::Input::Key> AsKey() const;
        inline std::optional<const char*> AsString() const
        {
            return text;
        }
        inline std::optional<std::string_view> AsStringView() const
        {
            return std::string_view(text, len);
        };
        std::optional<Graphics::Size> AsSize() const;
        std::optional<float> AsFloat() const;
        std::optional<double> AsDouble() const;

        unsigned long long ToUInt64(unsigned long long defaultValue = 0) const;
        unsigned int ToUInt32(unsigned int defaultValue = 0) const;
        long long ToInt64(long long defaultValue = -1) const;
        int ToInt32(int defaultValue = -1) const;
        bool ToBool(bool defaultValue = false) const;
        AppCUI::Input::Key ToKey(AppCUI::Input::Key defaultValue = AppCUI::Input::Key::None) const;
        const char* ToString(const char* defaultValue = nullptr) const;
        std::string_view ToStringView(std::string_view defaultValue = std::string_view{}) const;
        AppCUI::Graphics::Size ToSize(AppCUI::Graphics::Size defaultValue = AppCUI::Graphics::Size()) const;
        float ToFloat(float defaultValue = 0.0f) const;
        double ToDouble(double defaultValue = 0.0) const;

        inline bool HasValue() const
        {
            return text != nullptr;
        }
    };
    class EXPORT IniValue
    {
        void* Data;

      public:
        IniValue() : Data(nullptr)
        {
        }
        IniValue(void* data) : Data(data){};

        std::optional<unsigned long long> AsUInt64() const;
        std::optional<long long> AsInt64() const;
        std::optional<unsigned int> AsUInt32() const;
        std::optional<int> AsInt32() const;
        std::optional<bool> AsBool() const;
        std::optional<AppCUI::Input::Key> AsKey() const;
        std::optional<const char*> AsString() const;
        std::optional<std::string_view> AsStringView() const;
        std::optional<Graphics::Size> AsSize() const;
        std::optional<float> AsFloat() const;
        std::optional<double> AsDouble() const;

        unsigned long long ToUInt64(unsigned long long defaultValue = 0) const;
        unsigned int ToUInt32(unsigned int defaultValue = 0) const;
        long long ToInt64(long long defaultValue = -1) const;
        int ToInt32(int defaultValue = -1) const;
        bool ToBool(bool defaultValue = false) const;
        AppCUI::Input::Key ToKey(AppCUI::Input::Key defaultValue = AppCUI::Input::Key::None) const;
        const char* ToString(const char* defaultValue = nullptr) const;
        std::string_view ToStringView(std::string_view defaultValue = std::string_view{}) const;
        AppCUI::Graphics::Size ToSize(AppCUI::Graphics::Size defaultValue = AppCUI::Graphics::Size()) const;
        float ToFloat(float defaultValue = 0.0f) const;
        double ToDouble(double defaultValue = 0.0) const;

        bool IsArray() const;
        unsigned int GetArrayCount() const;
        IniValueArray operator[](int index) const;

        std::string_view GetName() const;

        inline bool HasValue() const
        {
            return Data != nullptr;
        }
    };
    class EXPORT IniSection
    {
        void* Data;

      public:
        IniSection() : Data(nullptr)
        {
        }
        IniSection(void* data) : Data(data){};

        inline bool Exists() const
        {
            return Data != nullptr;
        }
        std::string_view GetName() const;
        IniValue GetValue(std::string_view keyName);
        std::vector<IniValue> GetValues() const;
    };
    class EXPORT IniObject
    {
        void* Data;
        bool Init();

      public:
        IniObject();
        ~IniObject();

        bool CreateFromString(std::string_view text);
        bool CreateFromFile(const std::filesystem::path& fileName);
        bool Create();

        bool HasSection(std::string_view name) const;
        IniSection GetSection(std::string_view name);
        IniValue GetValue(std::string_view valuePath);
        std::vector<IniSection> GetSections() const;
        unsigned int GetSectionsCount();
    };

}; // namespace Utils
namespace OS
{
    class EXPORT Clipboard
    {
        Clipboard() = delete;

      public:
        static bool SetText(const AppCUI::Utils::ConstString& text);
        static bool GetText(AppCUI::Utils::UnicodeStringBuilder& text);
        static bool Clear();
    };
    class EXPORT IFile
    {
      public:
        virtual ~IFile();

        // virtual methods
        virtual bool ReadBuffer(void* buffer, unsigned int bufferSize, unsigned int& bytesRead);
        virtual bool WriteBuffer(const void* buffer, unsigned int bufferSize, unsigned int& bytesWritten);
        virtual unsigned long long GetSize();
        virtual unsigned long long GetCurrentPos();
        virtual bool SetSize(unsigned long long newSize);
        virtual bool SetCurrentPos(unsigned long long newPosition);
        virtual void Close();

        // other methods
        bool Read(void* buffer, unsigned int bufferSize, unsigned int& bytesRead);
        bool Write(const void* buffer, unsigned int bufferSize, unsigned int& bytesWritten);
        bool Read(void* buffer, unsigned int bufferSize);
        bool Write(const void* buffer, unsigned int bufferSize);
        bool Read(unsigned long long offset, void* buffer, unsigned int bufferSize, unsigned int& bytesRead);
        bool Write(unsigned long long offset, const void* buffer, unsigned int bufferSize, unsigned int& bytesWritten);
        bool Write(std::string_view text);
        bool Write(unsigned long long offset, std::string_view text, unsigned int& bytesWritten);
        std::unique_ptr<char[]> ReadContentToBuffer(unsigned int& bufferSize);
    };

    class EXPORT File : public IFile
    {
        union
        {
            void* Handle;
            unsigned long long u64Value;
            unsigned int u32Value;
            int fid;
        } FileID;

      public:
        File();
        ~File();

        /**
         * Opens a file for Read/Write. The file MUST exists. The file pointer will be set at the end of the file.
         * @param[in] filePath is the full path to an existing file.
         */
        bool OpenWrite(const std::filesystem::path& path);
        /**
         * Opens a file for Read. The file MUST exists. The file pointer will be set at the begining of the file.
         * @param[in] filePath is the full path to an existing file.
         */
        bool OpenRead(const std::filesystem::path& path);
        /**
         * Creates a new file. If the file exists and overwriteExisting parameter is set to true, it will be
         * overwritten.
         * @param[in] filePath is the full path to an existing file.
         * @param[in] overwriteExisting - if set to true and the file exists it will overwrite the file. If set to false
         * and a file exists, will fail to overwrite and the method will return false.
         */
        bool Create(const std::filesystem::path& path, bool overwriteExisting = true);

        bool ReadBuffer(void* buffer, unsigned int bufferSize, unsigned int& bytesRead) override;
        bool WriteBuffer(const void* buffer, unsigned int bufferSize, unsigned int& bytesWritten) override;
        unsigned long long GetSize() override;
        unsigned long long GetCurrentPos() override;
        bool SetSize(unsigned long long newSize) override;
        bool SetCurrentPos(unsigned long long newPosition) override;
        void Close() override;
    };

    class EXPORT Library
    {
        void* libraryHandle;

      public:
        Library();
        bool Load(const std::filesystem::path& path);
        void* GetFunction(const char* functionName) const;
        template <typename T>
        inline T GetFunction(const char* functionName) const
        {
            return reinterpret_cast<T>(GetFunction(functionName));
        }
    };
    enum class SpecialFolder : unsigned int
    {
        AppPath = 0,
        Desktop,
        Documents,
        LocalDocuments,
        Downloads,
        LocalDownloads,
        Music,
        LocalMusic,
        Pictures,
        LocalPictures,
        Videos,
        LocalVideos,
    };

    struct FSLocationData
    {
        std::string locationName;
        std::filesystem::path locationPath;
    };

    using SpecialFolderMap = std::map<SpecialFolder, FSLocationData>;
    using RootsVector      = std::vector<FSLocationData>;

    // Fills the specialFolders map and roots vector with paths
    EXPORT void GetSpecialFolders(SpecialFolderMap& specialFolders, RootsVector& roots);
    EXPORT std::filesystem::path GetCurrentApplicationPath();

} // namespace OS
namespace Graphics
{
    enum class Alignament : unsigned char
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
        MenuSign,
        FourPoints,

        // always last
        Count
    };

    enum class TextAlignament : unsigned int
    {
        Left   = 0x00,
        Center = 0x01,
        Right  = 0x02,
    };

    namespace ProgressStatus
    {
        void EXPORT Init(const AppCUI::Utils::ConstString& Title, unsigned long long maxValue = 0);
        bool EXPORT Update(unsigned long long value, const AppCUI::Utils::ConstString& content);
        bool EXPORT Update(unsigned long long value);
    }; // namespace ProgressStatus

    enum class WriteTextFlags : unsigned int
    {
        None            = 0,
        SingleLine      = 0x0000001,
        MultipleLines   = 0x0000002,
        OverwriteColors = 0x0000004,
        HighlightHotKey = 0x0000008,
        ClipToWidth     = 0x0000010,
        FitTextToWidth  = 0x0000020,
        WrapToWidth     = 0x0000040,
        LeftMargin      = 0x0000080,
        RightMargin     = 0x0000100,
    };
    struct WriteTextParams
    {
        WriteTextFlags Flags;
        ColorPair Color;
        ColorPair HotKeyColor;
        TextAlignament Align;
        unsigned int HotKeyPosition;
        int X, Y;
        unsigned int Width;

        WriteTextParams() : Flags(WriteTextFlags::None), Align(TextAlignament::Left)
        {
        }
        WriteTextParams(WriteTextFlags flags) : Flags(flags), Align(TextAlignament::Left)
        {
        }
        WriteTextParams(WriteTextFlags flags, TextAlignament align) : Flags(flags), Align(align)
        {
        }
    };

    class EXPORT Rect
    {
        int X, Y, Width, Height;

      public:
        Rect() : X(0), Y(0), Width(0), Height(0)
        {
        }
        bool Create(int x, int y, int width, int height, Alignament align);
        void Create(int left, int top, int right, int bottom);
        inline bool Contains(int x, int y) const
        {
            return (x >= this->X) && (y >= this->Y) && (x < (this->X + this->Width)) && (y < (this->Y + this->Height));
        }
        inline int GetCenterX() const
        {
            return X + (Width >> 1);
        }
        inline int GetCenterY() const
        {
            return Y + (Height >> 1);
        }
        inline int GetX() const
        {
            return X;
        }
        inline int GetY() const
        {
            return Y;
        }
        inline unsigned int GetWidth() const
        {
            return (unsigned int) Width;
        }
        inline unsigned int GetHeight() const
        {
            return (unsigned int) Height;
        }
        inline int GetLeft() const
        {
            return X;
        }
        inline int GetTop() const
        {
            return Y;
        }
        inline int GetRight() const
        {
            return X + Width - 1;
        }
        inline int GetBottom() const
        {
            return Y + Height - 1;
        }
    };

    class EXPORT Clip
    {
      public:
        struct
        {
            int X, Y, Width, Height;
        } ClipRect;
        struct
        {
            int X, Y;
        } ScreenPosition;
        bool Visible;

      public:
        Clip();
        Clip(const Clip& parent, int relative_x, int relative_y, int width, int height);
        Clip(int x, int y, int width, int height);
        void Reset();
        void Set(const Clip& parent, int relativeLeft, int relativeTop, int relativeRight, int relativeBottom);
        void Set(int x, int y, int width, int height);
    };

    class EXPORT CharacterBuffer
    {
        Character* Buffer;
        unsigned int Count;
        unsigned int Allocated;

        bool Grow(size_t newSize);

      public:
        static constexpr unsigned int INVALID_HOTKEY_OFFSET = 0xFFFFFFFF;

        void Swap(CharacterBuffer&) noexcept;
        CharacterBuffer();
        inline CharacterBuffer(const CharacterBuffer& obj)
        {
            Buffer = nullptr;
            Count = Allocated = 0;
            Set(obj);
        }
        inline CharacterBuffer(CharacterBuffer&& obj) noexcept
        {
            Buffer = nullptr;
            Count = Allocated = 0;
            Swap(obj);
        }
        ~CharacterBuffer();

        void Destroy();
        void Clear();

        inline unsigned int Len() const
        {
            return Count;
        }
        inline unsigned int GetAllocatedChars() const
        {
            return Allocated;
        }
        inline Character* GetBuffer() const
        {
            return Buffer;
        }
        inline bool IsValid() const
        {
            return Buffer != nullptr;
        }
        inline bool IsEmpty() const
        {
            return (Buffer == nullptr) || (Count == 0);
        }

        bool Resize(unsigned int size, char16_t character = ' ', const ColorPair color = NoColorPair);
        bool Fill(char16_t character, unsigned int size, const ColorPair color = NoColorPair);
        bool Set(const CharacterBuffer& obj);
        bool Add(const AppCUI::Utils::ConstString& text, const ColorPair color = NoColorPair);
        bool Set(const AppCUI::Utils::ConstString& text, const ColorPair color = NoColorPair);
        bool SetWithHotKey(
              const AppCUI::Utils::ConstString& text,
              unsigned int& hotKeyCharacterPosition,
              AppCUI::Input::Key& hotKey,
              AppCUI::Input::Key hotKeyModifier = AppCUI::Input::Key::None,
              const ColorPair color             = NoColorPair);

        bool Delete(unsigned int start, unsigned int end);
        bool DeleteChar(unsigned int position);
        bool Insert(const AppCUI::Utils::ConstString& text, unsigned int position, const ColorPair color = NoColorPair);
        bool InsertChar(unsigned short characterCode, unsigned int position, const ColorPair color = NoColorPair);
        bool SetColor(unsigned int start, unsigned int end, const ColorPair color);
        void SetColor(const ColorPair color);
        bool CopyString(Utils::String& text, unsigned int start, unsigned int end);
        bool CopyString(Utils::String& text);

        int Find(const AppCUI::Utils::ConstString& text, bool ignoreCase = true) const;
        inline bool Contains(const AppCUI::Utils::ConstString& text, bool ignoreCase = true) const
        {
            return Find(text, ignoreCase) != -1;
        }
        int CompareWith(const CharacterBuffer& obj, bool ignoreCase = true) const;
        std::optional<unsigned int> FindNext(
              unsigned int startOffset, bool (*shouldSkip)(unsigned int offset, Character ch)) const;
        std::optional<unsigned int> FindPrevious(
              unsigned int startOffset, bool (*shouldSkip)(unsigned int offset, Character ch)) const;

        bool ToString(std::string& output) const;
        bool ToString(std::u16string& output) const;
        bool ToPath(std::filesystem::path& output) const;

        inline CharacterBuffer& operator=(const CharacterBuffer& obj)
        {
            Set(obj);
            return *this;
        }
        inline CharacterBuffer& operator=(CharacterBuffer&& obj) noexcept
        {
            Swap(obj);
            return *this;
        }
        inline operator std::string() const
        {
            std::string temp;
            ToString(temp);
            return temp;
        }
        inline operator std::u16string() const
        {
            std::u16string temp;
            ToString(temp);
            return temp;
        }
        inline operator std::filesystem::path() const
        {
            std::filesystem::path temp;
            ToPath(temp);
            return temp;
        }
        inline operator AppCUI::Utils::CharacterView() const
        {
            return AppCUI::Utils::CharacterView(Buffer, Count);
        }
        inline AppCUI::Utils::CharacterView SubString(size_t start, size_t end) const
        {
            if ((end > start) && (Buffer) && (end <= Count))
                return AppCUI::Utils::CharacterView{ Buffer + start, end - start };
            return AppCUI::Utils::CharacterView{ nullptr, 0 };
        }
    };

    enum class ImageRenderingMethod : unsigned int
    {
        PixelTo16ColorsSmallBlock,
        PixelTo64ColorsLargeBlock,
        AsciiArt
    };
    enum class ImageScaleMethod : unsigned int
    {
        NoScale = 1,
        Scale50 = 2,
        Scale33 = 3,
        Scale25 = 4,
        Scale20 = 5,
        Scale10 = 10,
        Scale5  = 20
    };
    class EXPORT Image
    {
        unsigned int* Pixels;
        unsigned int Width;
        unsigned int Height;

      public:
        Image();
        ~Image();
        bool Create(unsigned int width, unsigned int height);
        bool Create(unsigned int width, unsigned int height, std::string_view image);
        bool SetPixel(unsigned int x, unsigned int y, const Color color);
        bool SetPixel(unsigned int x, unsigned int y, unsigned int colorRGB);
        bool SetPixel(
              unsigned int x,
              unsigned int y,
              unsigned char Red,
              unsigned char Green,
              unsigned char Blue,
              unsigned char Alpha = 255);
        unsigned int GetPixel(unsigned int x, unsigned int y, unsigned int invalidIndexValue = 0) const;
        bool GetPixel(unsigned int x, unsigned int y, unsigned int& color) const;
        unsigned int ComputeSquareAverageColor(unsigned int x, unsigned int y, unsigned int sz) const;
        bool Clear(unsigned int color);
        bool Clear(const Color color);
        inline unsigned int GetWidth() const
        {
            return Width;
        }
        inline unsigned int GetHeight() const
        {
            return Height;
        }
        inline unsigned int* GetPixelsBuffer() const
        {
            return Pixels;
        }
    };

    class EXPORT Canvas;
    class EXPORT Renderer
    {
      protected:
        Character* Characters;
        Character** OffsetRows;
        unsigned int Width, Height;
        int TranslateX, TranslateY;
        struct
        {
            int Left, Top, Right, Bottom;
            bool Visible;
        } Clip, ClipCopy;
        bool ClipHasBeenCopied;
        struct
        {
            unsigned int X, Y;
            bool Visible;
        } Cursor;

        Renderer();
        ~Renderer();

        void _Destroy();
        bool _ClearEntireSurface(int character, const ColorPair color);
        bool _Compute_DrawTextInfo_SingleLine_(
              const WriteTextParams& params, size_t charactersCount, void* drawTextInfoOutput);

      public:
        // Horizontal lines
        bool FillHorizontalLine(int left, int y, int right, int charCode, const ColorPair color);
        bool FillHorizontalLineSize(int x, int y, unsigned int size, int charCode, const ColorPair color);
        bool FillHorizontalLineWithSpecialChar(int left, int y, int right, SpecialChars charID, const ColorPair color);
        bool DrawHorizontalLine(int left, int y, int right, const ColorPair color, bool singleLine = true);

        // Vertical lines
        bool FillVerticalLine(int x, int top, int bottom, int charCode, const ColorPair color);
        bool FillVerticalLineSize(int x, int y, unsigned int size, int charCode, const ColorPair color);
        bool FillVerticalLineWithSpecialChar(int x, int top, int bottom, SpecialChars charID, const ColorPair color);
        bool DrawVerticalLine(int x, int top, int bottom, const ColorPair color, bool singleLine = true);

        // Rectangle
        bool FillRect(int left, int top, int right, int bottom, int charCode, const ColorPair color);
        bool FillRectSize(int x, int y, unsigned int width, unsigned int height, int charCode, const ColorPair color);
        bool DrawRect(int left, int top, int right, int bottom, const ColorPair color, bool doubleLine);
        bool DrawRectSize(
              int x, int y, unsigned int width, unsigned int height, const ColorPair color, bool doubleLine);

        // Characters
        bool GetCharacter(int x, int y, Character& c);
        bool WriteCharacter(int x, int y, int charCode, const ColorPair color);
        bool WriteSpecialCharacter(int x, int y, SpecialChars charID, const ColorPair color);

        // Texts
        bool WriteText(const AppCUI::Utils::ConstString& text, const WriteTextParams& params);

        // Single line wrappers
        bool WriteSingleLineCharacterBuffer(
              int x, int y, AppCUI::Graphics::CharacterBuffer& charBuffer, bool noTransparency = true);
        bool WriteSingleLineText(int x, int y, const AppCUI::Utils::ConstString& text, ColorPair color);
        bool WriteSingleLineText(
              int x, int y, const AppCUI::Utils::ConstString& text, ColorPair color, TextAlignament align);
        bool WriteSingleLineText(
              int x,
              int y,
              const AppCUI::Utils::ConstString& text,
              ColorPair color,
              ColorPair hotKeyColor,
              unsigned int hotKeyOffset);
        bool WriteSingleLineText(
              int x,
              int y,
              const AppCUI::Utils::ConstString& text,
              ColorPair color,
              ColorPair hotKeyColor,
              unsigned int hotKeyOffset,
              TextAlignament align);
        bool WriteSingleLineText(
              int x, int y, unsigned int width, const AppCUI::Utils::ConstString& text, ColorPair color);
        bool WriteSingleLineText(
              int x,
              int y,
              unsigned int width,
              const AppCUI::Utils::ConstString& text,
              ColorPair color,
              TextAlignament align);
        bool WriteSingleLineText(
              int x,
              int y,
              unsigned int width,
              const AppCUI::Utils::ConstString& text,
              ColorPair color,
              ColorPair hotKeyColor,
              unsigned int hotKeyOffset);
        bool WriteSingleLineText(
              int x,
              int y,
              unsigned int width,
              const AppCUI::Utils::ConstString& text,
              ColorPair color,
              ColorPair hotKeyColor,
              unsigned int hotKeyOffset,
              TextAlignament align);

        // Canvas & Images
        bool DrawCanvas(int x, int y, const Canvas& canvas, const ColorPair overwriteColor = NoColorPair);

        // Clear
        bool ClearWithSpecialChar(SpecialChars charID, const ColorPair color);
        bool Clear(int charCode, const ColorPair color);

        // Clipping
        bool SetClipMargins(int leftMargin, int topMargin, int rightMargin, int bottomMargin);
        bool ResetClip();

        // Cursor
        void HideCursor();
        bool SetCursor(int x, int y);

        // Images
        bool DrawImage(
              const Image& img,
              int x,
              int y,
              ImageRenderingMethod method = ImageRenderingMethod::PixelTo16ColorsSmallBlock,
              ImageScaleMethod scale      = ImageScaleMethod::NoScale);
        Size ComputeRenderingSize(
              const Image& img,
              ImageRenderingMethod method = ImageRenderingMethod::PixelTo16ColorsSmallBlock,
              ImageScaleMethod scale      = ImageScaleMethod::NoScale);
    };

    class EXPORT Canvas : public Renderer
    {
      public:
        Canvas();
        ~Canvas();
        bool Create(
              unsigned int width,
              unsigned int height,
              int fillCharacter     = ' ',
              const ColorPair color = DefaultColorPair);
        bool Resize(
              unsigned int width,
              unsigned int height,
              int fillCharacter     = ' ',
              const ColorPair color = DefaultColorPair);

        // Clipping & Translate
        void SetAbsoluteClip(const AppCUI::Graphics::Clip& clip);
        void ExtendAbsoluteClipInAllDirections(int size);
        void ExtendAbsoluteClipToRightBottomCorner();
        void ClearClip();
        void SetTranslate(int offX, int offY);

        void Reset();
        void DarkenScreen();
        bool ClearEntireSurface(int character, const ColorPair color);

        // inlines
        inline unsigned int GetWidth() const
        {
            return this->Width;
        }
        inline unsigned int GetHeight() const
        {
            return this->Height;
        }
        inline bool GetCursorVisibility() const
        {
            return this->Cursor.Visible;
        }
        inline unsigned int GetCursorX() const
        {
            return this->Cursor.X;
        }
        inline unsigned int GetCursorY() const
        {
            return this->Cursor.Y;
        }
        inline Character* GetCharactersBuffer() const
        {
            return this->Characters;
        }
    };

}; // namespace Graphics
namespace Controls
{
    namespace Factory
    {
        class EXPORT Label;
        class EXPORT Button;
        class EXPORT Password;
        class EXPORT CheckBox;
        class EXPORT RadioBox;
        class EXPORT Splitter;
        class EXPORT Panel;
        class EXPORT TextField;
        class EXPORT TextArea;
        class EXPORT TabPage;
        class EXPORT Tab;
        class EXPORT CanvasViewer;
        class EXPORT ImageViewer;
        class EXPORT ListView;
        class EXPORT ComboBox;
        class EXPORT NumericSelector;
        class EXPORT Window;
        class EXPORT Desktop;
        class EXPORT Tree;
    }; // namespace Factory
    enum class Event : unsigned int
    {
        WindowClose,
        WindowAccept,
        ButtonClicked,
        CheckedStatusChanged,
        TextChanged,
        TextFieldValidate,
        PasswordValidate,
        TabChanged,
        ListViewCurrentItemChanged,
        ListViewSelectionChanged,
        ListViewItemChecked,
        ListViewItemClicked,
        ComboBoxSelectedItemChanged,
        ComboBoxClosed,
        TerminateApplication,
        Command,
        NumericSelectorValueChanged,
        Custom,
    };
    typedef unsigned int ItemHandle;
    constexpr ItemHandle InvalidItemHandle = 0xFFFFFFFF;
    class EXPORT Control;
    class EXPORT Button;
    class EXPORT TextField;
    class EXPORT ListView;
    class EXPORT Tree;
    class EXPORT Menu;
    class EXPORT Window;

    using namespace AppCUI::Utils;

    namespace Handlers
    {
        using namespace AppCUI::Graphics;
        using namespace AppCUI;
        using namespace AppCUI::Input;

        typedef void (*OnButtonPressedHandler)(Reference<Controls::Button> r);
        typedef void (*PaintControlHandler)(Reference<Controls::Control> control, Renderer& renderer);
        typedef bool (*OnEventHandler)(Reference<Controls::Control> control, Controls::Event eventType, int controlID);
        typedef bool (*OnKeyEventHandler)(Reference<Controls::Control> control, Key keyCode, char16_t unicodeChar);
        typedef void (*OnCheckHandler)(Reference<Controls::Control> control, bool value);
        typedef void (*OnFocusHandler)(Reference<Controls::Control> control);
        typedef void (*OnLoseFocusHandler)(Reference<Controls::Control> control);
        typedef void (*OnTextColorHandler)(Reference<Controls::Control> control, Character* chars, unsigned int len);
        typedef bool (*OnTreeItemToggleHandler)(Reference<Controls::Tree> control, ItemHandle handle);

        struct OnButtonPressedInterface
        {
            virtual void OnButtonPressed(Reference<Controls::Button> r) = 0;
        };
        struct OnButtonPressedCallback : public OnButtonPressedInterface
        {
            OnButtonPressedHandler callback;
            virtual void OnButtonPressed(Reference<Controls::Button> r) override
            {
                callback(r);
            };
        };

        struct PaintControlInterface
        {
            virtual void PaintControl(Reference<Controls::Control> control, Renderer& renderer) = 0;
        };
        struct PaintControlCallback : public PaintControlInterface
        {
            PaintControlHandler callback;
            virtual void PaintControl(Reference<Controls::Control> control, Renderer& renderer) override
            {
                callback(control, renderer);
            };
        };

        struct OnEventInterface
        {
            virtual bool OnEvent(Reference<Controls::Control> control, Controls::Event eventType, int controlID) = 0;
        };
        struct OnEventCallback : public OnEventInterface
        {
            OnEventHandler callback;
            virtual bool OnEvent(
                  Reference<Controls::Control> control, Controls::Event eventType, int controlID) override
            {
                return callback(control, eventType, controlID);
            };
        };

        struct OnKeyEventInterface
        {
            virtual bool OnKeyEvent(Reference<Controls::Control> control, Key keyCode, char16_t unicodeChar) = 0;
        };
        struct OnKeyEventCallback : public OnKeyEventInterface
        {
            OnKeyEventHandler callback;
            virtual bool OnKeyEvent(Reference<Controls::Control> control, Key keyCode, char16_t unicodeChar) override
            {
                return callback(control, keyCode, unicodeChar);
            };
        };

        struct OnCheckInterface
        {
            virtual void OnCheck(Reference<Controls::Control> control, bool value) = 0;
        };
        struct OnCheckCallback : public OnCheckInterface
        {
            OnCheckHandler callback;
            virtual void OnCheck(Reference<Controls::Control> control, bool value) override
            {
                callback(control, value);
            };
        };

        struct OnFocusInterface
        {
            virtual void OnFocus(Reference<Controls::Control> control) = 0;
        };
        struct OnFocusCallback : public OnFocusInterface
        {
            OnFocusHandler callback;
            virtual void OnFocus(Reference<Controls::Control> control) override
            {
                callback(control);
            };
        };

        struct OnLoseFocusInterface
        {
            virtual void OnLoseFocus(Reference<Controls::Control> control) = 0;
        };
        struct OnLoseFocusCallback : public OnLoseFocusInterface
        {
            OnLoseFocusHandler callback;
            virtual void OnLoseFocus(Reference<Controls::Control> control) override
            {
                callback(control);
            };
        };

        struct OnTextColorInterface
        {
            virtual void OnTextColor(Reference<Controls::Control> ctrl, Character* chars, unsigned int len) = 0;
        };
        struct OnTextColorCallback : public OnTextColorInterface
        {
            OnTextColorHandler callback;
            virtual void OnTextColor(Reference<Controls::Control> ctrl, Character* chars, unsigned int len) override
            {
                callback(ctrl, chars, len);
            };
        };

        struct OnTreeItemToggleInterface
        {
            virtual bool OnTreeItemToggle(Reference<Controls::Tree> ctrl, ItemHandle handle) = 0;
        };
        struct OnTreeItemToggleCallback : public OnTreeItemToggleInterface
        {
            OnTreeItemToggleHandler callback;
            
            virtual bool OnTreeItemToggle(Reference<Controls::Tree> ctrl, ItemHandle handle) override
            {
                return callback(ctrl, handle);
            };
        };

        template <typename I, typename C, typename H>
        class Wrapper
        {
            C callbackObj;

          public:
            I* obj;

          public:
            Wrapper() : obj(nullptr)
            {
            }
            constexpr inline void operator=(I* implementation)
            {
                this->obj = implementation;
            }
            constexpr inline void operator=(H callback)
            {
                this->callbackObj.callback = callback;
                this->obj                  = &this->callbackObj;
            }
        };

        struct Control
        {
            Wrapper<PaintControlInterface, PaintControlCallback, PaintControlHandler> PaintControl;
            Wrapper<OnEventInterface, OnEventCallback, OnEventHandler> OnEvent;
            Wrapper<OnKeyEventInterface, OnKeyEventCallback, OnKeyEventHandler> OnKeyEvent;
            Wrapper<OnFocusInterface, OnFocusCallback, OnFocusHandler> OnFocus;
            Wrapper<OnLoseFocusInterface, OnLoseFocusCallback, OnLoseFocusHandler> OnLoseFocus;
            virtual ~Control()
            {
            }
        };
        struct Button : public Control
        {
            Wrapper<OnButtonPressedInterface, OnButtonPressedCallback, OnButtonPressedHandler> OnButtonPressed;
        };
        struct CheckState : public Control
        {
            Wrapper<OnCheckInterface, OnCheckCallback, OnCheckHandler> OnCheck;
        };
        struct TextControl : public Control
        {
            Wrapper<OnTextColorInterface, OnTextColorCallback, OnTextColorHandler> OnTextColor;
        };

        typedef int (*ListViewItemComparer)(
              AppCUI::Controls::ListView* control,
              ItemHandle item1,
              ItemHandle item2,
              unsigned int columnIndex,
              void* Context);

        struct Tree : public Control
        {
            Wrapper<OnTreeItemToggleInterface, OnTreeItemToggleCallback, OnTreeItemToggleHandler> OnTreeItemToggle;
        };

    } // namespace Handlers

    class EXPORT Control
    {
      public:
        void* Context;

      private:
        bool RemoveControlByID(unsigned int index);
        bool RemoveControlByRef(Reference<Control> control);

      protected:
        bool IsMouseInControl(int x, int y);
        bool SetMargins(int left, int top, int right, int bottom);
        bool ShowToolTip(const AppCUI::Utils::ConstString& caption);
        bool ShowToolTip(const AppCUI::Utils::ConstString& caption, int x, int y);
        void HideToolTip();

        Reference<Control> AddChildControl(std::unique_ptr<Control> control);

        // protected constructor
        Control(void* context, const AppCUI::Utils::ConstString& caption, std::string_view layout, bool computeHotKey);

      public:
        template <typename T>
        Reference<T> AddControl(std::unique_ptr<T> control)
        {
            return this->AddChildControl(std::move(control)).template DownCast<T>();
        }
        template <typename T, typename... Arguments>
        Reference<T> CreateChildControl(Arguments... args)
        {
            return this->AddControl<T>(std::unique_ptr<T>(new T(std::forward<Arguments>(args)...)));
        }
        bool RemoveControl(Control* control);

        template <typename T>
        bool RemoveControl(Reference<T>& control)
        {
            if (RemoveControlByRef(control.template To<AppCUI::Controls::Control>()))
            {
                control.Reset();
                return true;
            }
            return false;
        }

        bool IsInitialized();

        // coordonates
        int GetX() const;
        int GetY() const;
        int GetWidth() const;
        int GetHeight() const;
        void GetSize(AppCUI::Graphics::Size& size);
        void GetClientSize(AppCUI::Graphics::Size& size);
        void MoveTo(int newX, int newY);
        bool Resize(int newWidth, int newHeight);
        void RecomputeLayout();

        // groups
        int GetGroup();
        void SetGroup(int newGroupID);
        void ClearGroup();

        // hot key
        bool SetHotKey(char16_t hotKey);
        Input::Key GetHotKey();
        void ClearHotKey();

        // status
        void SetEnabled(const bool value);
        void SetVisible(const bool value);
        void SetChecked(const bool value);
        bool IsEnabled() const;
        bool IsVisible() const;
        bool IsChecked() const;
        bool HasFocus() const;
        bool IsMouseOver() const;

        // childern and parent
        Reference<Control> GetParent();
        Control** GetChildrenList();
        Reference<Control> GetChild(unsigned int index);
        unsigned int GetChildernCount();
        bool GetChildIndex(Reference<Control> control, unsigned int& index);

        // Events
        void RaiseEvent(Event eventType);
        void RaiseEvent(Event eventType, int ID);

        // focus & expact/pack view
        bool SetFocus();
        void SetControlID(int newID);
        void ExpandView();
        void PackView();

        // Text
        bool SetText(const AppCUI::Utils::ConstString& caption, bool updateHotKey = false);
        bool SetText(const AppCUI::Graphics::CharacterBuffer& caption);
        const AppCUI::Graphics::CharacterBuffer& GetText();

        // Scroll bars
        void UpdateHScrollBar(unsigned long long value, unsigned long long maxValue);
        void UpdateVScrollBar(unsigned long long value, unsigned long long maxValue);

        // handlers
        virtual Handlers::Control* Handlers();

        // paint
        virtual void Paint(Graphics::Renderer& renderer);

        // Evenimente
        virtual bool OnKeyEvent(AppCUI::Input::Key keyCode, char16_t UnicodeChar);
        virtual void OnHotKey();
        virtual void OnHotKeyChanged();
        virtual void OnFocus();
        virtual void OnLoseFocus();
        virtual bool OnFrameUpdate();

        virtual void OnMousePressed(int x, int y, AppCUI::Input::MouseButton button);
        virtual void OnMouseReleased(int x, int y, AppCUI::Input::MouseButton button);
        virtual bool OnMouseDrag(int x, int y, AppCUI::Input::MouseButton button);

        virtual bool OnMouseEnter();
        virtual bool OnMouseOver(int x, int y);
        virtual bool OnMouseLeave();
        virtual bool OnMouseWheel(int x, int y, AppCUI::Input::MouseWheel direction);

        virtual bool OnEvent(Reference<Control> sender, Event eventType, int controlID);
        virtual bool OnUpdateCommandBar(AppCUI::Application::CommandBar& commandBar);
        virtual void OnUpdateScrollBars();

        virtual bool OnBeforeResize(int newWidth, int newHeight);
        virtual void OnAfterResize(int newWidth, int newHeight);
        virtual bool OnBeforeAddControl(Reference<Control> ctrl);
        virtual void OnAfterAddControl(Reference<Control> ctrl);
        virtual void OnControlRemoved(Reference<Control> ctrl);
        virtual bool OnBeforeSetText(const AppCUI::Utils::ConstString& text);
        virtual void OnAfterSetText(const AppCUI::Utils::ConstString& text);

        virtual void OnExpandView(AppCUI::Graphics::Clip& expandedClip);
        virtual void OnPackView();

        virtual ~Control();
    };

    enum class WindowFlags : unsigned int
    {
        None          = 0,
        Sizeable      = 0x000100,
        NotifyWindow  = 0x000200,
        ErrorWindow   = 0x000400,
        WarningWindow = 0x000800,
        NoCloseButton = 0x001000,
        FixedPosition = 0x004000,
        Maximized     = 0x008000,
        Menu          = 0x010000,
    };
    enum class WindowControlsBarLayout : unsigned char
    {
        None               = 0,
        TopBarFromLeft     = 1,
        BottomBarFromLeft  = 2,
        TopBarFromRight    = 3,
        BottomBarFromRight = 4,
    };
    class EXPORT WindowControlsBar
    {
        void* Context;
        WindowControlsBarLayout Layout;
        WindowControlsBar(void* ctx, WindowControlsBarLayout layout) : Context(ctx), Layout(layout)
        {
        }

      public:
        ItemHandle AddCommandItem(
              const AppCUI::Utils::ConstString& name, int ID, const AppCUI::Utils::ConstString& toolTip = "");
        ItemHandle AddSingleChoiceItem(
              const AppCUI::Utils::ConstString& name,
              int ID,
              bool checked,
              const AppCUI::Utils::ConstString& toolTip = std::string_view());
        ItemHandle AddCheckItem(
              const AppCUI::Utils::ConstString& name,
              int ID,
              bool checked,
              const AppCUI::Utils::ConstString& toolTip = std::string_view());
        ItemHandle AddTextItem(
              const AppCUI::Utils::ConstString& caption, const AppCUI::Utils::ConstString& toolTip = "");
        bool SetItemText(ItemHandle itemHandle, const AppCUI::Utils::ConstString& caption);
        bool SetItemToolTip(ItemHandle itemHandle, const AppCUI::Utils::ConstString& toolTipText);
        bool IsItemChecked(ItemHandle itemHandle);
        bool SetItemCheck(ItemHandle itemHandle, bool value);
        bool IsItemVisible(ItemHandle itemHandle);
        bool IsItemShown(ItemHandle itemHandle);
        bool SetItemVisible(ItemHandle itemHandle, bool value);
        friend class Window;
    };
    class EXPORT Window : public Control
    {
        bool ProcessControlBarItem(unsigned int index);

      protected:
        Window(const AppCUI::Utils::ConstString& caption, std::string_view layout, WindowFlags windowsFlags);

      public:
        void Paint(Graphics::Renderer& renderer) override;
        void OnMousePressed(int x, int y, AppCUI::Input::MouseButton button) override;
        void OnMouseReleased(int x, int y, AppCUI::Input::MouseButton button) override;
        bool OnMouseDrag(int x, int y, AppCUI::Input::MouseButton button) override;
        bool OnMouseOver(int x, int y) override;
        bool OnMouseLeave() override;
        bool OnEvent(Reference<Control> sender, Event eventType, int controlID) override;
        void RemoveMe();

        int Show();
        int GetDialogResult();
        bool MaximizeRestore();
        void SetTag(const AppCUI::Utils::ConstString& name, const AppCUI::Utils::ConstString& toolTipText);
        const AppCUI::Graphics::CharacterBuffer& GetTag();
        bool OnBeforeResize(int newWidth, int newHeight) override;
        void OnAfterResize(int newWidth, int newHeight) override;
        bool CenterScreen();
        bool OnKeyEvent(AppCUI::Input::Key keyCode, char16_t UnicodeChar) override;
        void OnHotKeyChanged() override;
        bool Exit(int dialogResult);
        bool Exit(Dialogs::Result dialogResult);
        bool IsWindowInResizeMode();

        Reference<Menu> AddMenu(const AppCUI::Utils::ConstString& name);
        WindowControlsBar GetControlBar(WindowControlsBarLayout layout);

        virtual ~Window();

        friend Factory::Window;
        friend Control;
    };
    class EXPORT Label : public Control
    {
        Label(const AppCUI::Utils::ConstString& caption, std::string_view layout);

      public:
        void Paint(Graphics::Renderer& renderer) override;
        friend Factory::Label;
        friend Control;
    };

    enum class ButtonFlags : unsigned int
    {
        None = 0,
        Flat = 0x000100,
    };
    class EXPORT Button : public Control
    {
      protected:
        Button(const AppCUI::Utils::ConstString& caption, std::string_view layout, int controlID, ButtonFlags flags);

      public:
        void OnMousePressed(int x, int y, AppCUI::Input::MouseButton button) override;
        void OnMouseReleased(int x, int y, AppCUI::Input::MouseButton button) override;
        bool OnMouseDrag(int x, int y, AppCUI::Input::MouseButton button) override;
        void Paint(Graphics::Renderer& renderer) override;
        bool OnKeyEvent(AppCUI::Input::Key keyCode, char16_t UnicodeChar) override;
        void OnHotKey() override;
        bool OnMouseEnter() override;
        bool OnMouseLeave() override;

        // handlers covariant
        Handlers::Button* Handlers() override;

        friend Factory::Button;
        friend Control;
    };
    class EXPORT CheckBox : public Control
    {
      protected:
        CheckBox(const AppCUI::Utils::ConstString& caption, std::string_view layout, int controlID);

      public:
        void OnMouseReleased(int x, int y, AppCUI::Input::MouseButton button) override;
        void Paint(Graphics::Renderer& renderer) override;
        bool OnKeyEvent(AppCUI::Input::Key keyCode, char16_t UnicodeChar) override;
        void OnHotKey() override;
        bool OnMouseEnter() override;
        bool OnMouseLeave() override;

        // handlers covariant
        Handlers::CheckState* Handlers() override;

        friend Factory::CheckBox;
        friend Control;
    };
    class EXPORT RadioBox : public Control
    {
      protected:
        RadioBox(const AppCUI::Utils::ConstString& caption, std::string_view layout, int groupID, int controlID);

      public:
        void OnMouseReleased(int x, int y, AppCUI::Input::MouseButton button) override;
        void Paint(Graphics::Renderer& renderer) override;
        bool OnKeyEvent(AppCUI::Input::Key keyCode, char16_t UnicodeChar) override;
        void OnHotKey() override;
        bool OnMouseEnter() override;
        bool OnMouseLeave() override;

        // handlers covariant
        Handlers::CheckState* Handlers() override;

        friend Factory::RadioBox;
        friend Control;
    };
    class EXPORT Splitter : public Control
    {
      protected:
        Splitter(std::string_view layout, bool vertical);

      public:
        void Paint(Graphics::Renderer& renderer) override;
        bool OnKeyEvent(AppCUI::Input::Key keyCode, char16_t UnicodeChar) override;
        bool SetSecondPanelSize(int newSize);
        bool HideSecondPanel();
        bool MaximizeSecondPanel();
        void OnAfterResize(int newWidth, int newHeight) override;
        void OnFocus() override;
        bool OnBeforeAddControl(Reference<Control> ctrl) override;
        void OnAfterAddControl(Reference<Control> ctrl) override;
        void OnMousePressed(int x, int y, AppCUI::Input::MouseButton button) override;
        void OnMouseReleased(int x, int y, AppCUI::Input::MouseButton button) override;
        bool OnMouseOver(int x, int y) override;
        bool OnMouseDrag(int x, int y, AppCUI::Input::MouseButton button) override;
        bool OnMouseEnter() override;
        bool OnMouseLeave() override;
        int GetSplitterPosition();
        virtual ~Splitter();

        friend Factory::Splitter;
        friend Control;
    };
    class EXPORT Password : public Control
    {
      protected:
        Password(const AppCUI::Utils::ConstString& caption, std::string_view layout);

      public:
        void OnMousePressed(int x, int y, AppCUI::Input::MouseButton button) override;
        void OnMouseReleased(int x, int y, AppCUI::Input::MouseButton button) override;
        bool OnMouseDrag(int x, int y, AppCUI::Input::MouseButton button) override;
        void Paint(Graphics::Renderer& renderer) override;
        bool OnKeyEvent(AppCUI::Input::Key keyCode, char16_t UnicodeChar) override;
        bool OnMouseEnter() override;
        bool OnMouseLeave() override;

        friend Factory::Password;
        friend Control;
    };
    class EXPORT Panel : public Control
    {
      protected:
        Panel(const AppCUI::Utils::ConstString& caption, std::string_view layout);

      public:
        void Paint(Graphics::Renderer& renderer) override;

        friend Factory::Panel;
        friend Control;
    };
    enum class TextFieldFlags : unsigned int
    {
        None               = 0,
        ProcessEnter       = 0x000100,
        Readonly           = 0x000200,
        SyntaxHighlighting = 0x000400,
    };
    class EXPORT TextField : public Control
    {
      protected:
        TextField(const AppCUI::Utils::ConstString& caption, std::string_view layout, TextFieldFlags flags);

      public:
        bool OnKeyEvent(AppCUI::Input::Key keyCode, char16_t UnicodeChar) override;
        void OnAfterSetText(const AppCUI::Utils::ConstString& text) override;
        void Paint(Graphics::Renderer& renderer) override;
        void OnFocus() override;
        bool OnMouseEnter() override;
        bool OnMouseLeave() override;
        void OnAfterResize(int newWidth, int newHeight) override;

        // handlers covariant
        Handlers::TextControl* Handlers() override;

        void SelectAll();
        void ClearSelection();

        virtual ~TextField();

        friend Factory::TextField;
        friend Control;
    };
    enum class TextAreaFlags : unsigned int
    {
        None               = 0x000000,
        Border             = 0x000100,
        ShowLineNumbers    = 0x000200,
        ProcessTabKey      = 0x000400,
        Readonly           = 0x000800,
        ScrollBars         = 0x001000,
        SyntaxHighlighting = 0x002000,
    };

    class EXPORT TextArea : public Control
    {
      protected:
        TextArea(const AppCUI::Utils::ConstString& caption, std::string_view layout, TextAreaFlags flags);

      public:
        void Paint(Graphics::Renderer& renderer) override;
        bool OnKeyEvent(AppCUI::Input::Key keyCode, char16_t UnicodeChar) override;
        void OnUpdateScrollBars() override;
        void OnFocus() override;
        void OnAfterResize(int newWidth, int newHeight) override;
        void OnAfterSetText(const AppCUI::Utils::ConstString& text) override;
        void SetReadOnly(bool value);
        bool IsReadOnly();
        void SetTabCharacter(char tabCharacter);
        virtual ~TextArea();

        // handlers covariant
        Handlers::TextControl* Handlers() override;

        friend Factory::TextArea;
        friend Control;
    };

    enum class TabFlags : unsigned int
    {
        TopTabs               = 0x000000, // default mode
        BottomTabs            = 0x000100,
        LeftTabs              = 0x000200,
        ListView              = 0x000300,
        HideTabs              = 0x000400,
        TransparentBackground = 0x001000,
        TabsBar               = 0x002000,
    };
    class EXPORT TabPage : public Control
    {
      protected:
        TabPage(const AppCUI::Utils::ConstString& caption);

      public:
        bool OnBeforeResize(int newWidth, int newHeight);

        friend Factory::TabPage;
        friend Control;
    };
    class EXPORT Tab : public Control
    {
      protected:
        Tab(std::string_view layout, TabFlags flags, unsigned int tabPageSize);

      public:
        bool SetCurrentTabPageByIndex(unsigned int index);
        bool SetCurrentTabPageByRef(Reference<Control> page);
        template <typename T>
        inline bool SetCurrentTabPage(Reference<T> page)
        {
            return SetCurrentTabPageByRef(page.template DownCast<Control>());
        }
        bool SetTabPageTitleSize(unsigned int newSize);
        bool SetTabPageName(unsigned int index, const AppCUI::Utils::ConstString& name);
        void OnAfterResize(int newWidth, int newHeight) override;
        void OnFocus() override;
        void OnMouseReleased(int x, int y, AppCUI::Input::MouseButton button) override;
        bool OnMouseLeave() override;
        bool OnMouseOver(int x, int y) override;
        bool OnKeyEvent(AppCUI::Input::Key keyCode, char16_t UnicodeChar) override;
        void OnAfterAddControl(Reference<Control> ctrl) override;
        void Paint(Graphics::Renderer& renderer) override;
        Reference<Control> GetCurrentTab();

        friend Factory::Tab;
        friend Control;
    };
    class EXPORT UserControl : public Control
    {
      protected:
        UserControl(const AppCUI::Utils::ConstString& caption, std::string_view layout);
        UserControl(std::string_view layout);
    };
    enum class ViewerFlags : unsigned int
    {
        None   = 0,
        Border = 0x000100,
    };
    class EXPORT CanvasViewer : public Control
    {
      protected:
        CanvasViewer(
              const AppCUI::Utils::ConstString& caption,
              std::string_view layout,
              unsigned int canvasWidth,
              unsigned int canvasHeight,
              ViewerFlags flags);

      public:
        ~CanvasViewer();
        void Paint(Graphics::Renderer& renderer) override;
        bool OnKeyEvent(AppCUI::Input::Key keyCode, char16_t UnicodeChar) override;
        bool OnMouseLeave() override;
        bool OnMouseEnter() override;
        bool OnMouseWheel(int x, int y, AppCUI::Input::MouseWheel direction) override;
        void OnMousePressed(int x, int y, AppCUI::Input::MouseButton button) override;
        bool OnMouseDrag(int x, int y, AppCUI::Input::MouseButton button) override;
        void OnMouseReleased(int x, int y, AppCUI::Input::MouseButton button) override;
        void OnUpdateScrollBars() override;
        Reference<Graphics::Canvas> GetCanvas();

        friend Factory::CanvasViewer;
        friend Control;
    };
    class EXPORT ImageViewer : public CanvasViewer
    {
      protected:
        ImageViewer(const AppCUI::Utils::ConstString& caption, std::string_view layout, ViewerFlags flags);

      public:
        bool SetImage(
              const AppCUI::Graphics::Image& img,
              AppCUI::Graphics::ImageRenderingMethod method,
              AppCUI::Graphics::ImageScaleMethod scale);

        friend Factory::ImageViewer;
        friend Control;
    };
    enum class ListViewFlags : unsigned int
    {
        None                          = 0,
        HideColumns                   = 0x000100,
        CheckBoxes                    = 0x000200,
        HideColumnsSeparator          = 0x000400,
        Sortable                      = 0x000800,
        ItemSeparators                = 0x001000,
        HideCurrentItemWhenNotFocused = 0x002000,
        AllowMultipleItemsSelection   = 0x004000,
        SearchMode                    = 0x008000,
        HideSearchBar                 = 0x010000
    };
    enum class ListViewItemType : unsigned short
    {
        Normal             = 0,
        Highlighted        = 1,
        GrayedOut          = 2,
        ErrorInformation   = 3,
        WarningInformation = 4,
    };

    class EXPORT ListView : public Control
    {
      private:
        GenericRef GetItemDataAsPointer(ItemHandle item) const;
        bool SetItemDataAsPointer(ItemHandle item, GenericRef obj);

      protected:
        ListView(std::string_view layout, ListViewFlags flags);

      public:
        bool Reserve(unsigned int itemsCount);
        void Paint(Graphics::Renderer& renderer) override;
        bool OnKeyEvent(AppCUI::Input::Key keyCode, char16_t UnicodeChar) override;
        void OnMouseReleased(int x, int y, AppCUI::Input::MouseButton button) override;
        void OnMousePressed(int x, int y, AppCUI::Input::MouseButton button) override;
        bool OnMouseDrag(int x, int y, AppCUI::Input::MouseButton button) override;
        bool OnMouseWheel(int x, int y, AppCUI::Input::MouseWheel direction) override;
        bool OnMouseOver(int x, int y) override;
        bool OnMouseLeave() override;
        void OnFocus() override;
        void OnUpdateScrollBars() override;

        // coloane
        bool AddColumn(
              const AppCUI::Utils::ConstString& text, AppCUI::Graphics::TextAlignament Align, unsigned int Size = 10);
        bool SetColumnText(unsigned int columnIndex, const AppCUI::Utils::ConstString& text);
        bool SetColumnAlignament(unsigned int columnIndex, AppCUI::Graphics::TextAlignament Align);
        bool SetColumnWidth(unsigned int columnIndex, unsigned int width);
        bool SetColumnClipboardCopyState(unsigned int columnIndex, bool allowCopy);
        bool SetColumnFilterMode(unsigned int columnIndex, bool allowFilterForThisColumn);
        bool DeleteColumn(unsigned int columnIndex);
        void DeleteAllColumns();
        unsigned int GetColumnsCount();

        // items add
        ItemHandle AddItem(const AppCUI::Utils::ConstString& text);
        ItemHandle AddItem(const AppCUI::Utils::ConstString& text, const AppCUI::Utils::ConstString& subItem1);
        ItemHandle AddItem(
              const AppCUI::Utils::ConstString& text,
              const AppCUI::Utils::ConstString& subItem1,
              const AppCUI::Utils::ConstString& subItem2);
        ItemHandle AddItem(
              const AppCUI::Utils::ConstString& text,
              const AppCUI::Utils::ConstString& subItem1,
              const AppCUI::Utils::ConstString& subItem2,
              const AppCUI::Utils::ConstString& subItem3);
        ItemHandle AddItem(
              const AppCUI::Utils::ConstString& text,
              const AppCUI::Utils::ConstString& subItem1,
              const AppCUI::Utils::ConstString& subItem2,
              const AppCUI::Utils::ConstString& subItem3,
              const AppCUI::Utils::ConstString& subItem4);
        ItemHandle AddItem(
              const AppCUI::Utils::ConstString& text,
              const AppCUI::Utils::ConstString& subItem1,
              const AppCUI::Utils::ConstString& subItem2,
              const AppCUI::Utils::ConstString& subItem3,
              const AppCUI::Utils::ConstString& subItem4,
              const AppCUI::Utils::ConstString& subItem5);
        ItemHandle AddItem(
              const AppCUI::Utils::ConstString& text,
              const AppCUI::Utils::ConstString& subItem1,
              const AppCUI::Utils::ConstString& subItem2,
              const AppCUI::Utils::ConstString& subItem3,
              const AppCUI::Utils::ConstString& subItem4,
              const AppCUI::Utils::ConstString& subItem5,
              const AppCUI::Utils::ConstString& subItem6);
        ItemHandle AddItem(
              const AppCUI::Utils::ConstString& text,
              const AppCUI::Utils::ConstString& subItem1,
              const AppCUI::Utils::ConstString& subItem2,
              const AppCUI::Utils::ConstString& subItem3,
              const AppCUI::Utils::ConstString& subItem4,
              const AppCUI::Utils::ConstString& subItem5,
              const AppCUI::Utils::ConstString& subItem6,
              const AppCUI::Utils::ConstString& subItem7);
        ItemHandle AddItem(
              const AppCUI::Utils::ConstString& text,
              const AppCUI::Utils::ConstString& subItem1,
              const AppCUI::Utils::ConstString& subItem2,
              const AppCUI::Utils::ConstString& subItem3,
              const AppCUI::Utils::ConstString& subItem4,
              const AppCUI::Utils::ConstString& subItem5,
              const AppCUI::Utils::ConstString& subItem6,
              const AppCUI::Utils::ConstString& subItem7,
              const AppCUI::Utils::ConstString& subItem8);

        // items properties
        bool SetItemText(ItemHandle item, unsigned int subItemIndex, const AppCUI::Utils::ConstString& text);
        const AppCUI::Graphics::CharacterBuffer& GetItemText(ItemHandle item, unsigned int subItemIndex);
        bool SetItemCheck(ItemHandle item, bool check);
        bool SetItemSelect(ItemHandle item, bool select);
        bool SetItemColor(ItemHandle item, AppCUI::Graphics::ColorPair color);
        bool SetItemType(ItemHandle item, ListViewItemType type);
        bool IsItemChecked(ItemHandle item);
        bool IsItemSelected(ItemHandle item);

        bool SetItemData(ItemHandle item, unsigned long long value);
        unsigned long long GetItemData(ItemHandle item, unsigned long long errorValue);

        template <typename T>
        constexpr inline bool SetItemData(ItemHandle item, Reference<T> obj)
        {
            return this->SetItemDataAsPointer(item, obj.ToGenericRef());
        }
        template <typename T>
        constexpr inline Reference<T> GetItemData(ItemHandle item) const
        {
            return this->GetItemDataAsPointer(item).ToReference<T>();
        }
        bool SetItemXOffset(ItemHandle item, unsigned int XOffset);
        unsigned int GetItemXOffset(ItemHandle item);
        bool SetItemHeight(ItemHandle item, unsigned int Height);
        unsigned int GetItemHeight(ItemHandle item);
        void DeleteAllItems();
        unsigned int GetItemsCount();
        ItemHandle GetCurrentItem();
        bool SetCurrentItem(ItemHandle item);
        void SelectAllItems();
        void UnSelectAllItems();
        void CheckAllItems();
        void UncheckAllItems();
        unsigned int GetCheckedItemsCount();

        // misc
        void SetClipboardSeparator(char ch);

        // sort
        void SetItemCompareFunction(Handlers::ListViewItemComparer fnc, void* Context = nullptr);
        bool Sort();
        bool Sort(unsigned int columnIndex, bool ascendent);
        bool Sort(
              unsigned int columnIndex, bool ascendent, Handlers::ListViewItemComparer fnc, void* Context = nullptr);

        virtual ~ListView();

        friend Factory::ListView;
        friend Control;
    };

    class EXPORT ComboBox : public Control
    {
      private:
        GenericRef GetItemDataAsPointer(unsigned int index) const;
        bool SetItemDataAsPointer(unsigned int index, GenericRef obj);
        bool AddItem(const AppCUI::Utils::ConstString& caption, GenericRef userData);

      protected:
        ComboBox(std::string_view layout, const AppCUI::Utils::ConstString& text, char itemsSeparator);

      public:
        static const unsigned int NO_ITEM_SELECTED = 0xFFFFFFFF;

        inline unsigned long long GetCurrentItemUserData(unsigned long long errorValue) const
        {
            return GetItemUserData(GetCurrentItemIndex(), errorValue);
        }
        template <typename T>
        inline Reference<T> GetCurrentItemUserData() const
        {
            return GetItemDataAsPointer(GetCurrentItemIndex()).ToReference<T>();
        }

        unsigned int GetItemsCount() const;
        unsigned int GetCurrentItemIndex() const;
        const AppCUI::Graphics::CharacterBuffer& GetCurrentItemText();

        unsigned long long GetItemUserData(unsigned int index, unsigned long long errorValue) const;
        template <typename T>
        inline Reference<T> GetItemUserData(unsigned int index) const
        {
            return GetItemDataAsPointer(index).ToReference<T>();
        }

        const AppCUI::Graphics::CharacterBuffer& GetItemText(unsigned int index);

        bool SetItemUserData(unsigned int index, unsigned long long userData);
        template <typename T>
        inline bool SetItemUserData(unsigned int index, Reference<T> userData)
        {
            return SetItemDataAsPointer(index, userData.ToGenericRef());
        }
        bool SetCurentItemIndex(unsigned int index);
        void SetNoIndexSelected();

        template <typename T>
        inline bool AddItem(const AppCUI::Utils::ConstString& caption, Reference<T> obj)
        {
            return AddItem(caption, obj.ToGenericRef());
        }
        bool AddItem(const AppCUI::Utils::ConstString& caption, unsigned long long usedData);
        inline bool AddItem(const AppCUI::Utils::ConstString& caption)
        {
            return AddItem(caption, GenericRef(nullptr));
        }

        bool AddSeparator(const AppCUI::Utils::ConstString& caption = "");
        void DeleteAllItems();

        bool OnKeyEvent(AppCUI::Input::Key keyCode, char16_t UnicodeChar) override;
        void OnHotKey() override;
        bool OnMouseLeave() override;
        bool OnMouseEnter() override;
        bool OnMouseOver(int x, int y) override;
        void OnMousePressed(int x, int y, AppCUI::Input::MouseButton button) override;
        bool OnMouseWheel(int x, int y, AppCUI::Input::MouseWheel direction) override;
        void Paint(Graphics::Renderer& renderer) override;
        void OnExpandView(AppCUI::Graphics::Clip& expandedClip) override;
        void OnPackView() override;
        virtual ~ComboBox();

        friend Factory::ComboBox;
        friend Control;
    };

    class EXPORT Menu
    {
      public:
        void* Context;

        Menu();
        Menu(const Menu& obj) = delete;
        ~Menu();

        ItemHandle AddCommandItem(
              const AppCUI::Utils::ConstString& text,
              int CommandID,
              AppCUI::Input::Key shortcutKey = AppCUI::Input::Key::None);
        ItemHandle AddCheckItem(
              const AppCUI::Utils::ConstString& text,
              int CommandID,
              bool checked                   = false,
              AppCUI::Input::Key shortcutKey = AppCUI::Input::Key::None);
        ItemHandle AddRadioItem(
              const AppCUI::Utils::ConstString& text,
              int CommandID,
              bool checked                   = false,
              AppCUI::Input::Key shortcutKey = AppCUI::Input::Key::None);
        ItemHandle AddSeparator();
        ItemHandle AddSubMenu(const AppCUI::Utils::ConstString& text);

        Reference<Menu> GetSubMenu(ItemHandle menuItem);

        bool SetEnable(ItemHandle menuItem, bool status);
        bool SetChecked(ItemHandle menuItem, bool status);

        void Show(int x, int y, const AppCUI::Graphics::Size& maxSize = { 0, 0 });
        void Show(
              Reference<Control> parent,
              int relativeX,
              int relativeY,
              const AppCUI::Graphics::Size& maxSize = { 0, 0 });
    };

    class EXPORT NumericSelector : public Control
    {
      protected:
        NumericSelector(const long long minValue, const long long maxValue, long long value, std::string_view layout);

      public:
        long long GetValue() const;
        void SetValue(const long long value);
        void SetMinValue(const long long minValue);
        void SetMaxValue(const long long maxValue);

      private:
        bool IsValidValue(const long long value) const;
        bool IsValueInsertedWrong() const;
        bool GetRenderColor(Graphics::ColorPair& color) const;
        bool FormatTextField();
        bool IsOnPlusButton(const int x, const int y) const;
        bool IsOnMinusButton(const int x, const int y) const;
        bool IsOnTextField(const int x, const int y) const;
        bool MinValueReached() const;
        bool MaxValueReached() const;

      public:
        void Paint(Graphics::Renderer& renderer) override;
        bool OnKeyEvent(AppCUI::Input::Key keyCode, char16_t UnicodeChar) override;
        void OnMousePressed(int x, int y, AppCUI::Input::MouseButton button) override;
        void OnMouseReleased(int x, int y, AppCUI::Input::MouseButton button) override;
        bool OnMouseWheel(int x, int y, AppCUI::Input::MouseWheel direction) override;
        bool OnMouseEnter() override;
        bool OnMouseLeave() override;
        bool OnMouseDrag(int x, int y, AppCUI::Input::MouseButton button) override;
        bool OnMouseOver(int x, int y) override;
        void OnLoseFocus() override;

        friend Factory::NumericSelector;
        friend Control;
    };

    class EXPORT Desktop : public Control
    {
      protected:
        Desktop();

      public:
        void Paint(AppCUI::Graphics::Renderer& renderer) override;
        bool OnKeyEvent(AppCUI::Input::Key keyCode, char16_t UnicodeChar) override;

        friend Factory::Desktop;
        friend Control;
    };

    class EXPORT SingleApp : public Desktop
    {
    };

    enum class TreeFlags : unsigned int
    {
        None                            = 0x000000,
        HideColumns                     = 0x000100, // not implemented
        HideBorder                      = 0x000200,
        HideColumnsSeparator            = 0x000400, // not implemented
        Sortable                        = 0x000800, // not implemented
        DynamicallyPopulateNodeChildren = 0x001000,
        HideScrollBar                   = 0x002000,
        Searchable                      = 0x004000, // shows all elements highlighting the ones matching
        FilterSearch  = 0x008000, // shows only the elements matching the criteria from the previous search action
        HideSearchBar = 0x010000, // disables FilterMode & SearchMode
        // Reserved_020000                 = 0x020000,
        // Reserved_040000                 = 0x040000,
        // Reserved_080000                 = 0x080000,
        // Reserved_100000                 = 0x100000,
        // Reserved_200000                 = 0x200000,
        // Reserved_400000                 = 0x400000,
        // Reserved_800000                 = 0x800000
    };

    namespace Factory
    {
        class EXPORT Label
        {
            Label() = delete;

          public:
            static Reference<AppCUI::Controls::Label> Create(
                  AppCUI::Controls::Control* parent,
                  const AppCUI::Utils::ConstString& caption,
                  std::string_view layout);
            static Reference<AppCUI::Controls::Label> Create(
                  AppCUI::Controls::Control& parent,
                  const AppCUI::Utils::ConstString& caption,
                  std::string_view layout);
            static Pointer<AppCUI::Controls::Label> Create(
                  const AppCUI::Utils::ConstString& caption, std::string_view layout);
        };
        class EXPORT Button
        {
            Button() = delete;

          public:
            static Reference<AppCUI::Controls::Button> Create(
                  AppCUI::Controls::Control* parent,
                  const AppCUI::Utils::ConstString& caption,
                  std::string_view layout,
                  int controlID     = 0,
                  ButtonFlags flags = ButtonFlags::None);
            static Reference<AppCUI::Controls::Button> Create(
                  AppCUI::Controls::Control& parent,
                  const AppCUI::Utils::ConstString& caption,
                  std::string_view layout,
                  int controlID     = 0,
                  ButtonFlags flags = ButtonFlags::None);
            static Pointer<AppCUI::Controls::Button> Create(
                  const AppCUI::Utils::ConstString& caption,
                  std::string_view layout,
                  int controlID                       = 0,
                  AppCUI::Controls::ButtonFlags flags = AppCUI::Controls::ButtonFlags::None);
        };
        class EXPORT Password
        {
            Password() = delete;

          public:
            static Reference<AppCUI::Controls::Password> Create(
                  AppCUI::Controls::Control* parent,
                  const AppCUI::Utils::ConstString& caption,
                  std::string_view layout);
            static Reference<AppCUI::Controls::Password> Create(
                  AppCUI::Controls::Control& parent,
                  const AppCUI::Utils::ConstString& caption,
                  std::string_view layout);
            static Pointer<AppCUI::Controls::Password> Create(
                  const AppCUI::Utils::ConstString& caption, std::string_view layout);
        };
        class EXPORT CheckBox
        {
            CheckBox() = delete;

          public:
            static Reference<AppCUI::Controls::CheckBox> Create(
                  AppCUI::Controls::Control* parent,
                  const AppCUI::Utils::ConstString& caption,
                  std::string_view layout,
                  int controlID = 0);
            static Reference<AppCUI::Controls::CheckBox> Create(
                  AppCUI::Controls::Control& parent,
                  const AppCUI::Utils::ConstString& caption,
                  std::string_view layout,
                  int controlID = 0);
            static Pointer<AppCUI::Controls::CheckBox> Create(
                  const AppCUI::Utils::ConstString& caption, std::string_view layout, int controlID = 0);
        };
        class EXPORT RadioBox
        {
            RadioBox() = delete;

          public:
            static Pointer<AppCUI::Controls::RadioBox> Create(
                  const AppCUI::Utils::ConstString& caption, std::string_view layout, int groupID, int controlID = 0);
            static Reference<AppCUI::Controls::RadioBox> Create(
                  AppCUI::Controls::Control* parent,
                  const AppCUI::Utils::ConstString& caption,
                  std::string_view layout,
                  int groupID,
                  int controlID = 0);
            static Reference<AppCUI::Controls::RadioBox> Create(
                  AppCUI::Controls::Control& parent,
                  const AppCUI::Utils::ConstString& caption,
                  std::string_view layout,
                  int groupID,
                  int controlID = 0);
        };
        class EXPORT Splitter
        {
            Splitter() = delete;

          public:
            static Reference<AppCUI::Controls::Splitter> Create(
                  AppCUI::Controls::Control* parent, std::string_view layout, bool vertical);
            static Reference<AppCUI::Controls::Splitter> Create(
                  AppCUI::Controls::Control& parent, std::string_view layout, bool vertical);
            static Pointer<AppCUI::Controls::Splitter> Create(std::string_view layout, bool vertical);
        };
        class EXPORT Panel
        {
            Panel() = delete;

          public:
            static Reference<AppCUI::Controls::Panel> Create(
                  AppCUI::Controls::Control* parent,
                  const AppCUI::Utils::ConstString& caption,
                  std::string_view layout);
            static Reference<AppCUI::Controls::Panel> Create(
                  AppCUI::Controls::Control& parent,
                  const AppCUI::Utils::ConstString& caption,
                  std::string_view layout);
            static Pointer<AppCUI::Controls::Panel> Create(
                  const AppCUI::Utils::ConstString& caption, std::string_view layout);
            static Reference<AppCUI::Controls::Panel> Create(
                  AppCUI::Controls::Control* parent, std::string_view layout);
            static Reference<AppCUI::Controls::Panel> Create(
                  AppCUI::Controls::Control& parent, std::string_view layout);
            static Pointer<AppCUI::Controls::Panel> Create(std::string_view layout);
        };
        class EXPORT TextField
        {
            TextField() = delete;

          public:
            static Reference<AppCUI::Controls::TextField> Create(
                  AppCUI::Controls::Control* parent,
                  const AppCUI::Utils::ConstString& caption,
                  std::string_view layout,
                  AppCUI::Controls::TextFieldFlags flags = AppCUI::Controls::TextFieldFlags::None);
            static Reference<AppCUI::Controls::TextField> Create(
                  AppCUI::Controls::Control& parent,
                  const AppCUI::Utils::ConstString& caption,
                  std::string_view layout,
                  AppCUI::Controls::TextFieldFlags flags = AppCUI::Controls::TextFieldFlags::None);
            static Pointer<AppCUI::Controls::TextField> Create(
                  const AppCUI::Utils::ConstString& caption,
                  std::string_view layout,
                  AppCUI::Controls::TextFieldFlags flags = AppCUI::Controls::TextFieldFlags::None);
        };
        class EXPORT TextArea
        {
            TextArea() = delete;

          public:
            static Reference<AppCUI::Controls::TextArea> Create(
                  AppCUI::Controls::Control* parent,
                  const AppCUI::Utils::ConstString& caption,
                  std::string_view layout,
                  AppCUI::Controls::TextAreaFlags flags = AppCUI::Controls::TextAreaFlags::None);
            static Reference<AppCUI::Controls::TextArea> Create(
                  AppCUI::Controls::Control& parent,
                  const AppCUI::Utils::ConstString& caption,
                  std::string_view layout,
                  AppCUI::Controls::TextAreaFlags flags = AppCUI::Controls::TextAreaFlags::None);
            static Pointer<AppCUI::Controls::TextArea> Create(
                  const AppCUI::Utils::ConstString& caption,
                  std::string_view layout,
                  AppCUI::Controls::TextAreaFlags flags = AppCUI::Controls::TextAreaFlags::None);
        };
        class EXPORT TabPage
        {
            TabPage() = delete;

          public:
            static Reference<AppCUI::Controls::TabPage> Create(
                  AppCUI::Controls::Control* parent, const AppCUI::Utils::ConstString& caption);
            static Reference<AppCUI::Controls::TabPage> Create(
                  AppCUI::Controls::Control& parent, const AppCUI::Utils::ConstString& caption);
            static Pointer<AppCUI::Controls::TabPage> Create(const AppCUI::Utils::ConstString& caption);
        };
        class EXPORT Tab
        {
            Tab() = delete;

          public:
            static Reference<AppCUI::Controls::Tab> Create(
                  AppCUI::Controls::Control* parent,
                  std::string_view layout,
                  AppCUI::Controls::TabFlags flags = AppCUI::Controls::TabFlags::TopTabs,
                  unsigned int tabPageSize         = 16);
            static Reference<AppCUI::Controls::Tab> Create(
                  AppCUI::Controls::Control& parent,
                  std::string_view layout,
                  AppCUI::Controls::TabFlags flags = AppCUI::Controls::TabFlags::TopTabs,
                  unsigned int tabPageSize         = 16);
            static Pointer<AppCUI::Controls::Tab> Create(
                  std::string_view layout,
                  AppCUI::Controls::TabFlags flags = AppCUI::Controls::TabFlags::TopTabs,
                  unsigned int tabPageSize         = 16);
        };
        class EXPORT CanvasViewer
        {
            CanvasViewer() = delete;

          public:
            static Reference<AppCUI::Controls::CanvasViewer> Create(
                  AppCUI::Controls::Control* parent,
                  std::string_view layout,
                  unsigned int canvasWidth,
                  unsigned int canvasHeight,
                  AppCUI::Controls::ViewerFlags flags = AppCUI::Controls::ViewerFlags::None);
            static Reference<AppCUI::Controls::CanvasViewer> Create(
                  AppCUI::Controls::Control& parent,
                  std::string_view layout,
                  unsigned int canvasWidth,
                  unsigned int canvasHeight,
                  AppCUI::Controls::ViewerFlags flags = AppCUI::Controls::ViewerFlags::None);
            static Pointer<AppCUI::Controls::CanvasViewer> Create(
                  std::string_view layout,
                  unsigned int canvasWidth,
                  unsigned int canvasHeight,
                  AppCUI::Controls::ViewerFlags flags = ViewerFlags::None);
            static Reference<AppCUI::Controls::CanvasViewer> Create(
                  AppCUI::Controls::Control* parent,
                  const AppCUI::Utils::ConstString& caption,
                  std::string_view layout,
                  unsigned int canvasWidth,
                  unsigned int canvasHeight,
                  AppCUI::Controls::ViewerFlags flags = ViewerFlags::None);
            static Reference<AppCUI::Controls::CanvasViewer> Create(
                  AppCUI::Controls::Control& parent,
                  const AppCUI::Utils::ConstString& caption,
                  std::string_view layout,
                  unsigned int canvasWidth,
                  unsigned int canvasHeight,
                  AppCUI::Controls::ViewerFlags flags = ViewerFlags::None);
            static Pointer<AppCUI::Controls::CanvasViewer> Create(
                  const AppCUI::Utils::ConstString& caption,
                  std::string_view layout,
                  unsigned int canvasWidth,
                  unsigned int canvasHeight,
                  AppCUI::Controls::ViewerFlags flags = ViewerFlags::None);
        };
        class EXPORT ImageViewer
        {
            ImageViewer() = delete;

          public:
            static Pointer<AppCUI::Controls::ImageViewer> Create(
                  std::string_view layout, AppCUI::Controls::ViewerFlags flags = AppCUI::Controls::ViewerFlags::None);
            static Reference<AppCUI::Controls::ImageViewer> Create(
                  AppCUI::Controls::Control* parent,
                  std::string_view layout,
                  AppCUI::Controls::ViewerFlags flags = AppCUI::Controls::ViewerFlags::None);
            static Reference<AppCUI::Controls::ImageViewer> Create(
                  AppCUI::Controls::Control& parent,
                  std::string_view layout,
                  AppCUI::Controls::ViewerFlags flags = AppCUI::Controls::ViewerFlags::None);
            static Pointer<AppCUI::Controls::ImageViewer> Create(
                  const AppCUI::Utils::ConstString& caption,
                  std::string_view layout,
                  AppCUI::Controls::ViewerFlags flags = AppCUI::Controls::ViewerFlags::None);
            static Reference<AppCUI::Controls::ImageViewer> Create(
                  AppCUI::Controls::Control* parent,
                  const AppCUI::Utils::ConstString& caption,
                  std::string_view layout,
                  AppCUI::Controls::ViewerFlags flags = AppCUI::Controls::ViewerFlags::None);
            static Reference<AppCUI::Controls::ImageViewer> Create(
                  AppCUI::Controls::Control& parent,
                  const AppCUI::Utils::ConstString& caption,
                  std::string_view layout,
                  AppCUI::Controls::ViewerFlags flags = AppCUI::Controls::ViewerFlags::None);
        };
        class EXPORT ListView
        {
            ListView() = delete;

          public:
            static Pointer<AppCUI::Controls::ListView> Create(
                  std::string_view layout,
                  AppCUI::Controls::ListViewFlags flags = AppCUI::Controls::ListViewFlags::None);
            static Reference<AppCUI::Controls::ListView> Create(
                  AppCUI::Controls::Control* parent,
                  std::string_view layout,
                  AppCUI::Controls::ListViewFlags flags = AppCUI::Controls::ListViewFlags::None);
            static Reference<AppCUI::Controls::ListView> Create(
                  AppCUI::Controls::Control& parent,
                  std::string_view layout,
                  AppCUI::Controls::ListViewFlags flags = AppCUI::Controls::ListViewFlags::None);
        };
        class EXPORT ComboBox
        {
            ComboBox() = delete;

          public:
            static Pointer<AppCUI::Controls::ComboBox> Create(
                  std::string_view layout,
                  const AppCUI::Utils::ConstString& text = std::string_view(),
                  char itemsSeparator                    = ',');

            static Reference<AppCUI::Controls::ComboBox> Create(
                  AppCUI::Controls::Control* parent,
                  std::string_view layout,
                  const AppCUI::Utils::ConstString& text = std::string_view(),
                  char itemsSeparator                    = ',');
            static Reference<AppCUI::Controls::ComboBox> Create(
                  AppCUI::Controls::Control& parent,
                  std::string_view layout,
                  const AppCUI::Utils::ConstString& text = std::string_view(),
                  char itemsSeparator                    = ',');
        };
        class EXPORT NumericSelector
        {
            NumericSelector() = delete;

          public:
            static Pointer<AppCUI::Controls::NumericSelector> Create(
                  const long long minValue, const long long maxValue, long long value, std::string_view layout);
            static Reference<AppCUI::Controls::NumericSelector> Create(
                  AppCUI::Controls::Control* parent,
                  const long long minValue,
                  const long long maxValue,
                  long long value,
                  std::string_view layout);
            static Reference<AppCUI::Controls::NumericSelector> Create(
                  AppCUI::Controls::Control& parent,
                  const long long minValue,
                  const long long maxValue,
                  long long value,
                  std::string_view layout);
        };
        class EXPORT Window
        {
            Window() = delete;

          public:
            static Pointer<AppCUI::Controls::Window> Create(
                  const AppCUI::Utils::ConstString& caption,
                  std::string_view layout,
                  AppCUI::Controls::WindowFlags windowFlags = AppCUI::Controls::WindowFlags::None);
        };
        class EXPORT Desktop
        {
            Desktop() = delete;

          public:
            static Pointer<AppCUI::Controls::Desktop> Create();
        };

        class EXPORT Tree
        {
            Tree() = delete;

          public:
            static Pointer<AppCUI::Controls::Tree> Create(
                  std::string_view layout,
                  const AppCUI::Controls::TreeFlags flags = AppCUI::Controls::TreeFlags::None,
                  const unsigned int noOfColumns          = 1);
            static Reference<AppCUI::Controls::Tree> Create(
                  Control* parent,
                  std::string_view layout,
                  const AppCUI::Controls::TreeFlags flags = AppCUI::Controls::TreeFlags::None,
                  const unsigned int noOfColumns          = 1);
            static Reference<AppCUI::Controls::Tree> Create(
                  Control& parent,
                  std::string_view layout,
                  const AppCUI::Controls::TreeFlags flags = AppCUI::Controls::TreeFlags::None,
                  const unsigned int noOfColumns          = 1);
        };
    } // namespace Factory

    class EXPORT Tree : public Control
    {
      private:
        GenericRef GetItemDataAsPointer(const ItemHandle item) const;
        bool SetItemDataAsPointer(ItemHandle item, GenericRef obj);

      protected:
        Tree(std::string_view layout, const TreeFlags flags = TreeFlags::None, const unsigned int noOfColumns = 1);

      public:
        void Paint(Graphics::Renderer& renderer) override;
        bool OnKeyEvent(AppCUI::Input::Key keyCode, char16_t UnicodeChar) override;
        void OnFocus() override;
        void OnMousePressed(int x, int y, AppCUI::Input::MouseButton button) override;
        bool OnMouseOver(int x, int y) override;
        bool OnMouseWheel(int x, int y, AppCUI::Input::MouseWheel direction) override;
        void OnUpdateScrollBars() override;
        void OnAfterResize(int newWidth, int newHeight) override;

        // handlers covariant
        Handlers::Tree* Handlers() override;

        ItemHandle AddItem(
              const ItemHandle parent,
              const std::vector<Graphics::CharacterBuffer>& values,
              const ConstString metadata,
              bool process      = false,
              bool isExpandable = false);
        bool RemoveItem(const ItemHandle handle, bool process = false);
        bool ClearItems();
        ItemHandle GetCurrentItem();
        const ConstString GetItemText(const ItemHandle handle);

        bool SetItemData(ItemHandle item, unsigned long long value);
        template <typename T>
        constexpr inline bool SetItemData(ItemHandle item, Reference<T> obj)
        {
            return this->SetItemDataAsPointer(item, obj.ToGenericRef());
        }

        template <typename T>
        Reference<T> GetItemData(const ItemHandle item)
        {
            return GetItemDataAsPointer(item).ToReference<T>();
        }
        unsigned long long GetItemData(const size_t index, unsigned long long errorValue);
        ItemHandle GetItemHandleByIndex(const unsigned int index) const;

        unsigned int GetItemsCount() const;
        bool AddColumnData(
              const unsigned int index,
              const ConstString title,
              const AppCUI::Graphics::TextAlignament headerAlignment,
              const AppCUI::Graphics::TextAlignament contentAlignment,
              const unsigned int width = 0xFFFFFFFF);
        const AppCUI::Utils::UnicodeStringBuilder& GetItemMetadata(ItemHandle handle);
        bool SetItemMetadata(ItemHandle handle, const AppCUI::Utils::ConstString& metadata);

      private:
        bool ItemsPainting(Graphics::Renderer& renderer, const ItemHandle ih) const;
        bool PaintColumnHeaders(Graphics::Renderer& renderer);
        bool PaintColumnSeparators(Graphics::Renderer& renderer);
        bool MoveUp();
        bool MoveDown();
        bool ProcessItemsToBeDrawn(const ItemHandle handle, bool clear = true);
        bool IsAncestorOfChild(const ItemHandle ancestor, const ItemHandle child) const;
        bool ToggleExpandRecursive(const ItemHandle handle);
        bool ToggleItem(const ItemHandle handle);
        bool IsMouseOnToggleSymbol(int x, int y) const;
        bool IsMouseOnItem(int x, int y) const;
        bool IsMouseOnBorder(int x, int y) const;
        bool IsMouseOnColumnHeader(int x, int y) const;
        bool IsMouseOnColumnSeparator(int x, int y) const;
        bool IsMouseOnSearchField(int x, int y) const;
        bool AdjustElementsOnResize(const int newWidth, const int newHeight);
        bool AdjustItemsBoundsOnResize();
        bool AddToColumnWidth(const unsigned int columnIndex, const int value);
        bool SetColorForItems(const AppCUI::Graphics::ColorPair& color);
        bool SearchItems();
        bool ProcessOrderedItems(const ItemHandle handle, const bool clear = true);
        bool MarkAllItemsAsNotFound();
        bool MarkAllAncestorsWithChildFoundInFilterSearch(const ItemHandle handle);

        friend Factory::Tree;
        friend Control;
    };

}; // namespace Controls

namespace Dialogs
{
    class EXPORT MessageBox
    {
        MessageBox() = delete;

      public:
        static void ShowError(const AppCUI::Utils::ConstString& title, const AppCUI::Utils::ConstString& message);
        static void ShowNotification(
              const AppCUI::Utils::ConstString& title, const AppCUI::Utils::ConstString& message);
        static void ShowWarning(const AppCUI::Utils::ConstString& title, const AppCUI::Utils::ConstString& message);
        static Result ShowOkCancel(const AppCUI::Utils::ConstString& title, const AppCUI::Utils::ConstString& message);
        static Result ShowYesNoCancel(
              const AppCUI::Utils::ConstString& title, const AppCUI::Utils::ConstString& message);
    };

    class EXPORT FileDialog
    {
        FileDialog() = delete;

        // Add additional extension filters so that FileDialog will show only the specified extensions,
        // other extensions will be filtered. If no filter is passed (empty string) - "All files" is chosen
        //
        // Filter format is: <Name>:ext|<Name>:ext| ...
        //               or: <Name>:ext1,ext2,ext3|<Name>:ext|....
        //
        // For example:
        //       "Text Files:txt|Images:jpg,jpeg,png|Documents:pdf,doc,docx,xlsx,xls,ppt,pptx"
        //
        // Will show "Text Files" and, if selected, only .txt files will be shown
        // If the user selects "Images" - .jpg, .jpeg and .png files will be shown

      public:
        static std::optional<std::filesystem::path> ShowSaveFileWindow(
              const AppCUI::Utils::ConstString& fileName,
              const AppCUI::Utils::ConstString& extensionsFilter,
              const std::filesystem::path& path);
        static std::optional<std::filesystem::path> ShowOpenFileWindow(
              const AppCUI::Utils::ConstString& fileName,
              const AppCUI::Utils::ConstString& extensionsFilter,
              const std::filesystem::path& path);
    };
    class EXPORT WindowManager
    {
        WindowManager() = delete;

      public:
        static void Show();
    };
} // namespace Dialogs

namespace Log
{
    enum class Severity : unsigned int
    {
        Fatal         = 4,
        InternalError = 3,
        Error         = 2,
        Warning       = 1,
        Information   = 0,
    };
    struct Message
    {
        Severity Type;
        const char* Content;
        const char* FileName;
        const char* Function;
        const char* Condition;
        int LineNumber;
    };
    void EXPORT Report(
          Severity type,
          const char* fileName,
          const char* function,
          const char* condition,
          int line,
          const char* format,
          ...);
    void EXPORT SetLogCallback(void (*callback)(const Message&));
    bool EXPORT ToFile(const std::filesystem::path& fileName);
    bool EXPORT ToOutputDebugString();
    bool EXPORT ToStdErr();
    bool EXPORT ToStdOut();
} // namespace Log
namespace Application
{
    enum class InitializationFlags : unsigned int
    {
        None = 0,

        CommandBar          = 0x0001,
        Menu                = 0x0002,
        Maximized           = 0x0004,
        Fullscreen          = 0x0008,
        FixedSize           = 0x0010,
        LoadSettingsFile    = 0x0020,
        AutoHotKeyForWindow = 0x0040,
        EnableFPSMode       = 0x0080,
        SingleWindowApp     = 0x0100,
    };

    enum class CharacterSize : unsigned int
    {
        Default = 0,
        Tiny,
        Small,
        Normal,
        Large,
        Huge
    };
    enum class FrontendType : unsigned int
    {
        Default        = 0,
        SDL            = 1,
        Terminal       = 2,
        WindowsConsole = 3
    };

    struct InitializationData
    {
        unsigned int Width, Height;
        FrontendType Frontend;
        CharacterSize CharSize;
        InitializationFlags Flags;
        std::string_view FontName;
        AppCUI::Controls::Desktop* (*CustomDesktopConstructor)();

        InitializationData()
            : Width(0), Height(0), Frontend(FrontendType::Default), CharSize(CharacterSize::Default),
              Flags(InitializationFlags::None), FontName(""), CustomDesktopConstructor(nullptr)
        {
        }
    };

    enum class ArangeWindowsMethod
    {
        MaximizedAll,
        Cascade,
        Vertical,
        Horizontal,
        Grid
    };

    class EXPORT CommandBar
    {
        void* Controller;

      public:
        CommandBar();
        void Init(void* controller);
        bool SetCommand(AppCUI::Input::Key keyCode, const AppCUI::Utils::ConstString& caption, int CommandID);
    };

    struct Config
    {
        struct
        {
            int DesktopFillCharacterCode;
            Graphics::ColorPair Color;
        } Desktop;
        struct
        {
            Graphics::ColorPair BackgroundColor;
            Graphics::ColorPair ShiftKeysColor;
            struct
            {
                Graphics::ColorPair KeyColor;
                Graphics::ColorPair NameColor;
            } Normal, Hover, Pressed;
        } CommandBar;
        struct
        {
            Graphics::ColorPair ActiveColor;
            Graphics::ColorPair InactiveColor;
            Graphics::ColorPair TitleActiveColor;
            Graphics::ColorPair TitleInactiveColor;
            struct
            {
                struct
                {
                    struct
                    {
                        Graphics::ColorPair Text, HotKey;
                    } Normal, Pressed, Hover, Checked, Focused;
                } Item;
                struct
                {
                    Graphics::ColorPair Normal, Focused;
                } Separators;
                Graphics::ColorPair CloseButton, Tag, CheckMark, Text;
            } ControlBar;
        } Window, DialogError, DialogNotify, DialogWarning;
        struct
        {
            Graphics::ColorPair NormalColor;
            Graphics::ColorPair HotKeyColor;
        } Label;
        struct
        {
            struct
            {
                Graphics::ColorPair TextColor, HotKeyColor;
            } Normal, Focused, Inactive, Hover;
        } Button;
        struct
        {
            struct
            {
                Graphics::ColorPair TextColor, HotKeyColor, StateSymbolColor;
            } Normal, Focused, Inactive, Hover;
        } StateControl;
        struct
        {
            Graphics::ColorPair NormalColor, ClickColor, HoverColor;
            struct
            {
                Graphics::ColorPair Normal, Hover, Clicked;
            } Buttons;

        } Splitter;
        struct
        {
            Graphics::ColorPair NormalColor, Text;
        } Panel;
        struct
        {
            struct
            {
                Graphics::ColorPair Text, Border, LineNumbers, CurrentLineNumber;
            } Normal, Focus, Inactive, Hover;
            Graphics::ColorPair SelectionColor;
        } Text;
        struct
        {
            struct
            {
                Graphics::ColorPair Text, VisibleSign;
            } Normal, Focus, Inactive, Hover;
        } Password;
        struct
        {
            Graphics::ColorPair PageColor, TabBarColor, HoverColor, PageHotKeyColor, TabBarHotKeyColor,
                  HoverHotKeyColor;
            Graphics::ColorPair ListSelectedPageColor, ListSelectedPageHotKey;
        } Tab;
        struct
        {
            struct
            {
                Graphics::ColorPair Border, Text, Hotkey;
            } Normal, Focused, Inactive, Hover;
            Graphics::ColorPair InactiveCanvasColor;
        } View;
        struct
        {
            Graphics::ColorPair Bar, Arrows, Position;
        } ScrollBar;
        struct
        {
            struct
            {
                Graphics::ColorPair Border, LineSparators;
            } Normal, Focused, Inactive, Hover;
            struct
            {
                Graphics::ColorPair Text, HotKey, Separator;
            } ColumnNormal, ColumnHover, ColumnInactive, ColumnSort;
            struct
            {
                Graphics::ColorPair Regular, Highligheted, Inactive, Error, Warning;
            } Item;
            struct
            {
                Graphics::ColorPair Selected, Normal;
            } Highlight;
            Graphics::ColorPair CheckedSymbol, UncheckedSymbol;
            Graphics::ColorPair InactiveColor;
            Graphics::ColorPair FocusColor;
            Graphics::ColorPair SelectionColor;
            Graphics::ColorPair FocusAndSelectedColor;
            Graphics::ColorPair FilterText;
            Graphics::ColorPair StatusColor;

        } ListView;
        struct
        {
            Graphics::ColorPair Border, Title, TerminateMessage, Text, Time, Percentage;
            Graphics::ColorPair EmptyProgressBar, FullProgressBar;
        } ProgressStatus;
        struct
        {
            struct
            {
                Graphics::ColorPair Text, Button;
            } Focus, Normal, Inactive, Hover;
            Graphics::ColorPair Selection, HoverOveItem;
        } ComboBox;
        struct
        {
            struct
            {
                Graphics::ColorPair Background;
                struct
                {
                    Graphics::ColorPair Text, HotKey, ShortCut, Check, Uncheck;
                } Normal, Inactive, Selected;
                struct
                {
                    Graphics::ColorPair Normal, Hover, Inactive, Pressed;
                } Button;
            } Activ, Parent;
        } Menu;
        struct
        {
            Graphics::ColorPair BackgroundColor;
            struct
            {
                Graphics::ColorPair HotKeyColor;
                Graphics::ColorPair NameColor;
            } Normal, Hover, Pressed;
        } MenuBar;
        struct
        {
            struct
            {
                Graphics::ColorPair Normal, Focused, Inactive, Hover, WrongValue;
            } Text;
        } NumericSelector;
        struct
        {
            Graphics::ColorPair Text, Arrow;
        } ToolTip;
        struct
        {
            struct
            {
                Graphics::ColorPair Border;
                struct
                {
                    Graphics::ColorPair Normal, Focused;
                } Separator;
                struct
                {
                    Graphics::ColorPair Text, Header;
                } Column;
                struct
                {
                    Graphics::ColorPair Normal, Focused, Inactive, Filter, SearchActive;
                } Text;
                struct
                {
                    Graphics::ColorPair Expanded, Collapsed, SingleElement;
                } Symbol;
            };
        } Tree;
        void SetDarkTheme();
    };

    EXPORT Config* GetAppConfig();
    EXPORT AppCUI::Utils::IniObject* GetAppSettings();

    NODISCARD("Check the return of the Init function. If false, AppCUI has not been initialized properly")
    EXPORT bool Init(Application::InitializationFlags flags = Application::InitializationFlags::None);

    NODISCARD("Check the return of the Init function. If false, AppCUI has not been initialized properly")
    EXPORT bool Init(InitializationData& initData);

    EXPORT bool Run();
    EXPORT bool RunSingleApp(std::unique_ptr<AppCUI::Controls::SingleApp> singleApp);
    EXPORT AppCUI::Controls::ItemHandle AddWindow(
          std::unique_ptr<AppCUI::Controls::Window> wnd,
          AppCUI::Controls::ItemHandle referal = AppCUI::Controls::InvalidItemHandle);
    EXPORT AppCUI::Controls::ItemHandle AddWindow(
          std::unique_ptr<AppCUI::Controls::Window> wnd, AppCUI::Controls::Window* referalWindow);
    EXPORT AppCUI::Controls::Menu* AddMenu(const AppCUI::Utils::ConstString& name);
    EXPORT bool GetApplicationSize(AppCUI::Graphics::Size& size);
    EXPORT bool GetDesktopSize(AppCUI::Graphics::Size& size);
    EXPORT void Repaint();
    EXPORT void RecomputeControlsLayout();
    EXPORT void ArrangeWindows(ArangeWindowsMethod method);
    EXPORT void RaiseEvent(
          AppCUI::Utils::Reference<AppCUI::Controls::Control> control,
          AppCUI::Utils::Reference<AppCUI::Controls::Control> sourceControl,
          AppCUI::Controls::Event eventType,
          int controlID);
    EXPORT void Close();
}; // namespace Application
} // namespace AppCUI

// inline operations for enum classes
ADD_FLAG_OPERATORS(AppCUI::Application::InitializationFlags, unsigned int)
ADD_FLAG_OPERATORS(AppCUI::Input::Key, unsigned int);
ADD_FLAG_OPERATORS(AppCUI::Input::MouseButton, unsigned int);
ADD_FLAG_OPERATORS(AppCUI::Graphics::WriteTextFlags, unsigned int)
ADD_FLAG_OPERATORS(AppCUI::Graphics::TextAlignament, unsigned int);
ADD_FLAG_OPERATORS(AppCUI::Controls::TextAreaFlags, unsigned int);
ADD_FLAG_OPERATORS(AppCUI::Controls::ListViewFlags, unsigned int);
ADD_FLAG_OPERATORS(AppCUI::Controls::TabFlags, unsigned int)
ADD_FLAG_OPERATORS(AppCUI::Controls::WindowFlags, unsigned int)
ADD_FLAG_OPERATORS(AppCUI::Controls::ButtonFlags, unsigned int)
ADD_FLAG_OPERATORS(AppCUI::Controls::TextFieldFlags, unsigned int)
ADD_FLAG_OPERATORS(AppCUI::Utils::NumberParseFlags, unsigned int)
ADD_FLAG_OPERATORS(AppCUI::Utils::NumericFormatFlags, unsigned short)
ADD_FLAG_OPERATORS(AppCUI::Controls::TreeFlags, unsigned int)

#undef ADD_FLAG_OPERATORS
