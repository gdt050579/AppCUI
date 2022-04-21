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
              ((ParserCharacterTypes[*current]) == CHAR_TYPE_OTHER));
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
        while ((s < e) && (((*s) < '0') || (*s > '9')))
        {
            firstPart = firstPart * 10 + (int32) ((*s) - '0');
            s++;
        }
        if ((s < e) && ((*s) == '.'))
        {
            while ((s < e) && (((*s) < '0') || (*s > '9')))
            {
                secondPart = secondPart * 10 + (int32) ((*s) - '0');
                s++;
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
        if (negative)
            firstPart = -firstPart;
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
        return true;
    }
    inline uint32 ComputeHash(const T* s, const T* e)
    {
        return 0;
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
        if (valueStart)
        {
            // we have key = value => key = string
            pair.Key.number = 0;
            pair.Key.hash   = ComputeHash(keyStart, keyEnd);
            pair.Key.type   = KeyValuePair::Type::String;
            // pair.Key.String.ascii
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
            pair.Value.type = KeyValuePair::Type::None;
        }
        return true;
    }
};
bool KeyValueParser::Parse(std::string_view text)
{
    Parser<uint8> p((const uint8*) text.data(), text.size());
    KeyValuePair* k = this->items;
    KeyValuePair* e = k + KeyValueParser::MAX_ITEMS;
    this->count     = 0;

    while ((k < e) && (p.Next(*k)))
        k++;
    this->count = (uint32)(k - this->items);
    return true;
}
bool KeyValueParser::Parse(std::u16string_view text)
{
    NOT_IMPLEMENTED(false);
}
#undef CHECK_PARSE_ERROR
} // namespace AppCUI::Utils