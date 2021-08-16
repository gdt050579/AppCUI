#include "AppCUI.hpp"

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
static __KeyAndSize__ _Key_Name_Padded[] = {
    { "", 0 },         { " F1 ", 4 },     { " F2 ", 4 },         { " F3 ", 4 },     { " F4 ", 4 },
    { " F5 ", 4 },     { " F6 ", 4 },     { " F7 ", 4 },         { " F8 ", 4 },     { " F9 ", 4 },
    { " F10 ", 5 },    { " F11 ", 5 },    { " F12 ", 5 },        { " Enter ", 7 },  { " Escape ", 8 },
    { " Insert ", 8 }, { " Delete ", 8 }, { " Backspace ", 11 }, { " Tab ", 5 },    { " Left ", 6 },
    { " Up ", 4 },     { " Down ", 6 },   { " Right ", 7 },      { " PageUp ", 8 }, { " PageDown ", 10 },
    { " Home ", 6 },   { " End ", 5 },    { " Space ", 7 },      { " A ", 3 },      { " B ", 3 },
    { " C ", 3 },      { " D ", 3 },      { " E ", 3 },          { " F ", 3 },      { " G ", 3 },
    { " H ", 3 },      { " I ", 3 },      { " J ", 3 },          { " K ", 3 },      { " L ", 3 },
    { " M ", 3 },      { " N ", 3 },      { " O ", 3 },          { " P ", 3 },      { " Q ", 3 },
    { " R ", 3 },      { " S ", 3 },      { " T ", 3 },          { " U ", 3 },      { " V ", 3 },
    { " W ", 3 },      { " X ", 3 },      { " Y ", 3 },          { " Z ", 3 },      { " 0 ", 3 },
    { " 1 ", 3 },      { " 2 ", 3 },      { " 3 ", 3 },          { " 4 ", 3 },      { " 5 ", 3 },
    { " 6 ", 3 },      { " 7 ", 3 },      { " 8 ", 3 },          { " 9 ", 3 },
};

std::string_view AppCUI::Utils::KeyUtils::GetKeyName(AppCUI::Input::Key keyCode)
{
    unsigned int keyIndex = ((unsigned int) keyCode) & 0xFF;
    if ((keyIndex >= (sizeof(_Key_Name) / sizeof(std::string_view))))
        return std::string_view("",0);
    return _Key_Name[keyIndex];
}
const char* AppCUI::Utils::KeyUtils::GetKeyNamePadded(AppCUI::Input::Key keyCode, unsigned int* nameSize)
{
    unsigned int keyIndex = ((unsigned int) keyCode) & 0xFF;
    if ((keyIndex >= (sizeof(_Key_Name_Padded) / sizeof(__KeyAndSize__))))
        return nullptr;
    __KeyAndSize__* kas = _Key_Name_Padded + keyIndex;
    if (nameSize)
        (*nameSize) = kas->NameSize;
    return kas->Name;
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
AppCUI::Input::Key AppCUI::Utils::KeyUtils::FromString(const char* key)
{
    unsigned int code     = 0;
    unsigned int modifier = 0;

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
    for (int tr = 1; tr < sizeof(_Key_Name) / sizeof(_Key_Name[1]); tr++, p++)
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
AppCUI::Input::Key AppCUI::Utils::KeyUtils::FromString(AppCUI::Utils::String& text)
{
    return FromString(text.GetText());
}