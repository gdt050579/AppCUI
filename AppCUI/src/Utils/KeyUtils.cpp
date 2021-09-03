#include "AppCUI.hpp"
#include <string.h>

struct __KeyAndSize__
{
    const char* Name;
    unsigned int NameSize;
};
static constexpr std::string_view _Key_Modifiers[8] = {
    /* 0 */ "",
    /* 1 */ "Alt+",
    /* 2 */ "Ctrl+", 
    /* 3 */ "Ctrl+Alt+", 
    /* 4 */ "Shift+", 
    /* 5 */ "Alt+Shift+", 
    /* 6 */ "Ctrl+Shift+", 
    /* 7 */ "Ctrl+Alt+Shift+", 
};
static constexpr std::string_view _Key_Name[] = {
    "",      "F1",     "F2",       "F3",     "F4",     "F5",     "F6",        "F7",  "F8",   "F9", "F10",
    "F11",   "F12",    "Enter",    "Escape", "Insert", "Delete", "Backspace", "Tab", "Left", "Up", "Down",
    "Righ",  "PageUp", "PageDown", "Home",   "End",    "Space",  "A",         "B",   "C",    "D",  "E",
    "F",     "G",      "H",        "I",      "J",      "K",      "L",         "M",   "N",    "O",  "P",
    "Q",     "R",      "S",        "T",      "U",      "V",      "W",         "X",   "Y",    "Z",  "0",
    "1",     "2",      "3",        "4",      "5",      "6",      "7",         "8",   "9",
};
static constexpr std::string_view _Key_Name_Padded[] = {
    "",      " F1 ",   " F2 ",    " F3 ",     " F4 ",       " F5 ",     " F6 ",     " F7 ",        " F8 ",  " F9 ",
    " F10 ", " F11 ",  " F12 ",   " Enter ",  " Escape ",   " Insert ", " Delete ", " Backspace ", " Tab ", " Left ",
    " Up ",  " Down ", " Right ", " PageUp ", " PageDown ", " Home ",   " End ",    " Space ",     " A ",   " B ",
    " C ",   " D ",    " E ",     " F ",      " G ",        " H ",      " I ",      " J ",         " K ",   " L ",
    " M ",   " N ",    " O ",     " P ",      " Q ",        " R ",      " S ",      " T ",         " U ",   " V ",
    " W ",   " X ",    " Y ",     " Z ",      " 0 ",        " 1 ",      " 2 ",      " 3 ",         " 4 ",   " 5 ",
    " 6 ",   " 7 ",    " 8 ",     " 9 ",
};

std::string_view AppCUI::Utils::KeyUtils::GetKeyName(AppCUI::Input::Key keyCode)
{
    unsigned int keyIndex = ((unsigned int) keyCode) & 0xFF;
    if ((keyIndex >= (sizeof(_Key_Name) / sizeof(std::string_view))))
        return std::string_view("",0);
    return _Key_Name[keyIndex];
}
std::string_view AppCUI::Utils::KeyUtils::GetKeyNamePadded(AppCUI::Input::Key keyCode)
{
    unsigned int keyIndex = ((unsigned int) keyCode) & 0xFF;
    if ((keyIndex >= (sizeof(_Key_Name_Padded) / sizeof(__KeyAndSize__))))
        return std::string_view("", 0);
    return _Key_Name_Padded[keyIndex];
}
std::string_view AppCUI::Utils::KeyUtils::GetKeyModifierName(AppCUI::Input::Key keyCode)
{
    unsigned int keyIndex = (((unsigned int) keyCode) >> KEY_SHIFT_BITS) & 0x7;
    if (keyIndex > 7)
        return std::string_view("", 0);
    return _Key_Modifiers[keyIndex];
}
bool AppCUI::Utils::KeyUtils::ToString(AppCUI::Input::Key keyCode, char* text, int maxTextSize)
{
    CHECK(text != nullptr, false, "");
    AppCUI::Utils::String s;
    CHECK(s.Create(text, maxTextSize, true), false, "");
    CHECK(ToString(keyCode, s), false, "");
    return true;
}
bool AppCUI::Utils::KeyUtils::ToString(AppCUI::Input::Key keyCode, AppCUI::Utils::String& text)
{
    const char* m = GetKeyModifierName(keyCode).data();
    const char* k = GetKeyName(keyCode).data();
    CHECK(text.Set(""), false, "");
    CHECK((m != nullptr) && (k != nullptr), false, "");
    CHECK(text.Set(m), false, "");
    CHECK(text.Add(k), false, "");
    return true;
}
AppCUI::Input::Key AppCUI::Utils::KeyUtils::FromString(const std::string_view& stringRepresentation)
{
    
    unsigned int code     = 0;
    unsigned int modifier = 0;
    if (stringRepresentation.data() == nullptr)
        return AppCUI::Input::Key::None;
    if (stringRepresentation.length() == 0)
        return AppCUI::Input::Key::None;  
    if (stringRepresentation.length() > 48)
        return AppCUI::Input::Key::None;
    // temporary solution
    char Key[64];
    memcpy(Key, stringRepresentation.data(), stringRepresentation.length());
    
    // make sure that we have the trailing \0
    Key[stringRepresentation.length()] = 0;
    const char* key                    = Key;

    // automat de stari - pentru modifier
    for (int tr = 0; tr < 3; tr++)
    {
        if (Utils::String::StartsWith(key, "Alt+"))
        {
            modifier |= 1;
            key += 4;
            continue;
        }
        if (Utils::String::StartsWith(key, "Ctrl+"))
        {
            modifier |= 2;
            key += 5;
            continue;
        }
        if (Utils::String::StartsWith(key, "Shift+"))
        {
            modifier |= 4;
            key += 6;
            continue;
        }
        break;
    }
    auto *p = &_Key_Name[1];
    for (unsigned int tr = 1; tr < sizeof(_Key_Name) / sizeof(_Key_Name[1]); tr++, p++)
    {
        if (Utils::String::Equals(key, p->data()))
        {
            code = tr;
            break;
        }
    }
    if (code == 0)
        return AppCUI::Input::Key::None;
    return (AppCUI::Input::Key)((modifier << KEY_SHIFT_BITS) | code);
}
