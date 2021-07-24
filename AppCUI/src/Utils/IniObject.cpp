#include "AppCUI.h"
#include "Internal.h"
#include <map>

using BuffPtr = const unsigned char*;


#define CHAR_TYPE_OTHER             0
#define CHAR_TYPE_SPACE             1
#define CHAR_TYPE_STRING            2
#define CHAR_TYPE_COMMENT           3
#define CHAR_TYPE_EQ                4
#define CHAR_TYPE_SECTION_START     5
#define CHAR_TYPE_SECTION_END       6
#define CHAR_TYPE_COMMA             7
#define CHAR_TYPE_NEW_LINE          8

// bitflags
#define CHAR_TYPE_WORD              0x40
#define CHAR_TYPE_NUMBER            0x80
#define CHAR_TYPE_WORD_OR_NUMBER    0xC0

#define PARSER_CHECK(condition,returnValue,errorMessage)   if (!(condition)) { SetError(errorMessage); RETURNERROR(returnValue,errorMessage);}


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
        class Key
        {
            //std::map<std::string_view,std::variant<> // to analye !!!
        };
        class Parser
        {
            BuffPtr     start;
            BuffPtr     end;
            BuffPtr     current;
            ParseState  state;

            inline void SkipSpaces();
            inline void SkipNewLine();
            inline void SkipWord();
            inline void SkipCurrentLine();

            void SetError(const char* message) { }
            bool AddSection(BuffPtr nameStart, BuffPtr nameEnd);
            bool ParseState_ExpectingKeyOrSection();
            bool ParseState_ExpectingEQ();
            bool ParseState_ExpectingValue();
            bool Parse();
        };
    };
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
bool AppCUI::Ini::Parser::ParseState_ExpectingKeyOrSection()
{
    BuffPtr nameStart, nameEnd;
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
                // all good - we have a section name ==> add-it to the map
                return AddSection(nameStart, nameEnd);
            case CHAR_TYPE_WORD:
                nameStart = current;
                SkipWord();
                nameEnd = current;
                SkipSpaces();
                // all good - store the key to be added after
                state = ParseState::ExpectingEQ;
                return false;
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
    NOT_IMPLEMENTED(false);
}
bool AppCUI::Ini::Parser::Parse()
{
    // sanity check
    CHECK(start, false, "Expecting a valid value for internal 'start' pointer");
    CHECK(start<end, false, "Expecting a valid value for internal 'end' pointer");
    current = start;
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
    NOT_IMPLEMENTED(false);
}
