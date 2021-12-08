#pragma once

// Version MUST be in the following format <Major>.<Minor>.<Patch>
#define APPCUI_VERSION "1.0.0"

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
// basic types
using uint64 = uint64_t;
using uint32 = uint32_t;
using uint16 = uint16_t;
using uint8  = uint8_t;
using int64  = int64_t;
using int32  = int32_t;
using int16  = int16_t;
using int8   = int8_t;
using char8  = char8_t;
using char16 = char16_t;

namespace StdIncludes
{
    using std::initializer_list;
    using std::optional;
    using std::string_view;
    using std::u16string_view;
    using std::u8string_view;
    using std::unique_ptr;
    using std::variant;
    using std::vector;
} // namespace StdIncludes

using namespace StdIncludes;

namespace Graphics
{
    enum class Color : uint8
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
                char16 Code;
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
        inline constexpr bool operator==(char16 value) const
        {
            return Code == value;
        }
        inline constexpr bool operator!=(char16 value) const
        {
            return Code != value;
        }
        inline constexpr bool operator>(char value) const
        {
            return Code > value;
        }
        inline constexpr bool operator>(char16 value) const
        {
            return Code > value;
        }
        inline constexpr bool operator<(char value) const
        {
            return Code < value;
        }
        inline constexpr bool operator<(char16 value) const
        {
            return Code < value;
        }
        inline constexpr bool operator>=(char value) const
        {
            return Code >= value;
        }
        inline constexpr bool operator>=(char16 value) const
        {
            return Code >= value;
        }
        inline constexpr bool operator<=(char value) const
        {
            return Code <= value;
        }
        inline constexpr bool operator<=(char16 value) const
        {
            return Code <= value;
        }
        inline constexpr operator char16() const
        {
            return Code;
        }
    };
    class EXPORT CharacterBuffer;
}; // namespace Graphics
namespace Utils
{
    using CharacterView = std::basic_string_view<Graphics::Character>;
    using ConstString   = variant<string_view, u8string_view, u16string_view, CharacterView>;
    template <typename T>
    class Pointer : public unique_ptr<T>
    {
      public:
        Pointer(T* obj) : unique_ptr<T>(obj)
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
using Utils::ConstString;
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
    enum class Key : uint32
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
    enum class MouseButton : uint32
    {
        None          = 0,
        Left          = 0x01,
        Center        = 0x02,
        Right         = 0x04,
        DoubleClicked = 0x08,
    };
    enum class MouseWheel : uint32
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
        uint32* Data;
        uint32 Count, Allocated;

        bool Grow(uint32 newSize);

      public:
        Array32();
        ~Array32();
        void Destroy();

        bool Create(uint32 allocatedCount = 64);
        bool Create(uint32* vector, uint32 vectorSize, uint32 elementsCount = 0);
        bool Create(int32* vector, uint32 vectorSize, uint32 elementsCount = 0);

        inline uint32* GetUInt32Array() const
        {
            return Data;
        }
        inline int32* GetInt32Array() const
        {
            return (int32*) Data;
        }
        inline uint32 Len() const
        {
            return Count;
        }
        inline uint32 GetAllocatedSize() const
        {
            return Allocated & 0x7FFFFFFF;
        }

        inline void Clear()
        {
            Count = 0;
        }
        bool Reserve(uint32 newSize);
        bool Resize(uint32 newSize);
        bool Push(uint32 value);
        bool Push(int32 value);
        bool Get(uint32 index, uint32& value);
        bool Get(uint32 index, int32& value);

        bool Sort(int32 (*compare)(int32 elem1, int32 elem2, void* Context), bool ascendent, void* Context = nullptr);
        bool Sort(int32 (*compare)(uint32 elem1, uint32 elem2, void* Context), bool ascendent, void* Context = nullptr);
    };

    class BufferView
    {
        const uint8* data;
        const size_t length;

      public:
        BufferView() : data(nullptr), length(0)
        {
        }
        BufferView(const void* ptr, size_t len) : data((const uint8*) ptr), length(len)
        {
        }
        BufferView(string_view txt) : data((const uint8*) txt.data()), length(txt.size())
        {
        }
        inline uint8 operator[](size_t index) const
        {
            return data[index];
        }
        inline bool IsValid() const
        {
            return length > 0;
        }
        inline bool Empty() const
        {
            return length == 0;
        }
        inline size_t GetLength() const
        {
            return length;
        }
        inline const uint8* GetData() const
        {
            return data;
        }
        template <typename T>
        inline Reference<const T> GetObject(unsigned int offset = 0) const
        {
            if (sizeof(T) + offset > length)
                return nullptr;
            return reinterpret_cast<const T*>(data + offset);
        }

        inline operator string_view() const
        {
            return string_view((const char*) data, length);
        }
        // iterators
        inline const uint8* begin() const
        {
            return data;
        }
        inline const uint8* end() const
        {
            return data + length;
        }
    };

    class EXPORT Buffer
    {
        uint8* data;
        size_t length;

      public:
        ~Buffer();
        Buffer() : data(nullptr), length(0)
        {
        }
        Buffer(size_t size);
        Buffer(const Buffer& buf);

        Buffer(void*& ptr, size_t size)
        {
            data   = (uint8*) ptr;
            length = size;
            ptr    = nullptr;
        }
        Buffer(char*& ptr, size_t size)
        {
            data   = (uint8*) ptr;
            length = size;
            ptr    = nullptr;
        }
        Buffer(uint8*& ptr, size_t size)
        {
            data   = (uint8*) ptr;
            length = size;
            ptr    = nullptr;
        }
        Buffer(Buffer&& buf) noexcept
        {
            data       = buf.data;
            length     = buf.length;
            buf.data   = nullptr;
            buf.length = 0;
        }

        inline Buffer& operator=(Buffer&& b) noexcept
        {
            std::swap(data, b.data);
            std::swap(length, b.length);
            return *this;
        }
        inline uint8& operator[](size_t index) const
        {
            return data[index];
        }
        inline operator BufferView() const
        {
            return BufferView((const void*) data, length);
        }
        inline operator string_view() const
        {
            return string_view((const char*) data, length);
        }
        inline bool IsValid() const
        {
            return length > 0;
        }
        inline size_t GetLength() const
        {
            return length;
        }
        inline uint8* GetData() const
        {
            return data;
        }
        template <typename T>
        inline Reference<T> GetObject(unsigned int offset = 0)
        {
            if (sizeof(T) + offset > length)
                return nullptr;
            return reinterpret_cast<T*>(data + offset);
        }
        void Resize(size_t newSize);
        Buffer& operator=(const Buffer& b);
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
        static bool StartsWith(string_view sir1, string_view sir2, bool ignoreCase = false);
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
        string_view Format(const char* format, ...);

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
        inline operator string_view() const
        {
            return string_view{ this->Text, this->Size };
        }
        inline string_view ToStringView() const
        {
            return string_view{ this->Text, this->Size };
        }
        char& operator[](int poz);
    };
    class EXPORT UnicodeStringBuilder
    {
        char16* chars;
        unsigned int length;
        unsigned int allocated;

        void Create(char16* localBuffer, size_t localBufferSize);

      public:
        UnicodeStringBuilder();
        UnicodeStringBuilder(char16* localBuffer, size_t localBufferSize);
        UnicodeStringBuilder(const ConstString& text);
        UnicodeStringBuilder(char16* localBuffer, size_t localBufferSize, const ConstString& text);
        UnicodeStringBuilder(const Graphics::CharacterBuffer& charBuffer);
        UnicodeStringBuilder(char16* localBuffer, size_t localBufferSize, const Graphics::CharacterBuffer& charBuffer);

        UnicodeStringBuilder(const UnicodeStringBuilder& obj);
        UnicodeStringBuilder(UnicodeStringBuilder&& obj) noexcept;

        ~UnicodeStringBuilder();
        void Destroy();

        bool Set(const ConstString& text);
        bool Set(const Graphics::CharacterBuffer& charBuffer);
        bool Add(const ConstString& text);
        bool Add(const Graphics::CharacterBuffer& charBuffer);
        bool AddChar(char16 ch);
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
        inline const char16* GetString() const
        {
            return chars;
        }
        inline u16string_view ToStringView() const
        {
            return u16string_view{ chars, (size_t) length };
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
        inline operator u16string_view() const
        {
            return u16string_view{ chars, (size_t) length };
        }
        inline UnicodeStringBuilder& operator+=(const ConstString& text)
        {
            Add(text);
            return *this;
        }
        inline UnicodeStringBuilder& operator+=(const Graphics::CharacterBuffer& charBuffer)
        {
            Add(charBuffer);
            return *this;
        }
        inline UnicodeStringBuilder& operator=(const ConstString& text)
        {
            Set(text);
            return *this;
        }
        inline UnicodeStringBuilder& operator=(const Graphics::CharacterBuffer& charBuffer)
        {
            Set(charBuffer);
            return *this;
        }

        UnicodeStringBuilder& operator=(const UnicodeStringBuilder& obj);
        UnicodeStringBuilder& operator=(UnicodeStringBuilder&& obj) noexcept;
    };

    enum class NumberParseFlags : uint32
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
        EXPORT optional<uint64> ToUInt64(
              string_view text, NumberParseFlags flags = NumberParseFlags::None, unsigned int* size = nullptr);
        EXPORT optional<unsigned int> ToUInt32(
              string_view text, NumberParseFlags flags = NumberParseFlags::None, unsigned int* size = nullptr);
        EXPORT optional<uint16> ToUInt16(
              string_view text, NumberParseFlags flags = NumberParseFlags::None, unsigned int* size = nullptr);
        EXPORT optional<uint8> ToUInt8(
              string_view text, NumberParseFlags flags = NumberParseFlags::None, unsigned int* size = nullptr);

        EXPORT optional<int64> ToInt64(
              string_view text, NumberParseFlags flags = NumberParseFlags::None, unsigned int* size = nullptr);
        EXPORT optional<int> ToInt32(
              string_view text, NumberParseFlags flags = NumberParseFlags::None, unsigned int* size = nullptr);
        EXPORT optional<int16> ToInt16(
              string_view text, NumberParseFlags flags = NumberParseFlags::None, unsigned int* size = nullptr);
        EXPORT optional<char> ToInt8(
              string_view text, NumberParseFlags flags = NumberParseFlags::None, unsigned int* size = nullptr);

        EXPORT optional<float> ToFloat(
              string_view text, NumberParseFlags flags = NumberParseFlags::None, unsigned int* size = nullptr);
        EXPORT optional<double> ToDouble(
              string_view text, NumberParseFlags flags = NumberParseFlags::None, unsigned int* size = nullptr);

    }; // namespace Number

    enum class NumericFormatFlags : uint16
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
        uint8 Base;
        uint8 GroupSize;
        char GroupSeparator;
        uint8 DigitsCount;
        NumericFormat(NumericFormatFlags flags)
            : Flags(flags), Base(10), GroupSize(0), GroupSeparator(0), DigitsCount(0)
        {
        }
        NumericFormat(NumericFormatFlags flags, uint8 base)
            : Flags(flags), Base(base), GroupSize(0), GroupSeparator(0), DigitsCount(0)
        {
        }
        NumericFormat(NumericFormatFlags flags, uint8 base, uint8 groupSize, char groupSeparator)
            : Flags(flags), Base(base), GroupSize(groupSize), GroupSeparator(groupSeparator), DigitsCount(0)
        {
        }
        NumericFormat(NumericFormatFlags flags, uint8 base, uint8 groupSize, char groupSeparator, uint8 digitsCount)
            : Flags(flags), Base(base), GroupSize(groupSize), GroupSeparator(groupSeparator), DigitsCount(digitsCount)
        {
        }
    };
    class EXPORT NumericFormatter
    {
        char temp[72]; // a minimum of 65 chars must be allocated to support 64 bits for binary translation
        char* heapBuffer;
        string_view ToHexString(uint64 value);
        string_view ToOctString(uint64 value);
        string_view ToBinString(uint64 value);
        string_view ToDecStringUnsigned(uint64 value);
        string_view ToDecStringSigned(int64 value);
        string_view ToBaseUnsigned(uint64 value, int base);
        string_view ToBaseSigned(int64 value, int base);
        string_view ToGenericBase(uint64 value, uint64 base);
        string_view ToStringUnsigned(uint64 value, NumericFormat fmt);
        string_view ToStringSigned(int64 value, NumericFormat fmt);

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
        inline string_view ToHex(uint64 value)
        {
            return ToHexString(value);
        }
        inline string_view ToHex(unsigned int value)
        {
            return ToHexString((uint64) value);
        }
        inline string_view ToHex(uint16 value)
        {
            return ToHexString((uint64) value);
        }
        inline string_view ToHex(uint8 value)
        {
            return ToHexString((uint64) value);
        }
        inline string_view ToHex(int64 value)
        {
            return ToHexString(*(uint64*) &value);
        }
        inline string_view ToHex(int value)
        {
            return ToHexString((uint64) (*(unsigned int*) &value));
        }
        inline string_view ToHex(int16 value)
        {
            return ToHexString((uint64) (*(uint16*) &value));
        }
        inline string_view ToHex(char value)
        {
            return ToHexString((uint64) (*(uint8*) &value));
        }

        // ToDec
        inline string_view ToDec(uint64 value)
        {
            return ToDecStringUnsigned(value);
        }
        inline string_view ToDec(unsigned int value)
        {
            return ToDecStringUnsigned((uint64) value);
        }
        inline string_view ToDec(uint16 value)
        {
            return ToDecStringUnsigned((uint64) value);
        }
        inline string_view ToDec(uint8 value)
        {
            return ToDecStringUnsigned((uint64) value);
        }
        inline string_view ToDec(int64 value)
        {
            return ToDecStringSigned(value);
        }
        inline string_view ToDec(int value)
        {
            return ToDecStringSigned((int64) value);
        }
        inline string_view ToDec(int16 value)
        {
            return ToDecStringSigned((int64) value);
        }
        inline string_view ToDec(char value)
        {
            return ToDecStringSigned((int64) value);
        }
        string_view ToDec(float value);
        string_view ToDec(double value);

        // ToOct
        inline string_view ToOct(uint64 value)
        {
            return ToOctString(value);
        }
        inline string_view ToOct(unsigned int value)
        {
            return ToOctString((uint64) value);
        }
        inline string_view ToOct(uint16 value)
        {
            return ToOctString((uint64) value);
        }
        inline string_view ToOct(uint8 value)
        {
            return ToOctString((uint64) value);
        }
        inline string_view ToOct(int64 value)
        {
            return ToOctString(*(uint64*) &value);
        }
        inline string_view ToOct(int value)
        {
            return ToOctString((uint64) (*(unsigned int*) &value));
        }
        inline string_view ToOct(int16 value)
        {
            return ToOctString((uint64) (*(uint16*) &value));
        }
        inline string_view ToOct(char value)
        {
            return ToOctString((uint64) (*(uint8*) &value));
        }

        // ToBin
        inline string_view ToBin(uint64 value)
        {
            return ToBinString(value);
        }
        inline string_view ToBin(unsigned int value)
        {
            return ToBinString((uint64) value);
        }
        inline string_view ToBin(uint16 value)
        {
            return ToBinString((uint64) value);
        }
        inline string_view ToBin(uint8 value)
        {
            return ToBinString((uint64) value);
        }
        inline string_view ToBin(int64 value)
        {
            return ToBinString(*(uint64*) &value);
        }
        inline string_view ToBin(int value)
        {
            return ToBinString((uint64) (*(unsigned int*) &value));
        }
        inline string_view ToBin(int16 value)
        {
            return ToBinString((uint64) (*(uint16*) &value));
        }
        inline string_view ToBin(char value)
        {
            return ToBinString((uint64) (*(uint8*) &value));
        }

        // ToBase
        inline string_view ToBase(uint64 value, int base)
        {
            return ToBaseUnsigned(value, base);
        }
        inline string_view ToBase(unsigned int value, int base)
        {
            return ToBaseUnsigned((uint64) value, base);
        }
        inline string_view ToBase(uint16 value, int base)
        {
            return ToBaseUnsigned((uint64) value, base);
        }
        inline string_view ToBase(uint8 value, int base)
        {
            return ToBaseUnsigned((uint64) value, base);
        }
        inline string_view ToBase(int64 value, int base)
        {
            return ToBaseSigned(value, base);
        }
        inline string_view ToBase(int value, int base)
        {
            return ToBaseSigned((int64) value, base);
        }
        inline string_view ToBase(int16 value, int base)
        {
            return ToBaseSigned((int64) value, base);
        }
        inline string_view ToBase(char value, int base)
        {
            return ToBaseSigned((int64) value, base);
        }

        // ToString
        inline string_view ToString(uint64 value, NumericFormat fmt)
        {
            return ToStringUnsigned(value, fmt);
        }
        inline string_view ToString(unsigned int value, NumericFormat fmt)
        {
            return ToStringUnsigned((uint64) value, fmt);
        }
        inline string_view ToString(uint16 value, NumericFormat fmt)
        {
            return ToStringUnsigned((uint64) value, fmt);
        }
        inline string_view ToString(uint8 value, NumericFormat fmt)
        {
            return ToStringUnsigned((uint64) value, fmt);
        }
        inline string_view ToString(int64 value, NumericFormat fmt)
        {
            return ToStringSigned(value, fmt);
        }
        inline string_view ToString(int value, NumericFormat fmt)
        {
            return ToStringSigned((int64) value, fmt);
        }
        inline string_view ToString(int16 value, NumericFormat fmt)
        {
            return ToStringSigned((int64) value, fmt);
        }
        inline string_view ToString(char value, NumericFormat fmt)
        {
            return ToStringSigned((int64) value, fmt);
        }
    };

    enum class StringEncoding : uint32
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
                BuildFromAlternative<string_view>(obj, StringEncoding::Ascii);
                break;
            case 1:
                BuildFromAlternative<u8string_view>(obj, StringEncoding::UTF8);
                break;
            case 2:
                BuildFromAlternative<u16string_view>(obj, StringEncoding::Unicode16);
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
        char16 tempBuffer[size];

      public:
        LocalUnicodeStringBuilder() : UnicodeStringBuilder(tempBuffer, size)
        {
        }
        LocalUnicodeStringBuilder(const Graphics::CharacterBuffer& charBuffer)
            : UnicodeStringBuilder(tempBuffer, size, charBuffer)
        {
        }
        LocalUnicodeStringBuilder(const ConstString& text) : UnicodeStringBuilder(tempBuffer, size, text)
        {
        }
    };
    template <uint16 Size>
    class FixSizeString
    {
        static_assert(Size > 0);
        char data[Size + 1];
        uint16 size;

      public:
        FixSizeString() : size(0)
        {
            data[0] = 0;
        }
        FixSizeString(string_view txt)
        {
            Set(txt);
        }
        constexpr inline operator string_view() const
        {
            return string_view{ data, size };
        }
        constexpr inline operator bool() const
        {
            return this->size != 0;
        }
        void Set(string_view txt)
        {
            size = (uint16) std::min((size_t) Size, txt.length());
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
                Set(string_view{ text, static_cast<size_t>(e - text) });
            }
            else
            {
                Clear();
            }
        }
        constexpr inline uint16 Len() const
        {
            return size;
        }
        constexpr inline const char* GetText() const
        {
            return data;
        }
        constexpr inline uint16 MaxSize() const
        {
            return Size;
        }
        inline FixSizeString& operator=(string_view txt)
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
        static string_view GetKeyName(Input::Key keyCode);
        static string_view GetKeyModifierName(Input::Key keyCode);
        static string_view GetKeyNamePadded(Input::Key keyCode);
        static bool ToString(Input::Key keyCode, char* text, int maxTextSize);
        static bool ToString(Input::Key keyCode, Utils::String& text);
        static Input::Key FromString(string_view stringRepresentation);

        static Input::Key CreateHotKey(char16 hotKey, Input::Key modifier = Input::Key::None);
    };

    class EXPORT IniValueArray
    {
        const char* text;
        unsigned int len;

      public:
        IniValueArray() : text(nullptr), len(0)
        {
        }
        IniValueArray(string_view obj) : text(obj.data()), len((unsigned int) obj.size())
        {
        }

        optional<uint64> AsUInt64() const;
        optional<int64> AsInt64() const;
        optional<unsigned int> AsUInt32() const;
        optional<int> AsInt32() const;
        optional<bool> AsBool() const;
        optional<Input::Key> AsKey() const;
        inline optional<const char*> AsString() const
        {
            return text;
        }
        inline optional<string_view> AsStringView() const
        {
            return string_view(text, len);
        };
        optional<Graphics::Size> AsSize() const;
        optional<float> AsFloat() const;
        optional<double> AsDouble() const;

        uint64 ToUInt64(uint64 defaultValue = 0) const;
        unsigned int ToUInt32(unsigned int defaultValue = 0) const;
        int64 ToInt64(int64 defaultValue = -1) const;
        int ToInt32(int defaultValue = -1) const;
        bool ToBool(bool defaultValue = false) const;
        Input::Key ToKey(Input::Key defaultValue = Input::Key::None) const;
        const char* ToString(const char* defaultValue = nullptr) const;
        string_view ToStringView(string_view defaultValue = string_view{}) const;
        Graphics::Size ToSize(Graphics::Size defaultValue = Graphics::Size()) const;
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

        optional<uint64> AsUInt64() const;
        optional<int64> AsInt64() const;
        optional<unsigned int> AsUInt32() const;
        optional<int> AsInt32() const;
        optional<bool> AsBool() const;
        optional<Input::Key> AsKey() const;
        optional<const char*> AsString() const;
        optional<string_view> AsStringView() const;
        optional<Graphics::Size> AsSize() const;
        optional<float> AsFloat() const;
        optional<double> AsDouble() const;

        uint64 ToUInt64(uint64 defaultValue = 0) const;
        unsigned int ToUInt32(unsigned int defaultValue = 0) const;
        int64 ToInt64(int64 defaultValue = -1) const;
        int ToInt32(int defaultValue = -1) const;
        bool ToBool(bool defaultValue = false) const;
        Input::Key ToKey(Input::Key defaultValue = Input::Key::None) const;
        const char* ToString(const char* defaultValue = nullptr) const;
        string_view ToStringView(string_view defaultValue = string_view{}) const;
        Graphics::Size ToSize(Graphics::Size defaultValue = Graphics::Size()) const;
        float ToFloat(float defaultValue = 0.0f) const;
        double ToDouble(double defaultValue = 0.0) const;

        bool IsArray() const;
        unsigned int GetArrayCount() const;
        IniValueArray operator[](int index) const;

        string_view GetName() const;

        inline bool HasValue() const
        {
            return Data != nullptr;
        }

        void operator=(bool value);
        void operator=(unsigned int value);
        void operator=(uint64 value);
        void operator=(int value);
        void operator=(int64 value);
        void operator=(float value);
        void operator=(double value);
        void operator=(const char* value);
        void operator=(string_view value);
        void operator=(Graphics::Size value);
        void operator=(Input::Key value);
        void operator=(const initializer_list<const char*>& values);
        void operator=(const initializer_list<std::string>& values);
        void operator=(const initializer_list<bool>& values);
        void operator=(const initializer_list<unsigned int>& values);
        void operator=(const initializer_list<uint64>& values);
        void operator=(const initializer_list<int>& values);
        void operator=(const initializer_list<int64>& values);
        void operator=(const initializer_list<float>& values);
        void operator=(const initializer_list<double>& values);
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
        string_view GetName() const;
        IniValue GetValue(string_view keyName);
        vector<IniValue> GetValues() const;
        IniValue operator[](string_view keyName);

        void Clear();
        bool DeleteValue(string_view keyName);
        bool HasValue(string_view keyName);

        void UpdateValue(string_view name, bool value, bool dontUpdateIfValueExits);
        void UpdateValue(string_view name, unsigned int value, bool dontUpdateIfValueExits);
        void UpdateValue(string_view name, uint64 value, bool dontUpdateIfValueExits);
        void UpdateValue(string_view name, int value, bool dontUpdateIfValueExits);
        void UpdateValue(string_view name, int64 value, bool dontUpdateIfValueExits);
        void UpdateValue(string_view name, float value, bool dontUpdateIfValueExits);
        void UpdateValue(string_view name, double value, bool dontUpdateIfValueExits);
        void UpdateValue(string_view name, const char* value, bool dontUpdateIfValueExits);
        void UpdateValue(string_view name, string_view value, bool dontUpdateIfValueExits);
        void UpdateValue(string_view name, Graphics::Size value, bool dontUpdateIfValueExits);
        void UpdateValue(string_view name, Input::Key value, bool dontUpdateIfValueExits);
        void UpdateValue(string_view name, const initializer_list<std::string>& values, bool dontUpdateIfValueExits);
        void UpdateValue(string_view name, const initializer_list<const char*>& values, bool dontUpdateIfValueExits);
        void UpdateValue(string_view name, const initializer_list<bool>& values, bool dontUpdateIfValueExits);
        void UpdateValue(string_view name, const initializer_list<int>& values, bool dontUpdateIfValueExits);
        void UpdateValue(string_view name, const initializer_list<int64>& values, bool dontUpdateIfValueExits);
        void UpdateValue(string_view name, const initializer_list<unsigned int>& values, bool dontUpdateIfValueExits);
        void UpdateValue(string_view name, const initializer_list<uint64>& values, bool dontUpdateIfValueExits);
        void UpdateValue(string_view name, const initializer_list<float>& values, bool dontUpdateIfValueExits);
        void UpdateValue(string_view name, const initializer_list<double>& values, bool dontUpdateIfValueExits);
    };
    class EXPORT IniObject
    {
        void* Data;
        bool Init();

      public:
        IniObject();
        ~IniObject();

        bool CreateFromString(string_view text);
        bool CreateFromFile(const std::filesystem::path& fileName);
        bool Save(const std::filesystem::path& fileName);
        bool Create();
        void Clear();

        bool HasSection(string_view name) const;
        IniSection GetSection(string_view name);
        IniSection CreateSection(string_view name, bool emptyContent);
        inline IniSection operator[](string_view name)
        {
            return CreateSection(name, false);
        }
        IniValue GetValue(string_view valuePath);
        vector<IniSection> GetSections() const;
        unsigned int GetSectionsCount();

        bool DeleteSection(string_view name);
        bool DeleteValue(string_view valuePath);

        string_view ToString();
    };

}; // namespace Utils
namespace OS
{
    class EXPORT Clipboard
    {
        Clipboard() = delete;

      public:
        static bool SetText(const ConstString& text);
        static bool GetText(Utils::UnicodeStringBuilder& text);
        static bool Clear();
        static bool HasText();
    };
    class EXPORT IFile
    {
      public:
        virtual ~IFile();

        // virtual methods
        virtual bool ReadBuffer(void* buffer, unsigned int bufferSize, unsigned int& bytesRead);
        virtual bool WriteBuffer(const void* buffer, unsigned int bufferSize, unsigned int& bytesWritten);
        virtual uint64 GetSize();
        virtual uint64 GetCurrentPos();
        virtual bool SetSize(uint64 newSize);
        virtual bool SetCurrentPos(uint64 newPosition);
        virtual void Close();

        // other methods
        bool Read(void* buffer, unsigned int bufferSize, unsigned int& bytesRead);
        bool Write(const void* buffer, unsigned int bufferSize, unsigned int& bytesWritten);
        bool Read(void* buffer, unsigned int bufferSize);
        bool Write(const void* buffer, unsigned int bufferSize);
        bool Read(uint64 offset, void* buffer, unsigned int bufferSize, unsigned int& bytesRead);
        bool Write(uint64 offset, const void* buffer, unsigned int bufferSize, unsigned int& bytesWritten);
        bool Write(string_view text);
        bool Write(uint64 offset, string_view text, unsigned int& bytesWritten);
    };

    class EXPORT File : public IFile
    {
        union
        {
            void* Handle;
            uint64 u64Value;
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
        uint64 GetSize() override;
        uint64 GetCurrentPos() override;
        bool SetSize(uint64 newSize) override;
        bool SetCurrentPos(uint64 newPosition) override;
        void Close() override;

        static Utils::Buffer ReadContent(const std::filesystem::path& path);
        static bool WriteContent(const std::filesystem::path& path, Utils::BufferView buf);
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
    enum class SpecialFolder : uint32
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
    using RootsVector      = vector<FSLocationData>;

    // Fills the specialFolders map and roots vector with paths
    EXPORT void GetSpecialFolders(SpecialFolderMap& specialFolders, RootsVector& roots);
    EXPORT std::filesystem::path GetCurrentApplicationPath();

} // namespace OS
namespace Graphics
{
    enum class Alignament : uint8
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
    enum class SpecialChars : uint32
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

        // extended ascii codes (195 / 251C, 194 / 252C, 180 / 2524, 193 / 2534) / Graphics Extended Code Page 1252
        // https://en.wikipedia.org/wiki/Windows-1252
        BoxMidleLeft,
        BoxMidleTop,
        BoxMidleRight,
        BoxMidleBottom,

        // always last
        Count
    };

    enum class TextAlignament : uint32
    {
        Left   = 0x00,
        Center = 0x01,
        Right  = 0x02,
    };

    namespace ProgressStatus
    {
        void EXPORT Init(const ConstString& Title, uint64 maxValue = 0);
        bool EXPORT Update(uint64 value, const ConstString& content);
        bool EXPORT Update(uint64 value);
    }; // namespace ProgressStatus

    enum class WriteTextFlags : uint32
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

        bool Resize(unsigned int size, char16 character = ' ', const ColorPair color = NoColorPair);
        bool Fill(char16 character, unsigned int size, const ColorPair color = NoColorPair);
        bool Set(const CharacterBuffer& obj);
        bool Add(const ConstString& text, const ColorPair color = NoColorPair);
        bool Set(const ConstString& text, const ColorPair color = NoColorPair);
        bool SetWithHotKey(
              const ConstString& text,
              unsigned int& hotKeyCharacterPosition,
              Input::Key& hotKey,
              Input::Key hotKeyModifier = Input::Key::None,
              const ColorPair color     = NoColorPair);

        bool Delete(unsigned int start, unsigned int end);
        bool DeleteChar(unsigned int position);
        bool Insert(const ConstString& text, unsigned int position, const ColorPair color = NoColorPair);
        bool InsertChar(uint16 characterCode, unsigned int position, const ColorPair color = NoColorPair);
        bool SetColor(unsigned int start, unsigned int end, const ColorPair color);
        void SetColor(const ColorPair color);
        bool CopyString(Utils::String& text, unsigned int start, unsigned int end);
        bool CopyString(Utils::String& text);
        bool ConvertToUpper(unsigned int start, unsigned int end);
        bool ConvertToLower(unsigned int start, unsigned int end);

        int Find(const ConstString& text, bool ignoreCase = true) const;
        inline bool Contains(const ConstString& text, bool ignoreCase = true) const
        {
            return Find(text, ignoreCase) != -1;
        }
        int CompareWith(const CharacterBuffer& obj, bool ignoreCase = true) const;
        optional<unsigned int> FindNext(
              unsigned int startOffset, bool (*shouldSkip)(unsigned int offset, Character ch)) const;
        optional<unsigned int> FindPrevious(
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
        inline operator Utils::CharacterView() const
        {
            return Utils::CharacterView(Buffer, Count);
        }
        inline Utils::CharacterView SubString(size_t start, size_t end) const
        {
            if ((end > start) && (Buffer) && (end <= Count))
                return Utils::CharacterView{ Buffer + start, end - start };
            return Utils::CharacterView{ nullptr, 0 };
        }
    };

    enum class ImageRenderingMethod : uint32
    {
        PixelTo16ColorsSmallBlock,
        PixelTo64ColorsLargeBlock,
        GrayScale,
        AsciiArt
    };
    enum class ImageScaleMethod : uint32
    {
        NoScale = 1,
        Scale50 = 2,
        Scale33 = 3,
        Scale25 = 4,
        Scale20 = 5,
        Scale10 = 10,
        Scale5  = 20
    };
    struct EXPORT Pixel
    {
        union
        {
            unsigned int ColorValue;
            struct
            {
                uint8 Blue, Green, Red, Alpha;
            };
        };
        Pixel() : ColorValue(0)
        {
        }
        Pixel(uint8 red, uint8 green, uint8 blue) : Blue(blue), Green(green), Red(red), Alpha(255)
        {
        }
        explicit Pixel(unsigned int value) : ColorValue(value)
        {
        }
        inline unsigned int ToGrayScale() const
        {
            return (((((unsigned int) Red) + ((unsigned int) Blue) + ((unsigned int) Green))) * 100U) / (255U * 3);
        }
    };
    class EXPORT Image
    {
        Pixel* pixels;
        unsigned int width;
        unsigned int height;

      public:
        Image();
        Image(const Image& img);
        Image(Image&& img) noexcept
        {
            this->width  = img.width;
            this->height = img.height;
            this->pixels = img.pixels;
            img.pixels   = nullptr;
            img.width    = 0;
            img.height   = 0;
        }
        ~Image();
        bool Load(const std::filesystem::path& imageFilePath);
        bool Create(unsigned int width, unsigned int height);
        bool Create(unsigned int width, unsigned int height, string_view image);
        bool Create(const uint8* imageBuffer, unsigned int size);
        inline bool Create(Utils::BufferView buf)
        {
            if (buf.GetLength() <= 0xFFFFFFFF)
                return Create(buf.GetData(), (unsigned int) buf.GetLength());
            else
                return false;
        }
        bool CreateFromDIB(const uint8* imageBuffer, unsigned int size, bool isIcon);
        inline bool CreateFromDIB(Utils::BufferView buf, bool isIcon)
        {
            if (buf.GetLength() <= 0xFFFFFFFF)
                return CreateFromDIB(buf.GetData(), (unsigned int) buf.GetLength(), isIcon);
            else
                return false;
        }
        bool SetPixel(unsigned int x, unsigned int y, const Color color);
        bool SetPixel(unsigned int x, unsigned int y, Pixel colorRGB);

        Pixel GetPixel(unsigned int x, unsigned int y, Pixel invalidIndexValue = {}) const;
        bool GetPixel(unsigned int x, unsigned int y, Pixel& color) const;
        Pixel ComputeSquareAverageColor(unsigned int x, unsigned int y, unsigned int sz) const;
        bool Clear(Pixel color);
        bool Clear(const Color color);
        inline unsigned int GetWidth() const
        {
            return width;
        }
        inline unsigned int GetHeight() const
        {
            return height;
        }
        inline Pixel* GetPixelsBuffer() const
        {
            return pixels;
        }
        Image& operator=(const Image& img);
        Image& operator=(Image&& img) noexcept
        {
            std::swap(this->pixels, img.pixels);
            std::swap(this->width, img.width);
            std::swap(this->height, img.height);
            return *this;
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
        bool WriteText(const ConstString& text, const WriteTextParams& params);

        // Single line wrappers
        bool WriteSingleLineCharacterBuffer(
              int x, int y, Graphics::CharacterBuffer& charBuffer, bool noTransparency = true);
        bool WriteSingleLineText(int x, int y, const ConstString& text, ColorPair color);
        bool WriteSingleLineText(int x, int y, const ConstString& text, ColorPair color, TextAlignament align);
        bool WriteSingleLineText(
              int x, int y, const ConstString& text, ColorPair color, ColorPair hotKeyColor, unsigned int hotKeyOffset);
        bool WriteSingleLineText(
              int x,
              int y,
              const ConstString& text,
              ColorPair color,
              ColorPair hotKeyColor,
              unsigned int hotKeyOffset,
              TextAlignament align);
        bool WriteSingleLineText(int x, int y, unsigned int width, const ConstString& text, ColorPair color);
        bool WriteSingleLineText(
              int x, int y, unsigned int width, const ConstString& text, ColorPair color, TextAlignament align);
        bool WriteSingleLineText(
              int x,
              int y,
              unsigned int width,
              const ConstString& text,
              ColorPair color,
              ColorPair hotKeyColor,
              unsigned int hotKeyOffset);
        bool WriteSingleLineText(
              int x,
              int y,
              unsigned int width,
              const ConstString& text,
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
        void SetAbsoluteClip(const Graphics::Clip& clip);
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
        class EXPORT Grid;
    }; // namespace Factory
    enum class Event : uint32
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
        SplitterPositionChanged,
        Custom,
    };
    using ItemHandle                       = uint32;
    constexpr ItemHandle InvalidItemHandle = 0xFFFFFFFF;
    class EXPORT Control;
    class EXPORT Button;
    class EXPORT TextField;
    class EXPORT ListView;
    class EXPORT Tree;
    class EXPORT Menu;
    class EXPORT Window;
    class EXPORT Grid;

    using namespace Utils;

    namespace Handlers
    {
        using namespace Graphics;
        using namespace AppCUI;
        using namespace Input;

        using OnButtonPressedHandler = void (*)(Reference<Controls::Button> r);
        using PaintControlHandler    = void (*)(Reference<Controls::Control> control, Renderer& renderer);
        using OnEventHandler     = bool (*)(Reference<Controls::Control> control, Controls::Event eventType, int ID);
        using OnKeyEventHandler  = bool (*)(Reference<Controls::Control> control, Key keyCode, char16 unicodeChar);
        using OnCheckHandler     = void (*)(Reference<Controls::Control> control, bool value);
        using OnFocusHandler     = void (*)(Reference<Controls::Control> control);
        using OnLoseFocusHandler = void (*)(Reference<Controls::Control> control);
        using OnTreeItemToggleHandler = bool (*)(Reference<Controls::Tree> control, ItemHandle handle);
        using OnAfterSetTextHandler   = void (*)(Reference<Controls::Control> control);
        using OnTextRightClickHandler = void (*)(Reference<Controls::Control> control, int x, int y);
        using OnTextColorHandler = void (*)(Reference<Controls::Control> control, Character* chars, unsigned int len);

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
            virtual bool OnKeyEvent(Reference<Controls::Control> control, Key keyCode, char16 unicodeChar) = 0;
        };
        struct OnKeyEventCallback : public OnKeyEventInterface
        {
            OnKeyEventHandler callback;
            virtual bool OnKeyEvent(Reference<Controls::Control> control, Key keyCode, char16 unicodeChar) override
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

        struct OnTextRightClickInterface
        {
            virtual void OnTextRightClick(Reference<Controls::Control> ctrl, int x, int y) = 0;
        };
        struct OnTextRightClickCallback : public OnTextRightClickInterface
        {
            OnTextRightClickHandler callback;
            virtual void OnTextRightClick(Reference<Controls::Control> ctrl, int x, int y) override
            {
                callback(ctrl, x, y);
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

        struct OnAfterSetTextInterface
        {
            virtual void OnAfterSetText(Reference<Controls::Control> control) = 0;
        };
        struct OnAfterSetTextCallback : public OnAfterSetTextInterface
        {
            OnAfterSetTextHandler callback;
            virtual void OnAfterSetText(Reference<Controls::Control> control) override
            {
                callback(control);
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
            Wrapper<OnAfterSetTextInterface, OnAfterSetTextCallback, OnAfterSetTextHandler> OnAfterSetText;
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
            Wrapper<OnTextRightClickInterface, OnTextRightClickCallback, OnTextRightClickHandler> OnTextRightClick;
        };

        using ListViewItemComparer = int (*)(
              Controls::ListView* control, ItemHandle item1, ItemHandle item2, unsigned int columnIndex, void* Context);

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
        bool ShowToolTip(const ConstString& caption);
        bool ShowToolTip(const ConstString& caption, int x, int y);
        void HideToolTip();

        Reference<Control> AddChildControl(unique_ptr<Control> control);

        // protected constructor
        Control(void* context, const ConstString& caption, string_view layout, bool computeHotKey);

      public:
        template <typename T>
        Reference<T> AddControl(unique_ptr<T> control)
        {
            return this->AddChildControl(std::move(control)).template DownCast<T>();
        }
        template <typename T, typename... Arguments>
        Reference<T> CreateChildControl(Arguments... args)
        {
            return this->AddControl<T>(unique_ptr<T>(new T(std::forward<Arguments>(args)...)));
        }
        bool RemoveControl(Control* control);

        template <typename T>
        bool RemoveControl(Reference<T>& control)
        {
            if (RemoveControlByRef(control.template To<Controls::Control>()))
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
        void GetSize(Graphics::Size& size);
        void GetClientSize(Graphics::Size& size);
        void MoveTo(int newX, int newY);
        bool Resize(int newWidth, int newHeight);
        void RecomputeLayout();

        // groups
        int GetGroup();
        void SetGroup(int newGroupID);
        void ClearGroup();

        // hot key
        bool SetHotKey(char16 hotKey);
        Input::Key GetHotKey();
        unsigned int GetHotKeyTextOffset();
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
        bool SetText(const ConstString& caption, bool updateHotKey = false);
        bool SetText(const Graphics::CharacterBuffer& caption);
        bool SetTextWithHotKey(const ConstString& caption, unsigned int hotKeyTextOffset);
        const Graphics::CharacterBuffer& GetText();

        // Scroll bars
        void UpdateHScrollBar(uint64 value, uint64 maxValue);
        void UpdateVScrollBar(uint64 value, uint64 maxValue);

        // handlers
        virtual Handlers::Control* Handlers();

        // paint
        virtual void Paint(Graphics::Renderer& renderer);

        // virtual methods
        virtual bool OnKeyEvent(Input::Key keyCode, char16 UnicodeChar);
        virtual void OnHotKey();
        virtual void OnHotKeyChanged();
        virtual void OnFocus();
        virtual void OnLoseFocus();
        virtual bool OnFrameUpdate();

        virtual void OnMousePressed(int x, int y, Input::MouseButton button);
        virtual void OnMouseReleased(int x, int y, Input::MouseButton button);
        virtual bool OnMouseDrag(int x, int y, Input::MouseButton button);

        virtual bool OnMouseEnter();
        virtual bool OnMouseOver(int x, int y);
        virtual bool OnMouseLeave();
        virtual bool OnMouseWheel(int x, int y, Input::MouseWheel direction);

        virtual bool OnEvent(Reference<Control> sender, Event eventType, int controlID);
        virtual bool OnUpdateCommandBar(Application::CommandBar& commandBar);
        virtual void OnUpdateScrollBars();

        virtual bool OnBeforeResize(int newWidth, int newHeight);
        virtual void OnAfterResize(int newWidth, int newHeight);
        virtual bool OnBeforeAddControl(Reference<Control> ctrl);
        virtual void OnAfterAddControl(Reference<Control> ctrl);
        virtual void OnControlRemoved(Reference<Control> ctrl);
        virtual bool OnBeforeSetText(const ConstString& text);
        virtual void OnAfterSetText();

        virtual void OnExpandView(Graphics::Clip& expandedClip);
        virtual void OnPackView();

        virtual ~Control();
    };

    enum class WindowFlags : uint32
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
    enum class WindowControlsBarLayout : uint8
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
        ItemHandle AddCommandItem(const ConstString& name, int ID, const ConstString& toolTip = "");
        ItemHandle AddSingleChoiceItem(
              const ConstString& name, int ID, bool checked, const ConstString& toolTip = string_view());
        ItemHandle AddCheckItem(
              const ConstString& name, int ID, bool checked, const ConstString& toolTip = string_view());
        ItemHandle AddTextItem(const ConstString& caption, const ConstString& toolTip = "");
        bool SetItemText(ItemHandle itemHandle, const ConstString& caption);
        bool SetItemTextWithHotKey(ItemHandle itemHandle, const ConstString& caption, unsigned int hotKeyOffset);
        bool SetItemToolTip(ItemHandle itemHandle, const ConstString& toolTipText);
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
        Window(const ConstString& caption, string_view layout, WindowFlags windowsFlags);

      public:
        void Paint(Graphics::Renderer& renderer) override;
        void OnMousePressed(int x, int y, Input::MouseButton button) override;
        void OnMouseReleased(int x, int y, Input::MouseButton button) override;
        bool OnMouseDrag(int x, int y, Input::MouseButton button) override;
        bool OnMouseOver(int x, int y) override;
        bool OnMouseLeave() override;
        bool OnEvent(Reference<Control> sender, Event eventType, int controlID) override;
        void RemoveMe();

        int Show();
        int GetDialogResult();
        bool MaximizeRestore();
        void SetTag(const ConstString& name, const ConstString& toolTipText);
        const Graphics::CharacterBuffer& GetTag();
        bool OnBeforeResize(int newWidth, int newHeight) override;
        void OnAfterResize(int newWidth, int newHeight) override;
        bool CenterScreen();
        bool OnKeyEvent(Input::Key keyCode, char16 UnicodeChar) override;
        void OnHotKeyChanged() override;
        bool Exit(int dialogResult);
        bool Exit(Dialogs::Result dialogResult);
        bool IsWindowInResizeMode();

        Reference<Menu> AddMenu(const ConstString& name);
        WindowControlsBar GetControlBar(WindowControlsBarLayout layout);

        virtual ~Window();

        friend Factory::Window;
        friend Control;
    };
    class EXPORT Label : public Control
    {
        Label(const ConstString& caption, string_view layout);

      public:
        void Paint(Graphics::Renderer& renderer) override;
        friend Factory::Label;
        friend Control;
    };

    enum class ButtonFlags : uint32
    {
        None = 0,
        Flat = 0x000100,
    };
    class EXPORT Button : public Control
    {
      protected:
        Button(const ConstString& caption, string_view layout, int controlID, ButtonFlags flags);

      public:
        void OnMousePressed(int x, int y, Input::MouseButton button) override;
        void OnMouseReleased(int x, int y, Input::MouseButton button) override;
        bool OnMouseDrag(int x, int y, Input::MouseButton button) override;
        void Paint(Graphics::Renderer& renderer) override;
        bool OnKeyEvent(Input::Key keyCode, char16 UnicodeChar) override;
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
        CheckBox(const ConstString& caption, string_view layout, int controlID);

      public:
        void OnMouseReleased(int x, int y, Input::MouseButton button) override;
        void Paint(Graphics::Renderer& renderer) override;
        bool OnKeyEvent(Input::Key keyCode, char16 UnicodeChar) override;
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
        RadioBox(const ConstString& caption, string_view layout, int groupID, int controlID);

      public:
        void OnMouseReleased(int x, int y, Input::MouseButton button) override;
        void Paint(Graphics::Renderer& renderer) override;
        bool OnKeyEvent(Input::Key keyCode, char16 UnicodeChar) override;
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
        Splitter(string_view layout, bool vertical);

      public:
        void Paint(Graphics::Renderer& renderer) override;
        bool OnKeyEvent(Input::Key keyCode, char16 UnicodeChar) override;
        bool SetSecondPanelSize(int newSize);
        bool HideSecondPanel();
        bool MaximizeSecondPanel();
        void OnAfterResize(int newWidth, int newHeight) override;
        void OnFocus() override;
        bool OnBeforeAddControl(Reference<Control> ctrl) override;
        void OnAfterAddControl(Reference<Control> ctrl) override;
        void OnMousePressed(int x, int y, Input::MouseButton button) override;
        void OnMouseReleased(int x, int y, Input::MouseButton button) override;
        bool OnMouseOver(int x, int y) override;
        bool OnMouseDrag(int x, int y, Input::MouseButton button) override;
        bool OnMouseEnter() override;
        bool OnMouseLeave() override;

        unsigned int GetFirstPanelSize();
        unsigned int GetSecondPanelSize();

        virtual ~Splitter();

        friend Factory::Splitter;
        friend Control;
    };
    class EXPORT Password : public Control
    {
      protected:
        Password(const ConstString& caption, string_view layout);

      public:
        void OnMousePressed(int x, int y, Input::MouseButton button) override;
        void OnMouseReleased(int x, int y, Input::MouseButton button) override;
        bool OnMouseDrag(int x, int y, Input::MouseButton button) override;
        void Paint(Graphics::Renderer& renderer) override;
        bool OnKeyEvent(Input::Key keyCode, char16 UnicodeChar) override;
        bool OnMouseEnter() override;
        bool OnMouseLeave() override;

        friend Factory::Password;
        friend Control;
    };
    class EXPORT Panel : public Control
    {
      protected:
        Panel(const ConstString& caption, string_view layout);

      public:
        void Paint(Graphics::Renderer& renderer) override;

        friend Factory::Panel;
        friend Control;
    };
    enum class TextFieldFlags : uint32
    {
        None               = 0,
        ProcessEnter       = 0x000100,
        Readonly           = 0x000200,
        SyntaxHighlighting = 0x000400,
    };
    class EXPORT TextField : public Control
    {
      protected:
        TextField(const ConstString& caption, string_view layout, TextFieldFlags flags);

      public:
        bool OnKeyEvent(Input::Key keyCode, char16 UnicodeChar) override;
        void OnAfterSetText() override;
        void Paint(Graphics::Renderer& renderer) override;
        void OnFocus() override;
        void OnMousePressed(int x, int y, Input::MouseButton button) override;
        void OnMouseReleased(int x, int y, Input::MouseButton button) override;
        bool OnMouseDrag(int x, int y, Input::MouseButton button) override;
        bool OnMouseEnter() override;
        bool OnMouseLeave() override;
        void OnAfterResize(int newWidth, int newHeight) override;
        bool OnEvent(Reference<Control> sender, Event eventType, int controlID) override;

        // handlers covariant
        Handlers::TextControl* Handlers() override;

        // selection
        void SelectAll();
        void ClearSelection();
        bool HasSelection() const;
        bool GetSelection(unsigned int& start, unsigned int& size) const;

        // clipboard
        void CopyToClipboard(bool deleteSelectionAfterCopy);
        void PasteFromClipboard();

        virtual ~TextField();

        friend Factory::TextField;
        friend Control;
    };
    enum class TextAreaFlags : uint32
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
        TextArea(const ConstString& caption, string_view layout, TextAreaFlags flags);

      public:
        void Paint(Graphics::Renderer& renderer) override;
        bool OnKeyEvent(Input::Key keyCode, char16 UnicodeChar) override;
        void OnUpdateScrollBars() override;
        void OnFocus() override;
        void OnAfterResize(int newWidth, int newHeight) override;
        void OnAfterSetText() override;
        void SetReadOnly(bool value);
        bool IsReadOnly();
        void SetTabCharacter(char tabCharacter);
        virtual ~TextArea();

        // handlers covariant
        Handlers::TextControl* Handlers() override;

        friend Factory::TextArea;
        friend Control;
    };

    enum class TabFlags : uint32
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
        TabPage(const ConstString& caption);

      public:
        bool OnBeforeResize(int newWidth, int newHeight);

        friend Factory::TabPage;
        friend Control;
    };
    class EXPORT Tab : public Control
    {
      protected:
        Tab(string_view layout, TabFlags flags, unsigned int tabPageSize);

      public:
        bool SetCurrentTabPageByIndex(unsigned int index);
        bool SetCurrentTabPageByRef(Reference<Control> page);
        template <typename T>
        inline bool SetCurrentTabPage(Reference<T> page)
        {
            return SetCurrentTabPageByRef(page.template DownCast<Control>());
        }
        bool SetTabPageTitleSize(unsigned int newSize);
        bool SetTabPageName(unsigned int index, const ConstString& name);
        void OnAfterResize(int newWidth, int newHeight) override;
        void OnFocus() override;
        void OnMouseReleased(int x, int y, Input::MouseButton button) override;
        bool OnMouseLeave() override;
        bool OnMouseOver(int x, int y) override;
        bool OnKeyEvent(Input::Key keyCode, char16 UnicodeChar) override;
        void OnAfterAddControl(Reference<Control> ctrl) override;
        void Paint(Graphics::Renderer& renderer) override;
        Reference<Control> GetCurrentTab();

        friend Factory::Tab;
        friend Control;
    };
    class EXPORT UserControl : public Control
    {
      protected:
        UserControl(const ConstString& caption, string_view layout);
        UserControl(string_view layout);
    };
    enum class ViewerFlags : uint32
    {
        None   = 0,
        Border = 0x000100,
    };
    class EXPORT CanvasViewer : public Control
    {
      protected:
        CanvasViewer(
              const ConstString& caption,
              string_view layout,
              unsigned int canvasWidth,
              unsigned int canvasHeight,
              ViewerFlags flags);

      public:
        ~CanvasViewer();
        void Paint(Graphics::Renderer& renderer) override;
        bool OnKeyEvent(Input::Key keyCode, char16 UnicodeChar) override;
        bool OnMouseLeave() override;
        bool OnMouseEnter() override;
        bool OnMouseWheel(int x, int y, Input::MouseWheel direction) override;
        void OnMousePressed(int x, int y, Input::MouseButton button) override;
        bool OnMouseDrag(int x, int y, Input::MouseButton button) override;
        void OnMouseReleased(int x, int y, Input::MouseButton button) override;
        void OnUpdateScrollBars() override;
        Reference<Graphics::Canvas> GetCanvas();

        friend Factory::CanvasViewer;
        friend Control;
    };
    class EXPORT ImageViewer : public CanvasViewer
    {
      protected:
        ImageViewer(const ConstString& caption, string_view layout, ViewerFlags flags);

      public:
        bool SetImage(
              const Graphics::Image& img, Graphics::ImageRenderingMethod method, Graphics::ImageScaleMethod scale);

        friend Factory::ImageViewer;
        friend Control;
    };
    enum class ListViewFlags : uint32
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
    enum class ListViewItemType : uint16
    {
        Normal             = 0,
        Highlighted        = 1,
        GrayedOut          = 2,
        ErrorInformation   = 3,
        WarningInformation = 4,
        Emphasized_1       = 5,
        Emphasized_2       = 6,
        Category           = 7
    };

    class EXPORT ListView : public Control
    {
      private:
        GenericRef GetItemDataAsPointer(ItemHandle item) const;
        bool SetItemDataAsPointer(ItemHandle item, GenericRef obj);

      protected:
        ListView(string_view layout, ListViewFlags flags);

      public:
        bool Reserve(unsigned int itemsCount);
        void Paint(Graphics::Renderer& renderer) override;
        bool OnKeyEvent(Input::Key keyCode, char16 UnicodeChar) override;
        void OnMouseReleased(int x, int y, Input::MouseButton button) override;
        void OnMousePressed(int x, int y, Input::MouseButton button) override;
        bool OnMouseDrag(int x, int y, Input::MouseButton button) override;
        bool OnMouseWheel(int x, int y, Input::MouseWheel direction) override;
        bool OnMouseOver(int x, int y) override;
        bool OnMouseLeave() override;
        void OnFocus() override;
        void OnUpdateScrollBars() override;

        // coloane
        bool AddColumn(const ConstString& text, Graphics::TextAlignament Align, unsigned int Size = 10);
        bool SetColumnText(unsigned int columnIndex, const ConstString& text);
        bool SetColumnAlignament(unsigned int columnIndex, Graphics::TextAlignament Align);
        bool SetColumnWidth(unsigned int columnIndex, unsigned int width);
        bool SetColumnClipboardCopyState(unsigned int columnIndex, bool allowCopy);
        bool SetColumnFilterMode(unsigned int columnIndex, bool allowFilterForThisColumn);
        bool DeleteColumn(unsigned int columnIndex);
        void DeleteAllColumns();
        unsigned int GetColumnsCount();

        // items add
        ItemHandle AddItem(const ConstString& text);
        ItemHandle AddItem(const ConstString& text, const ConstString& subItem1);
        ItemHandle AddItem(const ConstString& text, const ConstString& subItem1, const ConstString& subItem2);
        ItemHandle AddItem(
              const ConstString& text,
              const ConstString& subItem1,
              const ConstString& subItem2,
              const ConstString& subItem3);
        ItemHandle AddItem(
              const ConstString& text,
              const ConstString& subItem1,
              const ConstString& subItem2,
              const ConstString& subItem3,
              const ConstString& subItem4);
        ItemHandle AddItem(
              const ConstString& text,
              const ConstString& subItem1,
              const ConstString& subItem2,
              const ConstString& subItem3,
              const ConstString& subItem4,
              const ConstString& subItem5);
        ItemHandle AddItem(
              const ConstString& text,
              const ConstString& subItem1,
              const ConstString& subItem2,
              const ConstString& subItem3,
              const ConstString& subItem4,
              const ConstString& subItem5,
              const ConstString& subItem6);
        ItemHandle AddItem(
              const ConstString& text,
              const ConstString& subItem1,
              const ConstString& subItem2,
              const ConstString& subItem3,
              const ConstString& subItem4,
              const ConstString& subItem5,
              const ConstString& subItem6,
              const ConstString& subItem7);
        ItemHandle AddItem(
              const ConstString& text,
              const ConstString& subItem1,
              const ConstString& subItem2,
              const ConstString& subItem3,
              const ConstString& subItem4,
              const ConstString& subItem5,
              const ConstString& subItem6,
              const ConstString& subItem7,
              const ConstString& subItem8);

        // items properties
        bool SetItemText(ItemHandle item, unsigned int subItemIndex, const ConstString& text);
        const Graphics::CharacterBuffer& GetItemText(ItemHandle item, unsigned int subItemIndex);
        bool SetItemCheck(ItemHandle item, bool check);
        bool SetItemSelect(ItemHandle item, bool select);
        bool SetItemColor(ItemHandle item, Graphics::ColorPair color);
        bool SetItemType(ItemHandle item, ListViewItemType type);
        bool IsItemChecked(ItemHandle item);
        bool IsItemSelected(ItemHandle item);

        bool SetItemData(ItemHandle item, uint64 value);
        uint64 GetItemData(ItemHandle item, uint64 errorValue);

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
        bool AddItem(const ConstString& caption, GenericRef userData);

      protected:
        ComboBox(string_view layout, const ConstString& text, char itemsSeparator);

      public:
        static const unsigned int NO_ITEM_SELECTED = 0xFFFFFFFF;

        inline uint64 GetCurrentItemUserData(uint64 errorValue) const
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
        const Graphics::CharacterBuffer& GetCurrentItemText();

        uint64 GetItemUserData(unsigned int index, uint64 errorValue) const;
        template <typename T>
        inline Reference<T> GetItemUserData(unsigned int index) const
        {
            return GetItemDataAsPointer(index).ToReference<T>();
        }

        const Graphics::CharacterBuffer& GetItemText(unsigned int index);

        bool SetItemUserData(unsigned int index, uint64 userData);
        template <typename T>
        inline bool SetItemUserData(unsigned int index, Reference<T> userData)
        {
            return SetItemDataAsPointer(index, userData.ToGenericRef());
        }
        bool SetCurentItemIndex(unsigned int index);
        void SetNoIndexSelected();

        template <typename T>
        inline bool AddItem(const ConstString& caption, Reference<T> obj)
        {
            return AddItem(caption, obj.ToGenericRef());
        }
        bool AddItem(const ConstString& caption, uint64 usedData);
        inline bool AddItem(const ConstString& caption)
        {
            return AddItem(caption, GenericRef(nullptr));
        }

        bool AddSeparator(const ConstString& caption = "");
        void DeleteAllItems();

        bool OnKeyEvent(Input::Key keyCode, char16 UnicodeChar) override;
        void OnHotKey() override;
        bool OnMouseLeave() override;
        bool OnMouseEnter() override;
        bool OnMouseOver(int x, int y) override;
        void OnMousePressed(int x, int y, Input::MouseButton button) override;
        bool OnMouseWheel(int x, int y, Input::MouseWheel direction) override;
        void Paint(Graphics::Renderer& renderer) override;
        void OnExpandView(Graphics::Clip& expandedClip) override;
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

        ItemHandle AddCommandItem(const ConstString& text, int CommandID, Input::Key shortcutKey = Input::Key::None);
        ItemHandle AddCheckItem(
              const ConstString& text, int CommandID, bool checked = false, Input::Key shortcutKey = Input::Key::None);
        ItemHandle AddRadioItem(
              const ConstString& text, int CommandID, bool checked = false, Input::Key shortcutKey = Input::Key::None);
        ItemHandle AddSeparator();
        ItemHandle AddSubMenu(const ConstString& text);

        Reference<Menu> GetSubMenu(ItemHandle menuItem);

        bool SetEnable(ItemHandle menuItem, bool status);
        bool SetChecked(ItemHandle menuItem, bool status);

        void Show(int x, int y, const Graphics::Size& maxSize = { 0, 0 });
        void Show(Reference<Control> parent, int relativeX, int relativeY, const Graphics::Size& maxSize = { 0, 0 });
    };

    class EXPORT NumericSelector : public Control
    {
      protected:
        NumericSelector(const int64 minValue, const int64 maxValue, int64 value, string_view layout);

      public:
        int64 GetValue() const;
        void SetValue(const int64 value);
        void SetMinValue(const int64 minValue);
        void SetMaxValue(const int64 maxValue);

      private:
        bool IsValidValue(const int64 value) const;
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
        bool OnKeyEvent(Input::Key keyCode, char16 UnicodeChar) override;
        void OnMousePressed(int x, int y, Input::MouseButton button) override;
        void OnMouseReleased(int x, int y, Input::MouseButton button) override;
        bool OnMouseWheel(int x, int y, Input::MouseWheel direction) override;
        bool OnMouseEnter() override;
        bool OnMouseLeave() override;
        bool OnMouseDrag(int x, int y, Input::MouseButton button) override;
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
        void Paint(Graphics::Renderer& renderer) override;
        bool OnKeyEvent(Input::Key keyCode, char16 UnicodeChar) override;

        friend Factory::Desktop;
        friend Control;
    };

    class EXPORT SingleApp : public Desktop
    {
    };

    enum class TreeFlags : uint32
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

    class EXPORT Tree : public Control
    {
      private:
        GenericRef GetItemDataAsPointer(const ItemHandle item) const;
        bool SetItemDataAsPointer(ItemHandle item, GenericRef obj);

      protected:
        Tree(string_view layout, const TreeFlags flags = TreeFlags::None, const unsigned int noOfColumns = 1);

      public:
        void Paint(Graphics::Renderer& renderer) override;
        bool OnKeyEvent(Input::Key keyCode, char16 UnicodeChar) override;
        void OnFocus() override;
        void OnMousePressed(int x, int y, Input::MouseButton button) override;
        bool OnMouseOver(int x, int y) override;
        bool OnMouseWheel(int x, int y, Input::MouseWheel direction) override;
        void OnUpdateScrollBars() override;
        void OnAfterResize(int newWidth, int newHeight) override;

        // handlers covariant
        Handlers::Tree* Handlers() override;

        ItemHandle AddItem(
              const ItemHandle parent,
              const vector<Graphics::CharacterBuffer>& values,
              const ConstString metadata,
              bool process      = false,
              bool isExpandable = false);
        bool RemoveItem(const ItemHandle handle, bool process = false);
        bool ClearItems();
        ItemHandle GetCurrentItem();
        const ConstString GetItemText(const ItemHandle handle);

        bool SetItemData(ItemHandle item, uint64 value);
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
        uint64 GetItemData(const size_t index, uint64 errorValue);
        ItemHandle GetItemHandleByIndex(const unsigned int index) const;

        unsigned int GetItemsCount() const;
        bool AddColumnData(
              const unsigned int index,
              const ConstString title,
              const Graphics::TextAlignament headerAlignment,
              const Graphics::TextAlignament contentAlignment,
              const unsigned int width = 0xFFFFFFFF);
        const Utils::UnicodeStringBuilder& GetItemMetadata(ItemHandle handle);
        bool SetItemMetadata(ItemHandle handle, const ConstString& metadata);

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
        bool SetColorForItems(const Graphics::ColorPair& color);
        bool SearchItems();
        bool ProcessOrderedItems(const ItemHandle handle, const bool clear = true);
        bool MarkAllItemsAsNotFound();
        bool MarkAllAncestorsWithChildFoundInFilterSearch(const ItemHandle handle);

        friend Factory::Tree;
        friend Control;
    };

    enum class GridFlags : uint32
    {
        None                  = 0x000000,
        HideHorizontalLines   = 0x000100,
        HideVerticalLines     = 0x000200,
        HideBoxes             = 0x000400,
        HideHoveredCell       = 0x000800,
        HideSelectedCell      = 0x001000,
        TransparentBackground = 0x002000,
        HideHeader            = 0x004000
    };

    class EXPORT Grid : public Control
    {
      public:
        enum class CellType
        {
            Boolean = 0,
            String  = 1
        };

      protected:
        Grid(string_view layout, unsigned int columnsNo, unsigned int rowsNo, GridFlags flags);

      public:
        void Paint(Graphics::Renderer& renderer) override;
        bool OnKeyEvent(Input::Key keyCode, char16 UnicodeChar) override;
        void OnMousePressed(int x, int y, Input::MouseButton button) override;
        void OnMouseReleased(int x, int y, Input::MouseButton button) override;
        bool OnMouseDrag(int x, int y, Input::MouseButton button) override;
        bool OnMouseOver(int x, int y) override;
        bool OnMouseLeave() override;
        void OnLoseFocus() override;
        bool OnEvent(Controls::Reference<Control>, Event eventType, int controlID) override;

        unsigned int GetCellsCount() const;
        Graphics::Size GetGridDimensions() const;
        bool UpdateCell(
              unsigned int index,
              CellType cellType,
              const variant<bool, ConstString>& content,
              Graphics::TextAlignament textAlignment = Graphics::TextAlignament::Left);
        bool UpdateCell(
              unsigned int x,
              unsigned int y,
              CellType cellType,
              const variant<bool, ConstString>& content,
              Graphics::TextAlignament textAlignment = Graphics::TextAlignament::Left);
        const ConstString GetSeparator() const;
        void SetSeparator(ConstString separator);
        bool UpdateHeaderValues(
              const vector<ConstString>& headerValues,
              Graphics::TextAlignament textAlignment = Graphics::TextAlignament::Left);

      private:
        void DrawBoxes(Graphics::Renderer& renderer);
        void DrawLines(Graphics::Renderer& renderer);
        unsigned int ComputeCellNumber(int x, int y);
        Graphics::SpecialChars ComputeBoxType(
              unsigned int colIndex,
              unsigned int rowIndex,
              unsigned int startColumnsIndex,
              unsigned int startRowsIndex,
              unsigned int endColumnsIndex,
              unsigned int endRowsIndex);
        void DrawCellsBackground(Graphics::Renderer& renderer);
        bool DrawCellContent(Graphics::Renderer& renderer, unsigned int cellIndex);
        bool DrawHeader(Graphics::Renderer& renderer);
        void UpdateGridParameters();
        bool MoveSelectedCellByKeys(Input::Key keyCode);
        bool SelectCellsByKeys(Input::Key keyCode);
        bool ToggleBooleanCell();
        bool CopySelectedCellsContent() const;
        bool PasteContentToSelectedCells();

      private:
        friend Factory::Grid;
        friend Control;
    };

    namespace Factory
    {
        class EXPORT Label
        {
            Label() = delete;

          public:
            static Reference<Controls::Label> Create(
                  Controls::Control* parent, const ConstString& caption, string_view layout);
            static Reference<Controls::Label> Create(
                  Controls::Control& parent, const ConstString& caption, string_view layout);
            static Pointer<Controls::Label> Create(const ConstString& caption, string_view layout);
        };
        class EXPORT Button
        {
            Button() = delete;

          public:
            static Reference<Controls::Button> Create(
                  Controls::Control* parent,
                  const ConstString& caption,
                  string_view layout,
                  int controlID     = 0,
                  ButtonFlags flags = ButtonFlags::None);
            static Reference<Controls::Button> Create(
                  Controls::Control& parent,
                  const ConstString& caption,
                  string_view layout,
                  int controlID     = 0,
                  ButtonFlags flags = ButtonFlags::None);
            static Pointer<Controls::Button> Create(
                  const ConstString& caption,
                  string_view layout,
                  int controlID               = 0,
                  Controls::ButtonFlags flags = Controls::ButtonFlags::None);
        };
        class EXPORT Password
        {
            Password() = delete;

          public:
            static Reference<Controls::Password> Create(
                  Controls::Control* parent, const ConstString& caption, string_view layout);
            static Reference<Controls::Password> Create(
                  Controls::Control& parent, const ConstString& caption, string_view layout);
            static Pointer<Controls::Password> Create(const ConstString& caption, string_view layout);
        };
        class EXPORT CheckBox
        {
            CheckBox() = delete;

          public:
            static Reference<Controls::CheckBox> Create(
                  Controls::Control* parent, const ConstString& caption, string_view layout, int controlID = 0);
            static Reference<Controls::CheckBox> Create(
                  Controls::Control& parent, const ConstString& caption, string_view layout, int controlID = 0);
            static Pointer<Controls::CheckBox> Create(
                  const ConstString& caption, string_view layout, int controlID = 0);
        };
        class EXPORT RadioBox
        {
            RadioBox() = delete;

          public:
            static Pointer<Controls::RadioBox> Create(
                  const ConstString& caption, string_view layout, int groupID, int controlID = 0);
            static Reference<Controls::RadioBox> Create(
                  Controls::Control* parent,
                  const ConstString& caption,
                  string_view layout,
                  int groupID,
                  int controlID = 0);
            static Reference<Controls::RadioBox> Create(
                  Controls::Control& parent,
                  const ConstString& caption,
                  string_view layout,
                  int groupID,
                  int controlID = 0);
        };
        class EXPORT Splitter
        {
            Splitter() = delete;

          public:
            static Reference<Controls::Splitter> Create(Controls::Control* parent, string_view layout, bool vertical);
            static Reference<Controls::Splitter> Create(Controls::Control& parent, string_view layout, bool vertical);
            static Pointer<Controls::Splitter> Create(string_view layout, bool vertical);
        };
        class EXPORT Panel
        {
            Panel() = delete;

          public:
            static Reference<Controls::Panel> Create(
                  Controls::Control* parent, const ConstString& caption, string_view layout);
            static Reference<Controls::Panel> Create(
                  Controls::Control& parent, const ConstString& caption, string_view layout);
            static Pointer<Controls::Panel> Create(const ConstString& caption, string_view layout);
            static Reference<Controls::Panel> Create(Controls::Control* parent, string_view layout);
            static Reference<Controls::Panel> Create(Controls::Control& parent, string_view layout);
            static Pointer<Controls::Panel> Create(string_view layout);
        };
        class EXPORT TextField
        {
            TextField() = delete;

          public:
            static Reference<Controls::TextField> Create(
                  Controls::Control* parent,
                  const ConstString& caption,
                  string_view layout,
                  Controls::TextFieldFlags flags = Controls::TextFieldFlags::None);
            static Reference<Controls::TextField> Create(
                  Controls::Control& parent,
                  const ConstString& caption,
                  string_view layout,
                  Controls::TextFieldFlags flags = Controls::TextFieldFlags::None);
            static Pointer<Controls::TextField> Create(
                  const ConstString& caption,
                  string_view layout,
                  Controls::TextFieldFlags flags = Controls::TextFieldFlags::None);
        };
        class EXPORT TextArea
        {
            TextArea() = delete;

          public:
            static Reference<Controls::TextArea> Create(
                  Controls::Control* parent,
                  const ConstString& caption,
                  string_view layout,
                  Controls::TextAreaFlags flags = Controls::TextAreaFlags::None);
            static Reference<Controls::TextArea> Create(
                  Controls::Control& parent,
                  const ConstString& caption,
                  string_view layout,
                  Controls::TextAreaFlags flags = Controls::TextAreaFlags::None);
            static Pointer<Controls::TextArea> Create(
                  const ConstString& caption,
                  string_view layout,
                  Controls::TextAreaFlags flags = Controls::TextAreaFlags::None);
        };
        class EXPORT TabPage
        {
            TabPage() = delete;

          public:
            static Reference<Controls::TabPage> Create(Controls::Control* parent, const ConstString& caption);
            static Reference<Controls::TabPage> Create(Controls::Control& parent, const ConstString& caption);
            static Pointer<Controls::TabPage> Create(const ConstString& caption);
        };
        class EXPORT Tab
        {
            Tab() = delete;

          public:
            static Reference<Controls::Tab> Create(
                  Controls::Control* parent,
                  string_view layout,
                  Controls::TabFlags flags = Controls::TabFlags::TopTabs,
                  unsigned int tabPageSize = 16);
            static Reference<Controls::Tab> Create(
                  Controls::Control& parent,
                  string_view layout,
                  Controls::TabFlags flags = Controls::TabFlags::TopTabs,
                  unsigned int tabPageSize = 16);
            static Pointer<Controls::Tab> Create(
                  string_view layout,
                  Controls::TabFlags flags = Controls::TabFlags::TopTabs,
                  unsigned int tabPageSize = 16);
        };
        class EXPORT CanvasViewer
        {
            CanvasViewer() = delete;

          public:
            static Reference<Controls::CanvasViewer> Create(
                  Controls::Control* parent,
                  string_view layout,
                  unsigned int canvasWidth,
                  unsigned int canvasHeight,
                  Controls::ViewerFlags flags = Controls::ViewerFlags::None);
            static Reference<Controls::CanvasViewer> Create(
                  Controls::Control& parent,
                  string_view layout,
                  unsigned int canvasWidth,
                  unsigned int canvasHeight,
                  Controls::ViewerFlags flags = Controls::ViewerFlags::None);
            static Pointer<Controls::CanvasViewer> Create(
                  string_view layout,
                  unsigned int canvasWidth,
                  unsigned int canvasHeight,
                  Controls::ViewerFlags flags = ViewerFlags::None);
            static Reference<Controls::CanvasViewer> Create(
                  Controls::Control* parent,
                  const ConstString& caption,
                  string_view layout,
                  unsigned int canvasWidth,
                  unsigned int canvasHeight,
                  Controls::ViewerFlags flags = ViewerFlags::None);
            static Reference<Controls::CanvasViewer> Create(
                  Controls::Control& parent,
                  const ConstString& caption,
                  string_view layout,
                  unsigned int canvasWidth,
                  unsigned int canvasHeight,
                  Controls::ViewerFlags flags = ViewerFlags::None);
            static Pointer<Controls::CanvasViewer> Create(
                  const ConstString& caption,
                  string_view layout,
                  unsigned int canvasWidth,
                  unsigned int canvasHeight,
                  Controls::ViewerFlags flags = ViewerFlags::None);
        };
        class EXPORT ImageViewer
        {
            ImageViewer() = delete;

          public:
            static Pointer<Controls::ImageViewer> Create(
                  string_view layout, Controls::ViewerFlags flags = Controls::ViewerFlags::None);
            static Reference<Controls::ImageViewer> Create(
                  Controls::Control* parent,
                  string_view layout,
                  Controls::ViewerFlags flags = Controls::ViewerFlags::None);
            static Reference<Controls::ImageViewer> Create(
                  Controls::Control& parent,
                  string_view layout,
                  Controls::ViewerFlags flags = Controls::ViewerFlags::None);
            static Pointer<Controls::ImageViewer> Create(
                  const ConstString& caption,
                  string_view layout,
                  Controls::ViewerFlags flags = Controls::ViewerFlags::None);
            static Reference<Controls::ImageViewer> Create(
                  Controls::Control* parent,
                  const ConstString& caption,
                  string_view layout,
                  Controls::ViewerFlags flags = Controls::ViewerFlags::None);
            static Reference<Controls::ImageViewer> Create(
                  Controls::Control& parent,
                  const ConstString& caption,
                  string_view layout,
                  Controls::ViewerFlags flags = Controls::ViewerFlags::None);
        };
        class EXPORT ListView
        {
            ListView() = delete;

          public:
            static Pointer<Controls::ListView> Create(
                  string_view layout, Controls::ListViewFlags flags = Controls::ListViewFlags::None);
            static Reference<Controls::ListView> Create(
                  Controls::Control* parent,
                  string_view layout,
                  Controls::ListViewFlags flags = Controls::ListViewFlags::None);
            static Reference<Controls::ListView> Create(
                  Controls::Control& parent,
                  string_view layout,
                  Controls::ListViewFlags flags = Controls::ListViewFlags::None);
        };
        class EXPORT ComboBox
        {
            ComboBox() = delete;

          public:
            static Pointer<Controls::ComboBox> Create(
                  string_view layout, const ConstString& text = string_view(), char itemsSeparator = ',');

            static Reference<Controls::ComboBox> Create(
                  Controls::Control* parent,
                  string_view layout,
                  const ConstString& text = string_view(),
                  char itemsSeparator     = ',');
            static Reference<Controls::ComboBox> Create(
                  Controls::Control& parent,
                  string_view layout,
                  const ConstString& text = string_view(),
                  char itemsSeparator     = ',');
        };
        class EXPORT NumericSelector
        {
            NumericSelector() = delete;

          public:
            static Pointer<Controls::NumericSelector> Create(
                  const int64 minValue, const int64 maxValue, int64 value, string_view layout);
            static Reference<Controls::NumericSelector> Create(
                  Controls::Control* parent,
                  const int64 minValue,
                  const int64 maxValue,
                  int64 value,
                  string_view layout);
            static Reference<Controls::NumericSelector> Create(
                  Controls::Control& parent,
                  const int64 minValue,
                  const int64 maxValue,
                  int64 value,
                  string_view layout);
        };
        class EXPORT Window
        {
            Window() = delete;

          public:
            static Pointer<Controls::Window> Create(
                  const ConstString& caption,
                  string_view layout,
                  Controls::WindowFlags windowFlags = Controls::WindowFlags::None);
        };
        class EXPORT Desktop
        {
            Desktop() = delete;

          public:
            static Pointer<Controls::Desktop> Create();
        };

        class EXPORT Tree
        {
            Tree() = delete;

          public:
            static Pointer<Controls::Tree> Create(
                  string_view layout,
                  const Controls::TreeFlags flags = Controls::TreeFlags::None,
                  const unsigned int noOfColumns  = 1);
            static Reference<Controls::Tree> Create(
                  Control* parent,
                  string_view layout,
                  const Controls::TreeFlags flags = Controls::TreeFlags::None,
                  const unsigned int noOfColumns  = 1);
            static Reference<Controls::Tree> Create(
                  Control& parent,
                  string_view layout,
                  const Controls::TreeFlags flags = Controls::TreeFlags::None,
                  const unsigned int noOfColumns  = 1);
        };

        class EXPORT Grid
        {
            Grid() = delete;

          public:
            static Pointer<Controls::Grid> Create(
                  string_view layout, unsigned int columnsNo, unsigned int rowsNo, Controls::GridFlags flags);
            static Reference<Controls::Grid> Create(
                  Controls::Control* parent,
                  string_view layout,
                  unsigned int columnsNo,
                  unsigned int rowsNo,
                  Controls::GridFlags flags);
            static Reference<Controls::Grid> Create(
                  Controls::Control& parent,
                  string_view layout,
                  unsigned int columnsNo,
                  unsigned int rowsNo,
                  Controls::GridFlags flags);
        };
    } // namespace Factory

}; // namespace Controls

namespace Dialogs
{
    class EXPORT MessageBox
    {
        MessageBox() = delete;

      public:
        static void ShowError(const ConstString& title, const ConstString& message);
        static void ShowNotification(const ConstString& title, const ConstString& message);
        static void ShowWarning(const ConstString& title, const ConstString& message);
        static Result ShowOkCancel(const ConstString& title, const ConstString& message);
        static Result ShowYesNoCancel(const ConstString& title, const ConstString& message);
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
        static optional<std::filesystem::path> ShowSaveFileWindow(
              const ConstString& fileName, const ConstString& extensionsFilter, const std::filesystem::path& path);
        static optional<std::filesystem::path> ShowOpenFileWindow(
              const ConstString& fileName, const ConstString& extensionsFilter, const std::filesystem::path& path);
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
    enum class Severity : uint32
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
    enum class InitializationFlags : uint32
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

    enum class CharacterSize : uint32
    {
        Default = 0,
        Tiny,
        Small,
        Normal,
        Large,
        Huge
    };
    enum class FrontendType : uint32
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
        string_view FontName;
        Controls::Desktop* (*CustomDesktopConstructor)();

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
        bool SetCommand(Input::Key keyCode, const ConstString& caption, int CommandID);
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
                Graphics::ColorPair Regular, Highligheted, Inactive, Error, Warning, Emphasized1, Emphasized2, Category;
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
        struct
        {
            struct
            {
                struct
                {
                    Graphics::ColorPair Normal, Selected, Hovered;
                } Horizontal;
                struct
                {
                    Graphics::ColorPair Normal, Selected, Hovered;
                } Vertical;
                struct
                {
                    Graphics::ColorPair Normal, Selected, Hovered;
                } Box;
            } Lines;
            struct
            {
                Graphics::ColorPair Grid;
                struct
                {
                    Graphics::ColorPair Normal, Selected, Hovered;
                } Cell;
            } Background;
            struct
            {
                Graphics::ColorPair Normal;
            } Text;
            Graphics::ColorPair Header;
        } Grid;
        void SetDarkTheme();
    };

    EXPORT Config* GetAppConfig();
    EXPORT Utils::IniObject* GetAppSettings();
    EXPORT bool SaveAppSettings();
    EXPORT void UpdateAppCUISettings(Utils::IniObject& ini, bool clearExistingSettings = false);
    EXPORT bool UpdateAppCUISettings(bool clearExistingSettings = false);
    EXPORT std::filesystem::path GetAppSettingsFile();

    NODISCARD("Check the return of the Init function. If false, AppCUI has not been initialized properly")
    EXPORT bool Init(Application::InitializationFlags flags = Application::InitializationFlags::None);

    NODISCARD("Check the return of the Init function. If false, AppCUI has not been initialized properly")
    EXPORT bool Init(InitializationData& initData);

    EXPORT bool Run();
    EXPORT bool RunSingleApp(unique_ptr<Controls::SingleApp> singleApp);
    EXPORT Controls::ItemHandle AddWindow(
          unique_ptr<Controls::Window> wnd, Controls::ItemHandle referal = Controls::InvalidItemHandle);
    EXPORT Controls::ItemHandle AddWindow(unique_ptr<Controls::Window> wnd, Controls::Window* referalWindow);
    EXPORT Controls::Menu* AddMenu(const ConstString& name);
    EXPORT bool GetApplicationSize(Graphics::Size& size);
    EXPORT bool GetDesktopSize(Graphics::Size& size);
    EXPORT void Repaint();
    EXPORT void RecomputeControlsLayout();
    EXPORT void ArrangeWindows(ArangeWindowsMethod method);
    EXPORT void RaiseEvent(
          Utils::Reference<Controls::Control> control,
          Utils::Reference<Controls::Control> sourceControl,
          Controls::Event eventType,
          int controlID);
    EXPORT void Close();
}; // namespace Application

} // namespace AppCUI

// inline operations for enum classes
ADD_FLAG_OPERATORS(AppCUI::Application::InitializationFlags, AppCUI::uint32)
ADD_FLAG_OPERATORS(AppCUI::Input::Key, AppCUI::uint32);
ADD_FLAG_OPERATORS(AppCUI::Input::MouseButton, AppCUI::uint32);
ADD_FLAG_OPERATORS(AppCUI::Graphics::WriteTextFlags, AppCUI::uint32)
ADD_FLAG_OPERATORS(AppCUI::Graphics::TextAlignament, AppCUI::uint32);
ADD_FLAG_OPERATORS(AppCUI::Controls::TextAreaFlags, AppCUI::uint32);
ADD_FLAG_OPERATORS(AppCUI::Controls::ListViewFlags, AppCUI::uint32);
ADD_FLAG_OPERATORS(AppCUI::Controls::TabFlags, AppCUI::uint32)
ADD_FLAG_OPERATORS(AppCUI::Controls::WindowFlags, AppCUI::uint32)
ADD_FLAG_OPERATORS(AppCUI::Controls::ButtonFlags, AppCUI::uint32)
ADD_FLAG_OPERATORS(AppCUI::Controls::TextFieldFlags, AppCUI::uint32)
ADD_FLAG_OPERATORS(AppCUI::Utils::NumberParseFlags, AppCUI::uint32)
ADD_FLAG_OPERATORS(AppCUI::Utils::NumericFormatFlags, AppCUI::uint16)
ADD_FLAG_OPERATORS(AppCUI::Controls::TreeFlags, AppCUI::uint32)
ADD_FLAG_OPERATORS(AppCUI::Controls::GridFlags, AppCUI::uint32)

#undef ADD_FLAG_OPERATORS