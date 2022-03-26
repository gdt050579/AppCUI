#include <AppCUI.hpp>
#include "Internal.hpp"
#include <unordered_map>

using namespace AppCUI;
using namespace Utils;
using namespace Input;

using BuffPtr = const uint8*;

#define WRAPPER ((Ini::Parser*) Data)

#define VALIDATE_VALUE(returnValue)                                                                                    \
    CHECK(Data, returnValue, "Invalid value object (null)");                                                           \
    Ini::Value* value = (Ini::Value*) this->Data;

#define PREPARE_VALUE                                                                                                  \
    if (!Data)                                                                                                         \
        return;                                                                                                        \
    Ini::Value* iniValue = (Ini::Value*) this->Data;

#define WRITE_INI_NUMERIC_VALUE                                                                                        \
    PREPARE_VALUE;                                                                                                     \
    NumericFormatter n;                                                                                                \
    iniValue->KeyValue = n.ToDec(value);                                                                               \
    iniValue->KeyValues.clear();

#define VALIDATE_INITED(returnValue)                                                                                   \
    CHECK(Data, returnValue, "Parser object has not been created. Have you called one of the Crete... methods first ?");

// we use the firts 4 bits for an enum like value, and the rest of the 4 bits for a bitmask
// bitmask is required to fasten some parse operations

constexpr uint8 CHAR_TYPE_OTHER         = 0;
constexpr uint8 CHAR_TYPE_SPACE         = 1;
constexpr uint8 CHAR_TYPE_STRING        = 2;
constexpr uint8 CHAR_TYPE_EQ            = 3;
constexpr uint8 CHAR_TYPE_SECTION_START = 4;
constexpr uint8 CHAR_TYPE_SECTION_END   = 5;
constexpr uint8 CHAR_TYPE_COMMA         = 6;
// max value is 0x0F

// bitflags
constexpr uint8 CHAR_TYPE_COMMENT        = 0x10;
constexpr uint8 CHAR_TYPE_NEW_LINE       = 0x20;
constexpr uint8 CHAR_TYPE_COMMENT_OR_NL  = 0x30;
constexpr uint8 CHAR_TYPE_WORD           = 0x40;
constexpr uint8 CHAR_TYPE_NUMBER         = 0x80;
constexpr uint8 CHAR_TYPE_WORD_OR_NUMBER = 0xC0;

#define PARSER_CHECK(condition, returnValue, errorMessage)                                                             \
    if (!(condition))                                                                                                  \
    {                                                                                                                  \
        SetError(errorMessage);                                                                                        \
        RETURNERROR(returnValue, errorMessage);                                                                        \
    }

const uint8 Ini_LoweCaseTable[256] = {
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
uint8 Ini_Char_Type[256] = { CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_SPACE,       CHAR_TYPE_NEW_LINE, CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_NEW_LINE,    CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_SPACE,  CHAR_TYPE_OTHER,       CHAR_TYPE_STRING,   CHAR_TYPE_COMMENT,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_STRING,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_COMMA,  CHAR_TYPE_OTHER,       CHAR_TYPE_WORD,     CHAR_TYPE_OTHER,
                             CHAR_TYPE_NUMBER, CHAR_TYPE_NUMBER,      CHAR_TYPE_NUMBER,   CHAR_TYPE_NUMBER,
                             CHAR_TYPE_NUMBER, CHAR_TYPE_NUMBER,      CHAR_TYPE_NUMBER,   CHAR_TYPE_NUMBER,
                             CHAR_TYPE_NUMBER, CHAR_TYPE_NUMBER,      CHAR_TYPE_OTHER,    CHAR_TYPE_COMMENT,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_EQ,          CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_WORD,        CHAR_TYPE_WORD,     CHAR_TYPE_WORD,
                             CHAR_TYPE_WORD,   CHAR_TYPE_WORD,        CHAR_TYPE_WORD,     CHAR_TYPE_WORD,
                             CHAR_TYPE_WORD,   CHAR_TYPE_WORD,        CHAR_TYPE_WORD,     CHAR_TYPE_WORD,
                             CHAR_TYPE_WORD,   CHAR_TYPE_WORD,        CHAR_TYPE_WORD,     CHAR_TYPE_WORD,
                             CHAR_TYPE_WORD,   CHAR_TYPE_WORD,        CHAR_TYPE_WORD,     CHAR_TYPE_WORD,
                             CHAR_TYPE_WORD,   CHAR_TYPE_WORD,        CHAR_TYPE_WORD,     CHAR_TYPE_WORD,
                             CHAR_TYPE_WORD,   CHAR_TYPE_WORD,        CHAR_TYPE_WORD,     CHAR_TYPE_SECTION_START,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_SECTION_END, CHAR_TYPE_OTHER,    CHAR_TYPE_WORD,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_WORD,        CHAR_TYPE_WORD,     CHAR_TYPE_WORD,
                             CHAR_TYPE_WORD,   CHAR_TYPE_WORD,        CHAR_TYPE_WORD,     CHAR_TYPE_WORD,
                             CHAR_TYPE_WORD,   CHAR_TYPE_WORD,        CHAR_TYPE_WORD,     CHAR_TYPE_WORD,
                             CHAR_TYPE_WORD,   CHAR_TYPE_WORD,        CHAR_TYPE_WORD,     CHAR_TYPE_WORD,
                             CHAR_TYPE_WORD,   CHAR_TYPE_WORD,        CHAR_TYPE_WORD,     CHAR_TYPE_WORD,
                             CHAR_TYPE_WORD,   CHAR_TYPE_WORD,        CHAR_TYPE_WORD,     CHAR_TYPE_WORD,
                             CHAR_TYPE_WORD,   CHAR_TYPE_WORD,        CHAR_TYPE_WORD,     CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER,
                             CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,       CHAR_TYPE_OTHER,    CHAR_TYPE_OTHER };

constexpr uint32 INI_VALUE_ON    = 0x6E6FU;
constexpr uint32 INI_VALUE_OFF   = 0x66666FU;
constexpr uint32 INI_VALUE_YES   = 0x736579U;
constexpr uint32 INI_VALUE_NO    = 0x6F6EU;
constexpr uint32 INI_VALUE_TRUE  = 0x65757274U;
constexpr uint32 INI_VALUE_FALSE = 0x736C6166;

namespace AppCUI
{
namespace Ini
{
    enum class ParseState : uint32
    {
        ExpectingKeyOrSection,
        ExpectingEQ,
        ExpectingValue,
        ExpectingArray
    };
    struct Value
    {
        std::string KeyName;
        std::string KeyValue;
        vector<std::string> KeyValues;
        Value()
        {
        }
        Value(string_view keyName)
        {
            KeyName = keyName;
        }
    };
    struct Section
    {
        Utils::String Name;
        std::unordered_map<uint64, Value> Keys;
        Section()
        {
        }
        Section(string_view name)
        {
            Name.Set(name.data(), (uint32) name.length());
        }
    };
    struct Parser
    {
        BuffPtr start;
        BuffPtr end;
        BuffPtr current;
        ParseState state;
        std::string toStringBuffer;

        std::unordered_map<uint64, unique_ptr<Ini::Section>> Sections;
        Section DefaultSection; // KeyValue entries that do not have a section name (writtem directly in the root)
        Section* CurrentSection;
        uint64 CurrentKeyHash;
        BuffPtr CurrentKeyNamePtr;
        uint32 CurrentKeyNameLen;

        inline void SkipSpaces();
        inline void SkipArrayDelimiters();
        inline void SkipNewLine();
        inline void SkipWord();
        inline void SkipCurrentLine();
        inline bool SkipString(bool& multiLineFormat);
        inline void SkipSingleLineWord(BuffPtr& wordEnds);
        inline void SkipArrayWord();

        void SetError(const char*)
        {
        }
        bool AddSection(BuffPtr nameStart, BuffPtr nameEnd);
        bool AddValue(BuffPtr valueStart, BuffPtr valueEnd);
        bool AddArrayValue(Ini::Value& value, BuffPtr valueStart, BuffPtr valueEnd);
        bool ParseState_ExpectingKeyOrSection();
        bool ParseState_ExpectingEQ();
        bool ParseState_ExpectingValue();
        bool ParseState_ExpectingArray();

        bool Parse(BuffPtr bufferStart, BuffPtr bufferEnd);
        void Clear();
    };
}; // namespace Ini
} // namespace AppCUI

uint64 __compute_hash__(BuffPtr p_start, BuffPtr p_end)
{
    // use FNV algorithm ==> https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
    uint64 hash = 0xcbf29ce484222325ULL;
    while (p_start < p_end)
    {
        hash = hash ^ (Ini_LoweCaseTable[*p_start]);
        hash = hash * 0x00000100000001B3ULL;
        p_start++;
    }
    return hash;
}
uint64 __compute_hash__(string_view text)
{
    return __compute_hash__((BuffPtr) text.data(), ((BuffPtr) text.data()) + text.length());
}
void AddSectionValueToString(std::string& res, std::string value)
{
    // quick_check
    auto spaces        = 0;
    auto quotes        = 0;
    auto double_quotes = 0;
    auto new_lines     = 0;
    // for empty strings --> add as ""
    if (value.length() == 0)
    {
        res += "\"\"";
        return;
    }
    for (auto ch : value)
    {
        if ((ch == ' ') || (ch == '\t'))
            spaces++;
        if (ch == '"')
            double_quotes++;
        if (ch == '\'')
            quotes++;
        if ((ch == '\n') || (ch == '\r'))
            new_lines++;
    }
    string_view string_separator = "";
    if (new_lines > 0)
        string_separator = "\"\"\"";
    else
    {
        if ((quotes > 0) && (double_quotes == 0))
            string_separator = "\"";
        else if ((quotes == 0) && (double_quotes > 0))
            string_separator = "'";
        else if ((quotes > 0) && (double_quotes > 0))
            string_separator = "\"\"\"";
        else
        {
            // no quotes or double_quotes or new_lines
            if (spaces > 0)
                string_separator = "\"";
        }
    }
    res += string_separator;
    res += value;
    res += string_separator;
}
void AddSectionToString(std::string& res, Ini::Section& sect)
{
    res += "\n";
    if (sect.Name.Len() > 0)
    {
        res += "[";
        res += sect.Name;
        res += "]";
        res += "\n";
    }
    // add values
    for (auto& entry : sect.Keys)
    {
        res += entry.second.KeyName;
        res += " = ";
        if (entry.second.KeyValues.size() > 0)
        {
            auto sz = entry.second.KeyValues.size();
            res += "[";
            for (size_t index = 0; index < sz; index++)
            {
                if (index > 0)
                    res += " , ";
                AddSectionValueToString(res, entry.second.KeyValues[index]);
            }
            res += "]";
        }
        else
        {
            AddSectionValueToString(res, entry.second.KeyValue);
        }
        res += "\n";
    }
}

void Ini::Parser::SkipSpaces()
{
    while ((current < end) && (Ini_Char_Type[*current] == CHAR_TYPE_SPACE))
        current++;
}
void Ini::Parser::SkipArrayDelimiters()
{
    while ((current < end) &&
           ((Ini_Char_Type[*current] == CHAR_TYPE_SPACE) || (Ini_Char_Type[*current] == CHAR_TYPE_NEW_LINE)))
        current++;
    if (current < end)
    {
        if ((*current) == ',')
            current++;
        // skip other spaces or new lines
        while ((current < end) &&
               ((Ini_Char_Type[*current] == CHAR_TYPE_SPACE) || (Ini_Char_Type[*current] == CHAR_TYPE_NEW_LINE)))
            current++;
    }
}
void Ini::Parser::SkipNewLine()
{
    while ((current < end) && (Ini_Char_Type[*current] == CHAR_TYPE_NEW_LINE))
        current++;
}
void Ini::Parser::SkipWord()
{
    while ((current < end) && (Ini_Char_Type[*current] & CHAR_TYPE_WORD_OR_NUMBER))
        current++;
}
void Ini::Parser::SkipCurrentLine()
{
    while ((current < end) && (Ini_Char_Type[*current] != CHAR_TYPE_NEW_LINE))
        current++;
}
void Ini::Parser::SkipSingleLineWord(BuffPtr& wordEnds)
{
    // asume it starts with a valid character (not a space)
    // we'll have to parse until we find a comment or a new line
    // skip spaces from the end (word will be trimmed)
    BuffPtr p_start = current;
    while ((current < end) && (!(Ini_Char_Type[*current] & CHAR_TYPE_COMMENT_OR_NL)))
        current++;
    if (current < end)
    {
        // remove the ending spaces
        wordEnds = current - 1;
        while ((wordEnds > p_start) && (Ini_Char_Type[*wordEnds] == CHAR_TYPE_SPACE))
            wordEnds--;
        wordEnds++;
    }
    else
    {
        wordEnds = end;
    }
}
void Ini::Parser::SkipArrayWord()
{
    // asume it starts with a valid character (not a space)
    // we'll have to parse until we find a space, a terminator or a new line

    while (current < end)
    {
        auto type = Ini_Char_Type[*current];
        if ((type == CHAR_TYPE_SPACE) || (type == CHAR_TYPE_NEW_LINE) || (type == CHAR_TYPE_SECTION_END))
            break;
        if ((*current) == ',')
            break;
        current++;
    }
}
bool Ini::Parser::SkipString(bool& multiLineFormat)
{
    // asume that current character is either ' or "
    uint8 currentChar = *current;
    if ((current + 2) < end)
        multiLineFormat = ((current[1] == currentChar) && (current[2] == currentChar));
    else
        multiLineFormat = false;

    if (multiLineFormat)
    {
        current += 3;
        BuffPtr s_end = end - 2;
        while (current < s_end)
        {
            if (((*current) == currentChar) && (current[1] == current[2]) && (current[0] == current[1]))
            {
                current += 3;
                return true;
            }
            current++;
        }
        SetError("Premaure end of a multi-line string");
        return false;
    }
    else
    {
        current++;
        while ((current < end) && ((*current) != currentChar) && (Ini_Char_Type[*current] != CHAR_TYPE_NEW_LINE))
            current++;
        PARSER_CHECK((current < end) && ((*current) == currentChar), false, "Premature end of a string !");
        current++;
        return true;
    }
}
bool Ini::Parser::ParseState_ExpectingKeyOrSection()
{
    BuffPtr nameStart, nameEnd;

    while (current < end)
    {
        switch (Ini_Char_Type[*current])
        {
        case CHAR_TYPE_SPACE:
            SkipSpaces();
            break;
        case CHAR_TYPE_NEW_LINE:
            SkipNewLine();
            break;
        case CHAR_TYPE_COMMENT:
            SkipCurrentLine();
            break;
        case CHAR_TYPE_SECTION_START:
            current++;
            SkipSpaces();
            PARSER_CHECK(current < end, false, "Premature end of INI section!");
            PARSER_CHECK(
                  Ini_Char_Type[*current] == CHAR_TYPE_WORD,
                  false,
                  "Expecting a valid name for a section (should start with a letter and be followed by letters, "
                  "number, underline or point)");
            nameStart = current;
            SkipWord();
            nameEnd = current;
            SkipSpaces();
            PARSER_CHECK(current < end, false, "Premature end of INI section!");
            PARSER_CHECK(Ini_Char_Type[*current] == CHAR_TYPE_SECTION_END, false, "Expecting a section delimiter ']'");
            current++;
            // all good - we have a section name ==> add-it to the map
            return AddSection(nameStart, nameEnd);
        case CHAR_TYPE_WORD:
            nameStart = current;
            SkipWord();
            nameEnd = current;
            SkipSpaces();
            // all good - store the key to be added after
            CurrentKeyHash    = __compute_hash__(nameStart, nameEnd);
            CurrentKeyNamePtr = nameStart;
            CurrentKeyNameLen = (uint32) (nameEnd - nameStart);
            state             = ParseState::ExpectingEQ;
            return true;
        default:
            SetError("Expecting either a section '[...]' or a key !");
            RETURNERROR(false, "Expecting either a section '[...]' or a key !");
        }
    }
    return true;
}
bool Ini::Parser::ParseState_ExpectingEQ()
{
    SkipSpaces();
    PARSER_CHECK(current < end, false, "Premature end of INI file: expecting '=' after a key !");
    PARSER_CHECK(Ini_Char_Type[*current] == CHAR_TYPE_EQ, false, "Expecting '=' after a key !");
    current++; // skip '=' character
    SkipSpaces();
    PARSER_CHECK(current < end, false, "Premature end of INI file: expecting a value after '=' character !");
    if ((*current) == '[')
        state = ParseState::ExpectingArray;
    else
        state = ParseState::ExpectingValue;
    return true;
}
bool Ini::Parser::ParseState_ExpectingValue()
{
    bool multiLineString;
    BuffPtr valueStart, valueEnd;
    SkipSpaces();
    // sanity check
    PARSER_CHECK(current < end, false, "Premature end of INI file: expecting a value after '=' character !");
    switch (Ini_Char_Type[*current])
    {
    case CHAR_TYPE_STRING:
        valueStart = current;
        CHECK(SkipString(multiLineString), false, "Fail parsing a string buffer !");
        if (multiLineString)
        {
            CHECK(AddValue(valueStart + 3, current - 3), false, "Fail to add multi-line string");
        }
        else
        {
            CHECK(AddValue(valueStart + 1, current - 1), false, "Fail to add single-line string");
        }
        // all good
        state = ParseState::ExpectingKeyOrSection;
        break;
    case CHAR_TYPE_WORD:
    case CHAR_TYPE_NUMBER:
    case CHAR_TYPE_OTHER:
        valueStart = current;
        SkipSingleLineWord(valueEnd);
        CHECK(AddValue(valueStart, valueEnd), false, "Fail to add word value");
        state = ParseState::ExpectingKeyOrSection;
        break;
    default:
        SetError("Expecting a value (a string, a number, etc)");
        RETURNERROR(false, "Expecting a value (a string, a number, etc)");
    }
    return true;
}
bool Ini::Parser::ParseState_ExpectingArray()
{
    bool multiLineString;
    BuffPtr valueStart;
    // it is assume that current points to a '[' character
    current++;
    // sanity check
    PARSER_CHECK(current < end, false, "Premature end of INI file: expecting a value after '[' character !");
    // all good --> create the value
    auto& value       = CurrentSection->Keys[this->CurrentKeyHash];
    value.KeyName     = string_view((const char*) CurrentKeyNamePtr, CurrentKeyNameLen);
    CurrentKeyNamePtr = nullptr;
    CurrentKeyNameLen = 0;
    // value is created and is empy
    while (true)
    {
        switch (Ini_Char_Type[*current])
        {
        case CHAR_TYPE_STRING:
            valueStart = current;
            CHECK(SkipString(multiLineString), false, "Fail parsing a string buffer !");
            if (multiLineString)
            {
                CHECK(AddArrayValue(value, valueStart + 3, current - 3), false, "Fail to add multi-line string");
            }
            else
            {
                CHECK(AddArrayValue(value, valueStart + 1, current - 1), false, "Fail to add single-line string");
            }
            SkipArrayDelimiters();
            break;
        case CHAR_TYPE_WORD:
        case CHAR_TYPE_NUMBER:
        case CHAR_TYPE_OTHER:
            valueStart = current;
            SkipArrayWord();
            CHECK(AddArrayValue(value, valueStart, current), false, "Fail to add word value");
            SkipArrayDelimiters();
            break;
        case CHAR_TYPE_SECTION_END:
            current++;
            state = ParseState::ExpectingKeyOrSection;
            return true;
        default:
            SetError("Expecting a value (a string, a number, etc)");
            RETURNERROR(false, "Expecting a value (a string, a number, etc)");
        }
        PARSER_CHECK(current < end, false, "Premature end of INI file: expecting ']' character !");
    }
    return true;
}
void Ini::Parser::Clear()
{
    this->Sections.clear();
    this->DefaultSection.Keys.clear();
    this->CurrentSection    = &this->DefaultSection;
    this->CurrentKeyHash    = 0;
    this->CurrentKeyNamePtr = nullptr;
    this->CurrentKeyNameLen = 0;
}
bool Ini::Parser::Parse(BuffPtr bufferStart, BuffPtr bufferEnd)
{
    Clear();
    // sanity check
    CHECK(bufferStart, false, "Expecting a valid value for internal 'bufferStart' pointer");
    CHECK(bufferStart <= bufferEnd, false, "Expecting a valid value for internal 'bufferEnd' pointer");
    // reset
    start   = bufferStart;
    end     = bufferEnd;
    current = start;
    state   = Ini::ParseState::ExpectingKeyOrSection;
    while (current < end)
    {
        switch (state)
        {
        case Ini::ParseState::ExpectingKeyOrSection:
            CHECK(ParseState_ExpectingKeyOrSection(), false, "");
            break;
        case Ini::ParseState::ExpectingEQ:
            CHECK(ParseState_ExpectingEQ(), false, "");
            break;
        case Ini::ParseState::ExpectingValue:
            CHECK(ParseState_ExpectingValue(), false, "");
            break;
        case Ini::ParseState::ExpectingArray:
            CHECK(ParseState_ExpectingArray(), false, "");
            break;
        default:
            RETURNERROR(false, "Internal error -> state (%d) was not implemented", (uint32) state);
            break;
        }
    }
    return true;
}
bool Ini::Parser::AddSection(BuffPtr nameStart, BuffPtr nameEnd)
{
    uint64 hash = __compute_hash__(nameStart, nameEnd);
    auto& sect  = Sections[hash];
    if (sect.get() == nullptr)
    {
        sect.reset(new Section());
    }
    CurrentSection = sect.get();
    CHECK(CurrentSection, false, "Fail to allocate a section");
    if (CurrentSection->Name.Len() == 0)
    {
        CHECK(CurrentSection->Name.Set((const char*) nameStart, (uint32) (nameEnd - nameStart)),
              false,
              "Fail to allocate section name");
    }
    return true;
}
bool Ini::Parser::AddValue(BuffPtr valueStart, BuffPtr valueEnd)
{
    CHECK(valueStart <= valueEnd, false, "Invalid buffer pointers !");
    auto& value       = CurrentSection->Keys[this->CurrentKeyHash];
    value.KeyValue    = string_view((const char*) valueStart, (uint32) (valueEnd - valueStart));
    value.KeyName     = string_view((const char*) CurrentKeyNamePtr, CurrentKeyNameLen);
    CurrentKeyNamePtr = nullptr;
    CurrentKeyNameLen = 0;
    return true;
}
bool Ini::Parser::AddArrayValue(Ini::Value& value, BuffPtr valueStart, BuffPtr valueEnd)
{
    CHECK(valueStart <= valueEnd, false, "Invalid buffer pointers !");
    value.KeyValues.push_back(std::string(string_view((const char*) valueStart, (uint32) (valueEnd - valueStart))));
    return true;
}
//============================================================================= INI Section iterator ===
IniSection::Iterator::Iterator(void* data)
{
    static_assert(sizeof(Iterator::data) >= sizeof(std::unordered_map<uint64, IniValue>::iterator));
    auto* it = reinterpret_cast<std::unordered_map<uint64, IniValue>::iterator*>(data);
    new (this->data) std::unordered_map<uint64, IniValue>::iterator(*it);
}
IniSection::Iterator& IniSection::Iterator::operator++()
{
    (*((std::unordered_map<uint64, IniValue>::iterator*) &this->data))++;
    return *this;
}
bool IniSection::Iterator::operator!=(const Iterator& it)
{
    return (*((std::unordered_map<uint64, IniValue>::iterator*) &this->data)) !=
           (*((std::unordered_map<uint64, IniValue>::iterator*) &it.data));
}
IniValue IniSection::Iterator::operator*()
{
    return IniValue(&((*((std::unordered_map<uint64, IniValue>::iterator*) &this->data))->second));
}
//============================================================================= INI Section ============
string_view IniSection::GetName() const
{
    CHECK(this->Data, "", "");
    return string_view{ ((Ini::Section*) Data)->Name.GetText(), ((Ini::Section*) Data)->Name.Len() };
}
IniValue IniSection::GetValue(string_view keyName)
{
    CHECK(Data, IniValue(), "Section key does not exists (unable to get key-value datat!)");
    Ini::Section* entry = ((Ini::Section*) Data);
    auto value          = entry->Keys.find(__compute_hash__(keyName));
    CHECK(value != entry->Keys.cend(), IniValue(), "Unable to find key !");
    // all good -> value exists
    return IniValue(&value->second);
}
bool IniSection::HasValue(string_view keyName)
{
    CHECK(Data, false, "Section key does not exists (unable to get key-value datat!)");
    Ini::Section* entry = ((Ini::Section*) Data);
    auto value          = entry->Keys.find(__compute_hash__(keyName));
    return value != entry->Keys.cend();
}
IniValue IniSection::operator[](string_view keyName)
{
    CHECK(Data, IniValue(), "Section key does not exists (unable to get key-value datat!)");
    Ini::Section* entry = ((Ini::Section*) Data);
    auto hash           = __compute_hash__(keyName);
    auto value          = entry->Keys.find(hash);
    // if element already exists --> return it
    if (value != entry->Keys.cend())
        return IniValue(&value->second);
    auto res = entry->Keys.emplace(hash, Ini::Value(keyName));
    return IniValue(&res.first->second);
}
vector<IniValue> IniSection::GetValues() const
{
    CHECK(Data, vector<IniValue>(), "Section was not initialized");

    vector<IniValue> res;
    auto sect = ((Ini::Section*) Data);

    res.reserve(sect->Keys.size());
    for (auto& v : sect->Keys)
    {
        res.push_back(IniValue(&v.second));
    }

    return res;
}

IniSection::Iterator IniSection::begin()
{
    auto it = ((Ini::Section*) Data)->Keys.begin();
    return Iterator(&it);
}
IniSection::Iterator IniSection::end()
{
    auto it = ((Ini::Section*) Data)->Keys.end();
    return Iterator(&it);
}
void IniSection::Clear()
{
    if (this->Data)
        ((Ini::Section*) Data)->Keys.clear();
}
bool IniSection::DeleteValue(string_view keyName)
{
    CHECK(Data, false, "Section key does not exists (unable to get key-value datat!)");
    ((Ini::Section*) Data)->Keys.erase(__compute_hash__(keyName));
    return true;
}
template <typename T>
void UpdateValueForSection(void* sectionData, string_view name, T value, bool dontUpdateIfValueExits)
{
    if (!sectionData)
        return;
    Ini::Section* entry = ((Ini::Section*) sectionData);

    if (dontUpdateIfValueExits)
    {
        auto hash = __compute_hash__(name);
        if (entry->Keys.contains(hash))
            return;
    }
    IniSection sect(sectionData);
    sect[name] = value;
}
void IniSection::UpdateValue(string_view name, bool value, bool dontUpdateIfValueExits)
{
    UpdateValueForSection<bool>(this->Data, name, value, dontUpdateIfValueExits);
}
void IniSection::UpdateValue(string_view name, uint32 value, bool dontUpdateIfValueExits)
{
    UpdateValueForSection<uint32>(this->Data, name, value, dontUpdateIfValueExits);
}
void IniSection::UpdateValue(string_view name, uint64 value, bool dontUpdateIfValueExits)
{
    UpdateValueForSection<uint64>(this->Data, name, value, dontUpdateIfValueExits);
}
void IniSection::UpdateValue(string_view name, int32 value, bool dontUpdateIfValueExits)
{
    UpdateValueForSection<int32>(this->Data, name, value, dontUpdateIfValueExits);
}
void IniSection::UpdateValue(string_view name, int64 value, bool dontUpdateIfValueExits)
{
    UpdateValueForSection<int64>(this->Data, name, value, dontUpdateIfValueExits);
}
void IniSection::UpdateValue(string_view name, float value, bool dontUpdateIfValueExits)
{
    UpdateValueForSection<float>(this->Data, name, value, dontUpdateIfValueExits);
}
void IniSection::UpdateValue(string_view name, double value, bool dontUpdateIfValueExits)
{
    UpdateValueForSection<double>(this->Data, name, value, dontUpdateIfValueExits);
}
void IniSection::UpdateValue(string_view name, const char* value, bool dontUpdateIfValueExits)
{
    UpdateValueForSection<const char*>(this->Data, name, value, dontUpdateIfValueExits);
}
void IniSection::UpdateValue(string_view name, string_view value, bool dontUpdateIfValueExits)
{
    UpdateValueForSection<string_view>(this->Data, name, value, dontUpdateIfValueExits);
}
void IniSection::UpdateValue(string_view name, Graphics::Size value, bool dontUpdateIfValueExits)
{
    UpdateValueForSection<Graphics::Size>(this->Data, name, value, dontUpdateIfValueExits);
}
void IniSection::UpdateValue(string_view name, Input::Key value, bool dontUpdateIfValueExits)
{
    UpdateValueForSection<Input::Key>(this->Data, name, value, dontUpdateIfValueExits);
}
void IniSection::UpdateValue(string_view name, const initializer_list<std::string>& values, bool dontUpdateIfValueExits)
{
    UpdateValueForSection<const initializer_list<std::string>&>(this->Data, name, values, dontUpdateIfValueExits);
}
void IniSection::UpdateValue(string_view name, const initializer_list<const char*>& values, bool dontUpdateIfValueExits)
{
    UpdateValueForSection<const initializer_list<const char*>&>(this->Data, name, values, dontUpdateIfValueExits);
}
void IniSection::UpdateValue(string_view name, const initializer_list<bool>& values, bool dontUpdateIfValueExits)
{
    UpdateValueForSection<const initializer_list<bool>&>(this->Data, name, values, dontUpdateIfValueExits);
}
void IniSection::UpdateValue(string_view name, const initializer_list<int32>& values, bool dontUpdateIfValueExits)
{
    UpdateValueForSection<const initializer_list<int32>&>(this->Data, name, values, dontUpdateIfValueExits);
}
void IniSection::UpdateValue(string_view name, const initializer_list<int64>& values, bool dontUpdateIfValueExits)
{
    UpdateValueForSection<const initializer_list<int64>&>(this->Data, name, values, dontUpdateIfValueExits);
}
void IniSection::UpdateValue(string_view name, const initializer_list<uint32>& values, bool dontUpdateIfValueExits)
{
    UpdateValueForSection<const initializer_list<uint32>&>(this->Data, name, values, dontUpdateIfValueExits);
}
void IniSection::UpdateValue(string_view name, const initializer_list<uint64>& values, bool dontUpdateIfValueExits)
{
    UpdateValueForSection<const initializer_list<uint64>&>(this->Data, name, values, dontUpdateIfValueExits);
}
void IniSection::UpdateValue(string_view name, const initializer_list<float>& values, bool dontUpdateIfValueExits)
{
    UpdateValueForSection<const initializer_list<float>&>(this->Data, name, values, dontUpdateIfValueExits);
}
void IniSection::UpdateValue(string_view name, const initializer_list<double>& values, bool dontUpdateIfValueExits)
{
    UpdateValueForSection<const initializer_list<double>&>(this->Data, name, values, dontUpdateIfValueExits);
}
//============================================================================= INI Value ===

optional<bool> IniValue_ToBool(const char* txt, uint32 len)
{
    auto v = 0U;
    switch (len)
    {
    case 1:
        if ((*txt) == '1')
            return true;
        if ((*txt) == '0')
            return false;
        break;
    case 2:
        v = (*((const uint16*) txt)) | 0x2020;
        if (v == INI_VALUE_ON)
            return true;
        if (v == INI_VALUE_NO)
            return false;
        break;
    case 3:
        // in fact there are 4 bytes (3 for the text followed by 0)
        v = (*((const uint32*) txt)) | 0x202020;
        if (v == INI_VALUE_YES)
            return true;
        if (v == INI_VALUE_OFF)
            return false;
        break;
    case 4:
        v = (*((const uint32*) txt)) | 0x20202020;
        if (v == INI_VALUE_TRUE)
            return true;
        break;
    case 5:
        v = (*((const uint32*) txt)) | 0x20202020;
        if ((v == INI_VALUE_FALSE) && ((txt[4] | 0x20) == 'e'))
            return false;
        break;
    default:
        break;
    }
    RETURNERROR(
          std::nullopt, "value can not be converted into a bool (accepted values are 'yes', 'no', 'true' or 'false'")
}

optional<uint64> IniValue::AsUInt64() const
{
    VALIDATE_VALUE(std::nullopt);
    return Number::ToUInt64(static_cast<string_view>(value->KeyValue));
}
optional<int64> IniValue::AsInt64() const
{
    VALIDATE_VALUE(std::nullopt);
    return Number::ToInt64(static_cast<string_view>(value->KeyValue));
}
optional<uint32> IniValue::AsUInt32() const
{
    VALIDATE_VALUE(std::nullopt);
    return Number::ToUInt32(static_cast<string_view>(value->KeyValue));
}
optional<int32> IniValue::AsInt32() const
{
    VALIDATE_VALUE(std::nullopt);
    return Number::ToInt32(static_cast<string_view>(value->KeyValue));
}
optional<bool> IniValue::AsBool() const
{
    VALIDATE_VALUE(std::nullopt);
    return IniValue_ToBool(value->KeyValue.c_str(), (uint32) value->KeyValue.length());
}
optional<Input::Key> IniValue::AsKey() const
{
    VALIDATE_VALUE(std::nullopt);
    Key k = KeyUtils::FromString((static_cast<string_view>(value->KeyValue)));
    if (k == Key::None)
        return std::nullopt;
    return k;
}
optional<const char*> IniValue::AsString() const
{
    VALIDATE_VALUE(std::nullopt);
    return value->KeyValue.c_str();
}
optional<string_view> IniValue::AsStringView() const
{
    VALIDATE_VALUE(std::nullopt);
    return static_cast<string_view>(value->KeyValue);
}
optional<Graphics::Size> IniValue::AsSize() const
{
    VALIDATE_VALUE(std::nullopt);
    return Graphics::Size::FromString(value->KeyValue);
}
optional<float> IniValue::AsFloat() const
{
    VALIDATE_VALUE(std::nullopt);
    return Number::ToFloat((static_cast<string_view>(value->KeyValue)));
}
optional<double> IniValue::AsDouble() const
{
    VALIDATE_VALUE(std::nullopt);
    return Number::ToDouble((static_cast<string_view>(value->KeyValue)));
}
optional<Graphics::Color> IniValue::AsColor() const
{
    VALIDATE_VALUE(std::nullopt);
    return ColorUtils::GetColor(static_cast<string_view>(value->KeyValue));
}
optional<Graphics::ColorPair> IniValue::AsColorPair() const
{
    VALIDATE_VALUE(std::nullopt);
    return ColorUtils::GetColorPair(static_cast<string_view>(value->KeyValue));
}

uint64 IniValue::ToUInt64(uint64 defaultValue) const
{
    auto result = this->AsUInt64();
    if (result.has_value())
        return result.value();
    else
        return defaultValue;
}
uint32 IniValue::ToUInt32(uint32 defaultValue) const
{
    auto result = this->AsUInt32();
    if (result.has_value())
        return result.value();
    else
        return defaultValue;
}
int64 IniValue::ToInt64(int64 defaultValue) const
{
    auto result = this->AsInt64();
    if (result.has_value())
        return result.value();
    else
        return defaultValue;
}
int32 IniValue::ToInt32(int32 defaultValue) const
{
    auto result = this->AsInt32();
    if (result.has_value())
        return result.value();
    else
        return defaultValue;
}
bool IniValue::ToBool(bool defaultValue) const
{
    auto result = this->AsBool();
    if (result.has_value())
        return result.value();
    else
        return defaultValue;
}
Input::Key IniValue::ToKey(Input::Key defaultValue) const
{
    auto result = this->AsKey();
    if (result.has_value())
        return result.value();
    else
        return defaultValue;
}
const char* IniValue::ToString(const char* defaultValue) const
{
    VALIDATE_VALUE(defaultValue);
    return value->KeyValue.c_str();
}
string_view IniValue::ToStringView(string_view defaultValue) const
{
    auto result = this->AsStringView();
    if (result.has_value())
        return result.value();
    else
        return defaultValue;
}
Graphics::Size IniValue::ToSize(Graphics::Size defaultValue) const
{
    auto result = this->AsSize();
    if (result.has_value())
        return result.value();
    else
        return defaultValue;
}
float IniValue::ToFloat(float defaultValue) const
{
    auto result = this->AsFloat();
    if (result.has_value())
        return result.value();
    else
        return defaultValue;
}
double IniValue::ToDouble(double defaultValue) const
{
    auto result = this->AsDouble();
    if (result.has_value())
        return result.value();
    else
        return defaultValue;
}
Graphics::Color IniValue::ToColor(Graphics::Color defaultColor) const
{
    auto result = this->AsColor();
    if (result.has_value())
        return result.value();
    else
        return defaultColor;
}
Graphics::ColorPair IniValue::ToColorPair(Graphics::ColorPair defaultColorPair) const
{
    auto result = this->AsColorPair();
    if (result.has_value())
        return result.value();
    else
        return defaultColorPair;
}

string_view IniValue::GetName() const
{
    VALIDATE_VALUE(string_view());
    return (static_cast<string_view>(value->KeyName));
}

bool IniValue::IsArray() const
{
    VALIDATE_VALUE(false);
    return value->KeyValues.size() > 0;
}
uint32 IniValue::GetArrayCount() const
{
    VALIDATE_VALUE(0);
    return (uint32) value->KeyValues.size();
}
IniValueArray IniValue::operator[](int32 index) const
{
    VALIDATE_VALUE(IniValueArray());
    if ((index < 0) || (index >= value->KeyValues.size()))
        return IniValueArray();

    return IniValueArray((string_view) value->KeyValues[index]);
}

void IniValue::operator=(bool value)
{
    PREPARE_VALUE;
    if (value)
        iniValue->KeyValue = "true";
    else
        iniValue->KeyValue = "false";
    iniValue->KeyValues.clear();
}
void IniValue::operator=(uint32 value)
{
    WRITE_INI_NUMERIC_VALUE;
}
void IniValue::operator=(uint64 value)
{
    WRITE_INI_NUMERIC_VALUE;
}
void IniValue::operator=(int32 value)
{
    WRITE_INI_NUMERIC_VALUE;
}
void IniValue::operator=(int64 value)
{
    WRITE_INI_NUMERIC_VALUE;
}
void IniValue::operator=(float value)
{
    PREPARE_VALUE;
    LocalString<64> tmp;
    iniValue->KeyValue = tmp.Format("%.3f", value);
    iniValue->KeyValues.clear();
}
void IniValue::operator=(double value)
{
    PREPARE_VALUE;
    LocalString<64> tmp;
    iniValue->KeyValue = tmp.Format("%.3lf", value);
    iniValue->KeyValues.clear();
}
void IniValue::operator=(string_view value)
{
    PREPARE_VALUE;
    iniValue->KeyValue = value;
    iniValue->KeyValues.clear();
}
void IniValue::operator=(const char* value)
{
    PREPARE_VALUE;
    iniValue->KeyValue = value;
    iniValue->KeyValues.clear();
}
void IniValue::operator=(Graphics::Size value)
{
    PREPARE_VALUE;
    LocalString<64> tmp;
    iniValue->KeyValue = tmp.Format("%u x %u", value.Width, value.Height);
    iniValue->KeyValues.clear();
}
void IniValue::operator=(Input::Key value)
{
    PREPARE_VALUE;
    LocalString<64> tmp;
    if (!Utils::KeyUtils::ToString(value, tmp))
        return;
    iniValue->KeyValue = tmp;
    iniValue->KeyValues.clear();
}
void IniValue::operator=(const initializer_list<std::string>& values)
{
    PREPARE_VALUE;
    iniValue->KeyValue.clear();
    iniValue->KeyValues = values;
}
void IniValue::operator=(const initializer_list<const char*>& values)
{
    PREPARE_VALUE;
    iniValue->KeyValue.clear();
    iniValue->KeyValues.clear();
    iniValue->KeyValues.reserve(values.size());
    for (auto val : values)
    {
        iniValue->KeyValues.push_back(val);
    }
}
void IniValue::operator=(const initializer_list<bool>& values)
{
    PREPARE_VALUE;
    iniValue->KeyValue.clear();
    iniValue->KeyValues.clear();
    iniValue->KeyValues.reserve(values.size());
    for (auto val : values)
    {
        iniValue->KeyValues.push_back(val ? "true" : "false");
    }
}
template <typename T>
void IniValueSetVector(void* Data, const initializer_list<T>& values)
{
    if (!Data)
        return;
    Ini::Value* iniValue = (Ini::Value*) Data;
    iniValue->KeyValue.clear();
    iniValue->KeyValues.clear();
    iniValue->KeyValues.reserve(values.size());
    NumericFormatter n;
    for (auto val : values)
    {
        iniValue->KeyValues.push_back(std::string(n.ToDec(val)));
    }
}
void IniValue::operator=(const initializer_list<uint32>& values)
{
    IniValueSetVector<uint32>(this->Data, values);
}
void IniValue::operator=(const initializer_list<uint64>& values)
{
    IniValueSetVector<uint64>(this->Data, values);
}
void IniValue::operator=(const initializer_list<int32>& values)
{
    IniValueSetVector<int32>(this->Data, values);
}
void IniValue::operator=(const initializer_list<int64>& values)
{
    IniValueSetVector<int64>(this->Data, values);
}
void IniValue::operator=(const initializer_list<float>& values)
{
    IniValueSetVector<float>(this->Data, values);
}
void IniValue::operator=(const initializer_list<double>& values)
{
    IniValueSetVector<double>(this->Data, values);
}
//============================================================================= INI Array Value ===
optional<uint64> IniValueArray::AsUInt64() const
{
    return Number::ToUInt64(string_view(text, len));
}
optional<int64> IniValueArray::AsInt64() const
{
    return Number::ToInt64(string_view(text, len));
}
optional<uint32> IniValueArray::AsUInt32() const
{
    return Number::ToUInt32(string_view(text, len));
}
optional<int32> IniValueArray::AsInt32() const
{
    return Number::ToInt32(string_view(text, len));
}
optional<bool> IniValueArray::AsBool() const
{
    return IniValue_ToBool(text, len);
}
optional<Input::Key> IniValueArray::AsKey() const
{
    Key k = KeyUtils::FromString(string_view(text, len));
    if (k == Key::None)
        return std::nullopt;
    return k;
}
optional<Graphics::Size> IniValueArray::AsSize() const
{
    return Graphics::Size::FromString(string_view{ text, (size_t) len });
}
optional<float> IniValueArray::AsFloat() const
{
    return Number::ToFloat(string_view(text, len));
}
optional<double> IniValueArray::AsDouble() const
{
    return Number::ToDouble(string_view(text, len));
}

uint64 IniValueArray::ToUInt64(uint64 defaultValue) const
{
    auto result = this->AsUInt64();
    if (result.has_value())
        return result.value();
    else
        return defaultValue;
}
uint32 IniValueArray::ToUInt32(uint32 defaultValue) const
{
    auto result = this->AsUInt32();
    if (result.has_value())
        return result.value();
    else
        return defaultValue;
}
int64 IniValueArray::ToInt64(int64 defaultValue) const
{
    auto result = this->AsInt64();
    if (result.has_value())
        return result.value();
    else
        return defaultValue;
}
int32 IniValueArray::ToInt32(int32 defaultValue) const
{
    auto result = this->AsInt32();
    if (result.has_value())
        return result.value();
    else
        return defaultValue;
}
bool IniValueArray::ToBool(bool defaultValue) const
{
    auto result = this->AsBool();
    if (result.has_value())
        return result.value();
    else
        return defaultValue;
}
Input::Key IniValueArray::ToKey(Input::Key defaultValue) const
{
    auto result = this->AsKey();
    if (result.has_value())
        return result.value();
    else
        return defaultValue;
}
const char* IniValueArray::ToString(const char* /*defaultValue*/) const
{
    return text;
}
string_view IniValueArray::ToStringView(string_view defaultValue) const
{
    auto result = this->AsStringView();
    if (result.has_value())
        return result.value();
    else
        return defaultValue;
}
Graphics::Size IniValueArray::ToSize(Graphics::Size defaultValue) const
{
    auto result = this->AsSize();
    if (result.has_value())
        return result.value();
    else
        return defaultValue;
}
float IniValueArray::ToFloat(float defaultValue) const
{
    auto result = this->AsFloat();
    if (result.has_value())
        return result.value();
    else
        return defaultValue;
}
double IniValueArray::ToDouble(double defaultValue) const
{
    auto result = this->AsDouble();
    if (result.has_value())
        return result.value();
    else
        return defaultValue;
}
//============================================================================= INI object iterator ===
using IniObjectIterator = std::unordered_map<uint64, unique_ptr<Ini::Section>>::iterator;
IniObject::Iterator::Iterator(void* data)
{
    static_assert(
          sizeof(Iterator::data) >= sizeof(IniObjectIterator),
          "Please make sure that the size of IniObject::Iterator::data is at least the size of "
          "IniObject::Sections::iterator");
    auto* it = reinterpret_cast<IniObjectIterator*>(data);
    new (this->data) IniObjectIterator(*it);
}
IniObject::Iterator& IniObject::Iterator::operator++()
{
    (*((IniObjectIterator*) &this->data))++;
    return *this;
}
bool IniObject::Iterator::operator!=(const Iterator& it)
{
    return (*((IniObjectIterator*) &this->data)) != (*((IniObjectIterator*) &it.data));
}
IniSection IniObject::Iterator::operator*()
{
    return IniSection(&((*((IniObjectIterator*) &this->data))->second));
}
//============================================================================= INI Object ===
IniObject::IniObject()
{
    Data = nullptr;
}
IniObject::~IniObject()
{
    if (Data)
        delete ((Ini::Parser*) Data);
    Data = nullptr;
}
bool IniObject::Init()
{
    if (Data == nullptr)
    {
        Data = new Ini::Parser();
        CHECK(Data, false, "Fail to allocate memory for object parser !");
    }
    return true;
}
bool IniObject::CreateFromString(string_view text)
{
    CHECK(text.data(), false, "Expecting a valid (non-null) string !");
    CHECK(Init(), false, "Fail to initialize parser object !");
    BuffPtr start = (BuffPtr) text.data();
    BuffPtr end   = start + text.length();
    CHECK(WRAPPER->Parse(start, end), false, "Fail to parser buffer !");
    return true;
}
bool IniObject::CreateFromFile(const std::filesystem::path& fileName)
{
    auto buf = OS::File::ReadContent(fileName);
    CHECK(buf.IsValid(), false, "Unable to read content of ini file: %s", fileName.string().c_str());
    return CreateFromString(buf);
}
bool IniObject::Save(const std::filesystem::path& fileName)
{
    auto iniContent = this->ToString();
    CHECK(!iniContent.empty(), false, "Fail to create ini content !");
    OS::File f;
    CHECK(f.Create(fileName, true), false, "Fail to create file: %s", fileName.string().c_str());
    if (iniContent.size())
    {
        if (!f.Write(iniContent))
        {
            LOG_ERROR("Fail to write ini content to file: %s", fileName.string().c_str());
            f.Close();
            return false;
        }
    }
    f.Close();
    return true;
}
bool IniObject::Create()
{
    CHECK(Init(), false, "Fail to initialize parser object !");
    WRAPPER->Clear();
    return true;
}
void IniObject::Clear()
{
    if (this->Data)
    {
        WRAPPER->Clear();
    }
}
bool IniObject::DeleteSection(string_view name)
{
    VALIDATE_INITED(false);
    // null-strings or empty strings refer to the Default section that always exists
    if ((name.data() == nullptr) || (name.length() == 0))
    {
        WRAPPER->DefaultSection.Keys.clear();
        return true;
    }
    WRAPPER->Sections.erase(__compute_hash__(name));
    return true;
}
bool IniObject::HasSection(string_view name) const
{
    VALIDATE_INITED(false);
    // null-strings or empty strings refer to the Default section that always exists
    if ((name.data() == nullptr) || (name.length() == 0))
        return true;
    return WRAPPER->Sections.contains(__compute_hash__(name));
}
IniSection IniObject::GetSection(string_view name)
{
    VALIDATE_INITED(IniSection());
    if ((name.data() == nullptr) || (name.length() == 0))
        return IniSection(&(WRAPPER->DefaultSection));
    auto result = WRAPPER->Sections.find(__compute_hash__(name));
    if (result == WRAPPER->Sections.cend())
        return IniSection();
    return IniSection(result->second.get());
}
IniSection IniObject::CreateSection(string_view name, bool emptyContent)
{
    if (this->Data == nullptr)
    {
        CHECK(this->Create(), IniSection(nullptr), "Fail to create INI object !");
    }
    // if no name is provided --> return the default section
    if ((name.data() == nullptr) || (name.length() == 0))
    {
        if (emptyContent)
            WRAPPER->DefaultSection.Keys.clear();
        return IniSection(&(WRAPPER->DefaultSection));
    }
    // check if the section exists
    auto hash   = __compute_hash__(name);
    auto result = WRAPPER->Sections.find(hash);
    if (result != WRAPPER->Sections.cend())
    {
        if (emptyContent)
            result->second->Keys.clear();
        return IniSection(result->second.get());
    }
    // create a new section
    auto res = WRAPPER->Sections.emplace(hash, std::make_unique<Ini::Section>(name));
    return IniSection(res.first->second.get());
}
vector<IniSection> IniObject::GetSections() const
{
    VALIDATE_INITED(vector<IniSection>());
    vector<IniSection> res;
    res.reserve(WRAPPER->Sections.size());
    for (auto& s : WRAPPER->Sections)
        res.push_back(IniSection(s.second.get()));
    return res;
}
IniObject::Iterator IniObject::begin()
{
    auto it = WRAPPER->Sections.begin();
    return IniObject::Iterator(&it);
}
IniObject::Iterator IniObject::end()
{
    auto it = WRAPPER->Sections.end();
    return IniObject::Iterator(&it);
}
IniValue IniObject::GetValue(string_view valuePath)
{
    // valuePath is in the form "sectionName/sectionValue" or just "sectionValue" for default section
    VALIDATE_INITED(IniValue());
    const uint8* start = (const uint8*) valuePath.data();
    CHECK(start, IniValue(), "Invalid value path (expecting a non-null object)");
    const uint8* end = start + valuePath.size();
    CHECK(start < end, IniValue(), "Invalid value path (expecting a non-empty object)");
    const uint8* p = start;
    while ((p < end) && ((*p) != '/') && ((*p) != '\\'))
        p++;
    if (p >= end)
    {
        // no section was provided --> using the default one
        auto value = WRAPPER->DefaultSection.Keys.find(__compute_hash__(start, end));
        CHECK(value != WRAPPER->DefaultSection.Keys.cend(), IniValue(), "Unable to find key for default section!");
        // all good -> value exists
        return IniValue(&value->second);
    }
    else
    {
        // we have both a section and a value name
        auto result = WRAPPER->Sections.find(__compute_hash__(start, p));
        if (result == WRAPPER->Sections.cend())
            return IniValue();
        Ini::Section* sect = result->second.get();
        CHECK(sect, IniValue(), "Invalid section (null)");
        p++;
        CHECK(p < end, IniValue(), "Missing value from path !");
        auto value = sect->Keys.find(__compute_hash__(p, end));
        CHECK(value != sect->Keys.cend(), IniValue(), "Unable to find key for section: %s", sect->Name.GetText());
        // all good -> value exists
        return IniValue(&value->second);
    }
}
bool IniObject::DeleteValue(string_view valuePath)
{
    // valuePath is in the form "sectionName/sectionValue" or just "sectionValue" for default section
    VALIDATE_INITED(false);
    const uint8* start = (const uint8*) valuePath.data();
    CHECK(start, false, "Invalid value path (expecting a non-null object)");
    const uint8* end = start + valuePath.size();
    CHECK(start < end, false, "Invalid value path (expecting a non-empty object)");
    const uint8* p = start;
    while ((p < end) && ((*p) != '/') && ((*p) != '\\'))
        p++;
    if (p >= end)
    {
        // no section was provided --> using the default one
        WRAPPER->DefaultSection.Keys.erase(__compute_hash__(start, end));
        return true;
    }
    else
    {
        // we have both a section and a value name
        auto result = WRAPPER->Sections.find(__compute_hash__(start, p));
        if (result == WRAPPER->Sections.cend())
            return false;
        Ini::Section* sect = result->second.get();
        CHECK(sect, false, "Invalid section (null)");
        p++;
        CHECK(p < end, false, "Missing value from path !");
        sect->Keys.erase(__compute_hash__(p, end));
        return true;
    }
}
uint32 IniObject::GetSectionsCount()
{
    VALIDATE_INITED(0);
    return (uint32) WRAPPER->Sections.size();
}

string_view IniObject::ToString()
{
    VALIDATE_INITED(string_view());
    WRAPPER->toStringBuffer.reserve(4096);
    WRAPPER->toStringBuffer.clear();

    // add default section
    AddSectionToString(WRAPPER->toStringBuffer, WRAPPER->DefaultSection);
    // add rest of the sections
    for (auto& entry : WRAPPER->Sections)
    {
        AddSectionToString(WRAPPER->toStringBuffer, *entry.second);
    }
    // return result
    return (string_view) WRAPPER->toStringBuffer;
}

#undef WRAPPER
