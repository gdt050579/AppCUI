#include "AppCUI.hpp"
#include "Internal.hpp"
#include <unordered_map>


using namespace AppCUI::Utils;

using BuffPtr = const unsigned char*;

#define WRAPPER                             ((AppCUI::Ini::Parser*)Data)
#define PREPARE_KEYVALUE_ENTRY(returnValue) \
    CHECK(Data,returnValue,"Section key does not exists (unable to get key-value datat!)"); \
    AppCUI::Ini::KeyValueEntry* entry = ((AppCUI::Ini::KeyValueEntry*)Data); \
    auto value = entry->Keys.find(__compute_hash__(name)); \
    CHECK(value!=entry->Keys.cend(),returnValue,"Unable to find key !");


#define VALIDATE_INITED(returnValue)        CHECK(Data,returnValue,"Parser object has not been created. Have you called one of the Crete... methods first ?");

// we use the firts 4 bits for an enum like value, and the rest of the 4 bits for a bitmask
// bitmask is required to fasten some parse operations

#define CHAR_TYPE_OTHER             0
#define CHAR_TYPE_SPACE             1
#define CHAR_TYPE_STRING            2
#define CHAR_TYPE_EQ                3
#define CHAR_TYPE_SECTION_START     4
#define CHAR_TYPE_SECTION_END       5
#define CHAR_TYPE_COMMA             6
// max value is 0x0F

// bitflags
#define CHAR_TYPE_COMMENT           0x10
#define CHAR_TYPE_NEW_LINE          0x20
#define CHAR_TYPE_COMMENT_OR_NL     0x30
#define CHAR_TYPE_WORD              0x40
#define CHAR_TYPE_NUMBER            0x80
#define CHAR_TYPE_WORD_OR_NUMBER    0xC0

#define PARSER_CHECK(condition,returnValue,errorMessage)   if (!(condition)) { SetError(errorMessage); RETURNERROR(returnValue,errorMessage);}

const unsigned char __lower_case_table_for_hashing__[256] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255 };
unsigned char __char_type__[256] = { CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_SPACE,CHAR_TYPE_NEW_LINE,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_NEW_LINE,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_SPACE,CHAR_TYPE_OTHER,CHAR_TYPE_STRING,CHAR_TYPE_COMMENT,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_STRING,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_COMMA,CHAR_TYPE_OTHER,CHAR_TYPE_WORD,CHAR_TYPE_OTHER,CHAR_TYPE_NUMBER,CHAR_TYPE_NUMBER,CHAR_TYPE_NUMBER,CHAR_TYPE_NUMBER,CHAR_TYPE_NUMBER,CHAR_TYPE_NUMBER,CHAR_TYPE_NUMBER,CHAR_TYPE_NUMBER,CHAR_TYPE_NUMBER,CHAR_TYPE_NUMBER,CHAR_TYPE_OTHER,CHAR_TYPE_COMMENT,CHAR_TYPE_OTHER,CHAR_TYPE_EQ,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_SECTION_START,CHAR_TYPE_OTHER,CHAR_TYPE_SECTION_END,CHAR_TYPE_OTHER,CHAR_TYPE_WORD,CHAR_TYPE_OTHER,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER };

namespace AppCUI 
{
    namespace Ini
    {
        enum class ParseState : unsigned int
        {
            ExpectingKeyOrSection,
            ExpectingEQ,
            ExpectingValue
        };
        struct KeyValueEntry
        {
            std::unordered_map<unsigned long long, AppCUI::Utils::String> Keys;
        };
        struct Parser
        {
            BuffPtr     start;
            BuffPtr     end;
            BuffPtr     current;
            ParseState  state;

            std::unordered_map<unsigned long long, std::unique_ptr<AppCUI::Ini::KeyValueEntry>> Sections;
            KeyValueEntry       DefaultSection; // KeyValue entries that do not have a section name (writtem directly in the root)
            KeyValueEntry*      CurrentSection;
            unsigned long long  CurrentKeyHash;

            inline void SkipSpaces();
            inline void SkipNewLine();
            inline void SkipWord();
            inline void SkipCurrentLine();
            inline bool SkipString(bool &multiLineFormat);
            inline void SkipSingleLineWord(BuffPtr& wordEnds);

            void SetError(const char* message) { }
            bool AddSection(BuffPtr nameStart, BuffPtr nameEnd);
            bool AddValue(BuffPtr valueStart, BuffPtr valueEnd);
            bool ParseState_ExpectingKeyOrSection();
            bool ParseState_ExpectingEQ();
            bool ParseState_ExpectingValue();

            bool Parse(BuffPtr bufferStart, BuffPtr bufferEnd);
            void Clear();
        };
    };
}

unsigned long long __compute_hash__(BuffPtr p_start, BuffPtr p_end)
{
    // use FNV algorithm ==> https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
    unsigned long long hash = 0xcbf29ce484222325ULL;
    while (p_start < p_end)
    {
        hash = hash ^ (__lower_case_table_for_hashing__[*p_start]);
        hash = hash * 0x00000100000001B3ULL;
        p_start++;
    }
    return hash;
}
unsigned long long __compute_hash__(std::string_view text)
{
    return __compute_hash__((BuffPtr)text.data(), ((BuffPtr)text.data()) + text.length());
}

void AppCUI::Ini::Parser::SkipSpaces()
{
    while ((current < end) && (__char_type__[*current]==CHAR_TYPE_SPACE))
        current++;
}
void AppCUI::Ini::Parser::SkipNewLine() 
{
    while ((current < end) && (__char_type__[*current] == CHAR_TYPE_NEW_LINE))
        current++;
}
void AppCUI::Ini::Parser::SkipWord()
{
    while ((current < end) && (__char_type__[*current] & CHAR_TYPE_WORD_OR_NUMBER))
        current++;
}
void AppCUI::Ini::Parser::SkipCurrentLine()
{
    while ((current < end) && (__char_type__[*current] != CHAR_TYPE_NEW_LINE))
        current++;
}
void AppCUI::Ini::Parser::SkipSingleLineWord(BuffPtr& wordEnds)
{
    // asume it starts with a valid character (not a space)
    // we'll have to parse until we find a comment or a new line
    // skip spaces from the end (word will be trimmed)
    BuffPtr p_start = current;
    while ((current < end) && (!(__char_type__[*current] & CHAR_TYPE_COMMENT_OR_NL)))
        current++;
    if (current < end) 
    {
        // remove the ending spaces
        wordEnds = current - 1;
        while ((wordEnds > p_start) && (__char_type__[*wordEnds] == CHAR_TYPE_SPACE))
            wordEnds--;
        wordEnds++;
    }
}
bool AppCUI::Ini::Parser::SkipString(bool& multiLineFormat)
{
    // asume that current character is either ' or "
    unsigned char currentChar = *current;
    if ((current + 2) < end)
        multiLineFormat = ((current[1] == currentChar) && (current[2] == currentChar));
    else
        multiLineFormat = false;
    
    if (multiLineFormat)
    {
        current+=3;
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
    else {
        current++;
        while ((current < end) && ((*current)!=currentChar) && (__char_type__[*current] != CHAR_TYPE_NEW_LINE))
            current++;
        PARSER_CHECK((current < end) && ((*current)==currentChar), false, "Premature end of a string !");
        current++;
        return true;
    }    
}
bool AppCUI::Ini::Parser::ParseState_ExpectingKeyOrSection()
{
    BuffPtr             nameStart, nameEnd;
    
    while (current < end)
    {
        switch (__char_type__[*current])
        {
            case CHAR_TYPE_SPACE: SkipSpaces(); break;
            case CHAR_TYPE_NEW_LINE: SkipNewLine(); break;
            case CHAR_TYPE_COMMENT: SkipCurrentLine(); break;
            case CHAR_TYPE_SECTION_START:
                current++;
                SkipSpaces();
                PARSER_CHECK(current < end, false, "Premature end of INI section!");
                PARSER_CHECK(__char_type__[*current] == CHAR_TYPE_WORD, false, "Expecting a valid name for a section (should start with a letter and be followed by letters, number, underline or point)");
                nameStart = current;
                SkipWord();
                nameEnd = current;
                SkipSpaces();
                PARSER_CHECK(current < end, false, "Premature end of INI section!");
                PARSER_CHECK(__char_type__[*current] == CHAR_TYPE_SECTION_END, false, "Expecting a section delimiter ']'");
                current++;
                // all good - we have a section name ==> add-it to the map
                return AddSection(nameStart, nameEnd);
            case CHAR_TYPE_WORD:
                nameStart = current;
                SkipWord();
                nameEnd = current;
                SkipSpaces();
                // all good - store the key to be added after
                CurrentKeyHash = __compute_hash__(nameStart, nameEnd);
                state = ParseState::ExpectingEQ;
                return true;
            default:
                SetError("Expecting either a section '[...]' or a key !");
                RETURNERROR(false, "Expecting either a section '[...]' or a key !");
        }
    }
    return true;
}
bool AppCUI::Ini::Parser::ParseState_ExpectingEQ()
{
    SkipSpaces();
    PARSER_CHECK(current < end, false, "Premature end of INI file: expecting '=' after a key !");
    PARSER_CHECK(__char_type__[*current] == CHAR_TYPE_EQ, false, "Expecting '=' after a key !");
    current++; // skip '=' character
    state = ParseState::ExpectingValue;    
    return true;
}
bool AppCUI::Ini::Parser::ParseState_ExpectingValue()
{
    bool    multiLineString;
    BuffPtr valueStart,valueEnd;
    SkipSpaces();
    PARSER_CHECK(current < end, false, "Premature end of INI file: expecting a value after '=' character !");
    switch (__char_type__[*current])
    {
        case CHAR_TYPE_STRING:
            valueStart = current;
            CHECK(SkipString(multiLineString), false, "Fail parsing a string buffer !");
            if (multiLineString)
            {
                CHECK(AddValue(valueStart + 3, current - 3), false, "Fail to add multi-line string");
            } else {
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
void AppCUI::Ini::Parser::Clear()
{
    this->Sections.clear();
    this->DefaultSection.Keys.clear();
    this->CurrentSection = &this->DefaultSection;
    this->CurrentKeyHash = 0;
}
bool AppCUI::Ini::Parser::Parse(BuffPtr bufferStart, BuffPtr bufferEnd)
{
    Clear();
    // sanity check
    CHECK(bufferStart, false, "Expecting a valid value for internal 'bufferStart' pointer");
    CHECK(bufferStart <= bufferEnd, false, "Expecting a valid value for internal 'bufferEnd' pointer");
    // reset
    start = bufferStart;
    end = bufferEnd;
    current = start;
    state = AppCUI::Ini::ParseState::ExpectingKeyOrSection;
    while (current < end)
    {
        switch (state)
        {
        case AppCUI::Ini::ParseState::ExpectingKeyOrSection:
            CHECK(ParseState_ExpectingKeyOrSection(), false, "");
            break;
        case AppCUI::Ini::ParseState::ExpectingEQ:
            CHECK(ParseState_ExpectingEQ(), false, "");
            break;
        case AppCUI::Ini::ParseState::ExpectingValue:
            CHECK(ParseState_ExpectingValue(), false, "");
            break;
        default:
            RETURNERROR(false, "Internal error -> state (%d) was not implemented", (unsigned int)state);
            break;
        }
    }
    return true;
}
bool AppCUI::Ini::Parser::AddSection(BuffPtr nameStart, BuffPtr nameEnd)
{    
    unsigned long long hash = __compute_hash__(nameStart, nameEnd);
    auto &sect = Sections[hash];
    if (sect.get() == nullptr)
    {
        sect.reset(new KeyValueEntry());
    }
    CurrentSection = sect.get();
    CHECK(CurrentSection, false, "Fail to allocate a section");
    return true;
}
bool AppCUI::Ini::Parser::AddValue(BuffPtr valueStart, BuffPtr valueEnd)
{
    CHECK(valueStart <= valueEnd, false, "Invalid buffer pointers !");
    CHECK(CurrentSection->Keys[this->CurrentKeyHash].Set((const char *)valueStart, (unsigned int)(valueEnd - valueStart)), false, "Fail to add key-value pair");
    return true;
}


const char* IniSection::GetValue(std::string_view name)
{
    PREPARE_KEYVALUE_ENTRY(nullptr);
    return value->second.GetText();
}
bool        IniSection::CopyStringValue(std::string_view name,Utils::String& result) const
{
    PREPARE_KEYVALUE_ENTRY(false);
    CHECK(result.Set(value->second), false, "Fail to copy string %s", value->second.GetText());
    return true;
}
bool        IniSection::CopyBoolValue(std::string_view name, bool& result) const
{
    PREPARE_KEYVALUE_ENTRY(false);
    if ((value->second.Equals("true", true)) || (value->second.Equals("yes", true)))
    {
        result = true;
        return true;
    }
    if ((value->second.Equals("false", true)) || (value->second.Equals("no", true)))
    {
        result = false;
        return true;
    }
    RETURNERROR(false, "Key value (%s) can not be converted into a bool (accepted values are 'yes', 'no', 'true' or 'false'");
}

IniObject::IniObject() {
    Data = nullptr;
}
IniObject::~IniObject()
{
    if (Data)
        delete ((AppCUI::Ini::Parser*)Data);
    Data = nullptr;
}
bool        IniObject::Init()
{
    if (Data == nullptr)
    {
        Data = new AppCUI::Ini::Parser();
        CHECK(Data, false, "Fail to allocate memory for object parser !");
    }
    return true;
}
bool        IniObject::CreateFromString(std::string_view text)
{
    CHECK(text.data(), false, "Expecting a valid (non-null) string !");
    CHECK(Init(), false, "Fail to initialize parser object !");
    BuffPtr start = (BuffPtr)text.data();
    BuffPtr end = start + text.length();
    CHECK(WRAPPER->Parse(start, end), false, "Fail to parser buffer !");
    return true;
}
bool        IniObject::CreateFromFile(const char* fileName)
{
    NOT_IMPLEMENTED(false);
}
bool        IniObject::Create()
{
    CHECK(Init(), false, "Fail to initialize parser object !");
    WRAPPER->Clear();
    return true;
}

bool        IniObject::HasSection(std::string_view name)
{
    VALIDATE_INITED(false);
    // null-strings or empty strings refer to the Default section that always exists
    if ((name.data() == nullptr) || (name.length() == 0))
        return true;
    return WRAPPER->Sections.contains(__compute_hash__(name));
}
IniSection  IniObject::GetSection(std::string_view name)
{
    VALIDATE_INITED(IniSection());
    if ((name.data() == nullptr) || (name.length() == 0))
        return IniSection(&(WRAPPER->DefaultSection));
    auto result = WRAPPER->Sections.find(__compute_hash__(name));
    if (result == WRAPPER->Sections.cend())
        return IniSection();
    return IniSection(result->second.get());
}
unsigned int IniObject::GetSectionsCount()
{
    VALIDATE_INITED(0);
    return (unsigned int)WRAPPER->Sections.size();
}
#undef WRAPPER 
