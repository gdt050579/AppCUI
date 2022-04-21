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

template <typename T>
class Parser
{
    const T* start;
    const T* end;
    const T* current;

    inline void SkipSpaces()
    {
        while ((current < end) && ((*current) < MAX_CHARS_IN_TABLE) &&
               ((ParserCharacterTypes[*current]) == CHAR_TYPE_SPACE))
            current++;
    }
    inline void ParseWord()
    {
        while ((current < end) &&
               ((((*current) < MAX_CHARS_IN_TABLE) && ((ParserCharacterTypes[*current]) == CHAR_TYPE_OTHER)) ||
                ((*current) >= MAX_CHARS_IN_TABLE)))
            current++;
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
  public:
    Parser(const T* _start, size_t size)
    {
        start = current = _start;
        end             = start + size;
    }
    bool Next()
    {
        SkipSpaces();
        if (current >= end)
            return false; // end the process
        if (IsWord())
    }
};
bool KeyValueParser::Parse(std::string_view text)
{
    Parser<uint8> p((const uint8*) text.data(), text.size());
}
bool KeyValueParser::Parse(std::u16string_view text)
{
    NOT_IMPLEMENTED(false);
}
} // namespace AppCUI::Utils