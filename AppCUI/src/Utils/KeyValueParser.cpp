#include "Internal.hpp"

namespace AppCUI::Utils
{
/*
General format can be one of:
1. <word> <equal> <value> <separator> ...
2. <word> <separator>
Where:
- <equal> can be '=' or ':'
- <separator> can be ',' or ';'
- <word> and <value> = any sequance of chars different than space, tab
*/
static constexpr uint8 CHAR_TYPE_OTHER     = 0;
static constexpr uint8 CHAR_TYPE_SPACE     = 1;
static constexpr uint8 CHAR_TYPE_SEPARATOR = 2;
static constexpr uint8 CHAR_TYPE_EQ        = 3;
static constexpr uint32 MAX_CHARS_IN_TABLE = 62;

static uint8 ParserCharacterTypes[MAX_CHARS_IN_TABLE] = {
    CHAR_TYPE_OTHER, CHAR_TYPE_OTHER, CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER, CHAR_TYPE_OTHER, CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER, CHAR_TYPE_OTHER, CHAR_TYPE_OTHER,     CHAR_TYPE_SPACE, CHAR_TYPE_SPACE, CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER, CHAR_TYPE_SPACE, CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER, CHAR_TYPE_OTHER, CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER, CHAR_TYPE_OTHER, CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER, CHAR_TYPE_OTHER, CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER, CHAR_TYPE_OTHER, CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER, CHAR_TYPE_OTHER, CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER, CHAR_TYPE_OTHER, CHAR_TYPE_SPACE,     CHAR_TYPE_OTHER, CHAR_TYPE_OTHER, CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER, CHAR_TYPE_OTHER, CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER, CHAR_TYPE_OTHER, CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER, CHAR_TYPE_OTHER, CHAR_TYPE_SEPARATOR, CHAR_TYPE_OTHER, CHAR_TYPE_OTHER, CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER, CHAR_TYPE_OTHER, CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER, CHAR_TYPE_OTHER, CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER, CHAR_TYPE_OTHER, CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER, CHAR_TYPE_EQ,    CHAR_TYPE_SEPARATOR,
    CHAR_TYPE_OTHER, CHAR_TYPE_EQ
};
const uint8 Parser_LoweCaseTable[256] = {
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,
    22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,
    44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  97,
    98,  99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
    120, 121, 122, 91,  92,  93,  94,  95,  96,  97,  98,  99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
    110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131,
    132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153,
    154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
    176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197,
    198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219,
    220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241,
    242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};
#define CHECK_PARSE_ERROR(c, msg)                                                                                      \
    if (!(c))                                                                                                          \
    {                                                                                                                  \
        error    = msg;                                                                                                \
        errorPos = current;                                                                                            \
        RETURNERROR(false, msg);                                                                                       \
    }

template <typename T>
class Parser
{
    const T* start;
    const T* end;
    const T* current;
    std::string_view error;
    const T* errorPos;

    inline void SkipSpaces()
    {
        while ((current < end) && ((*current) < MAX_CHARS_IN_TABLE) &&
               ((ParserCharacterTypes[*current]) == CHAR_TYPE_SPACE))
            current++;
    }
    inline const T* ParseWord()
    {
        while ((current < end) &&
               ((((*current) < MAX_CHARS_IN_TABLE) && ((ParserCharacterTypes[*current]) == CHAR_TYPE_OTHER)) ||
                ((*current) >= MAX_CHARS_IN_TABLE)))
            current++;
        return current;
    }
    inline bool IsWord() const
    {
        return (
              (current < end) &&
              ((((*current) < MAX_CHARS_IN_TABLE) && ((ParserCharacterTypes[*current]) == CHAR_TYPE_OTHER)) ||
               ((*current) >= MAX_CHARS_IN_TABLE)));
    }
    inline bool IsEq() const
    {
        return (
              (current < end) && ((*current) < MAX_CHARS_IN_TABLE) &&
              ((ParserCharacterTypes[*current]) == CHAR_TYPE_EQ));
    }
    inline bool IsSeparator() const
    {
        return (
              (current < end) && ((*current) < MAX_CHARS_IN_TABLE) &&
              ((ParserCharacterTypes[*current]) == CHAR_TYPE_SEPARATOR));
    }
    inline bool AnalizeValue(const T* s, const T* e, int& value, KeyValuePair::Type& type)
    {
        bool negative     = false;
        int32 firstPart   = 0;
        int32 secondPart  = 0;
        bool isPercentage = false;
        type              = KeyValuePair::Type::String;
        if ((*s) == '-')
        {
            negative = true;
            s++;
        }
        while ((s < e) && (((*s) >= '0') && ((*s) <= '9')))
        {
            firstPart = firstPart * 10 + (int32) ((*s) - '0');
            s++;
        }
        if ((s < e) && ((*s) == '.'))
        {
            auto cnt = 0U;
            while ((s < e) && (((*s) >= '0') && ((*s) <= '9')))
            {
                if (cnt < 2) // only the first two decimals are stored
                    secondPart = secondPart * 10 + (int32) ((*s) - '0');
                s++;
                cnt++;
            }
        }
        if ((s < e) && ((*s) == '%'))
        {
            isPercentage = true;
            s++;
        }
        if (s < e)
            return false; // not a valid number
        // valid number
        if (isPercentage)
        {
            value = firstPart * 100 + (secondPart % 100);
            type  = KeyValuePair::Type::Percentage;
        }
        else
        {
            value = firstPart;
            type  = KeyValuePair::Type::Number;
        }
        if (negative)
            value = -value;
        return true;
    }
    inline uint64 ComputeHash(const T* s, const T* e)
    {
        // use FNV algorithm ==> https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
        uint64 hash = 0xcbf29ce484222325ULL;
        while (s < e)
        {
            hash = hash ^ (Parser_LoweCaseTable[(*s) & 0xFF]);
            hash = hash * 0x00000100000001B3ULL;
            s++;
        }
        return hash;
    }

  public:
    Parser(const T* _start, size_t size)
    {
        start = current = _start;
        end             = start + size;
        errorPos        = nullptr;
    }
    bool Next(KeyValuePair& pair)
    {
        if (errorPos)
            return false;
        SkipSpaces();
        if (current >= end)
            return false; // end the process
        CHECK_PARSE_ERROR(IsWord(), "Expecting a valid key (word)");
        auto keyStart       = current;
        auto keyEnd         = ParseWord();
        const T* valueStart = nullptr;
        const T* valueEnd   = nullptr;
        SkipSpaces();
        if (IsEq())
        {
            current++;
            SkipSpaces();
            CHECK_PARSE_ERROR(IsWord(), "Expecting a valid value (word)");
            valueStart = current;
            valueEnd   = ParseWord();
            SkipSpaces();
        }
        if (IsSeparator())
            current++;
        // we have a key/value pair --> store it
        pair.Key.data     = keyStart;
        pair.Key.dataSize = (uint32) (keyEnd - keyStart);
        pair.Key.number   = 0;
        pair.Key.hash     = 0;
        pair.Value.hash   = 0;
        if (valueStart)
        {
            // we have key = value => key = string
            pair.Key.hash       = ComputeHash(keyStart, keyEnd);
            pair.Key.type       = KeyValuePair::Type::String;
            pair.Value.data     = valueStart;
            pair.Value.dataSize = (uint32) (valueEnd - valueStart);

            if (!AnalizeValue(valueStart, valueEnd, pair.Value.number, pair.Value.type))
            {
                // we have a hash:
                pair.Value.hash = ComputeHash(valueStart, valueEnd);
                pair.Value.type = KeyValuePair::Type::String;
            }
        }
        else
        {
            // we have just key
            if (!AnalizeValue(keyStart, keyEnd, pair.Key.number, pair.Key.type))
            {
                // we have a hash:
                pair.Key.hash = ComputeHash(valueStart, valueEnd);
                pair.Key.type = KeyValuePair::Type::String;
            }
            pair.Value.type     = KeyValuePair::Type::None;
            pair.Value.data     = nullptr;
            pair.Value.dataSize = 0;
        }
        return true;
    }
    inline const T* GetErrorPos() const
    {
        return errorPos;
    }
    inline std::string_view GetErrorName() const
    {
        return error;
    }
};
bool KeyValueParser::Parse(std::string_view text)
{
    Parser<uint8> p((const uint8*) text.data(), text.size());
    KeyValuePair* k = this->items;
    KeyValuePair* e = k + KeyValueParser::MAX_ITEMS;
    this->count     = 0;
    this->errorPos  = KeyValueParser::NO_ERRORS;
    this->errorName = "";

    while ((k < e) && (p.Next(*k)))
        k++;
    if (k == e)
    {
        this->errorPos  = 0;
        this->errorName = "Too many key/value pairs. Max allowed are 32 !";
        return false;
    }
    if (p.GetErrorPos())
    {
        // we have an error
        this->errorPos  = (uint32) (p.GetErrorPos() - ((const uint8*) text.data()));
        this->errorName = p.GetErrorName();
        return false;
    }
    this->count = (uint32) (k - this->items);
    return true;
}
bool KeyValueParser::Parse(std::u16string_view text)
{
    Parser<uint16> p((const uint16*) text.data(), text.size());
    KeyValuePair* k = this->items;
    KeyValuePair* e = k + KeyValueParser::MAX_ITEMS;
    this->count     = 0;
    this->errorPos  = KeyValueParser::NO_ERRORS;
    this->errorName = "";

    while ((k < e) && (p.Next(*k)))
        k++;
    if (k == e)
    {
        this->errorPos  = 0;
        this->errorName = "Too many key/value pairs. Max allowed are 32 !";
        return false;
    }
    if (p.GetErrorPos())
    {
        // we have an error
        this->errorPos  = (uint32) (p.GetErrorPos() - ((const uint16*) text.data()));
        this->errorName = p.GetErrorName();
        return false;
    }
    this->count = (uint32) (k - this->items);
    return true;
}
#undef CHECK_PARSE_ERROR
} // namespace AppCUI::Utils