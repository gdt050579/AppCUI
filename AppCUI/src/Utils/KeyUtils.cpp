#include "AppCUI.h"

struct __KeyAndSize__ {
    const char * Name;
    unsigned int NameSize;
};
static __KeyAndSize__ _Key_Modifiers[8] = {
    /* 0 */ {"",                0},
    /* 1 */ {"Alt+",            4},
    /* 2 */ {"Ctrl+",           5},
    /* 3 */ {"Ctrl+Alt+",       9},
    /* 4 */ {"Shift+",          6},
    /* 5 */ {"Alt+Shift+",      10},
    /* 6 */ {"Ctrl+Shift+",     11},
    /* 7 */ {"Ctrl+Alt+Shift+", 15}
};
static const char* _Key_Name[] = {
	nullptr,
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
	"Enter", "Escape", "Insert", "Delete", "Backspace", "Tab",
	"Left", "Up", "Down", "Righ", "PageUp", "PageDown", "Home", "End", "Space",
	"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
};
static __KeyAndSize__ _Key_Name_Padded[] = {
    {"",0},
    {" F1 ",4}, {" F2 ",4}, {" F3 ",4}, {" F4 ",4}, {" F5 ",4}, {" F6 ",4}, {" F7 ",4}, {" F8 ",4}, {" F9 ",4}, {" F10 ",5}, {" F11 ",5}, {" F12 ",5},
    {" Enter ",7}, {" Escape ",8}, {" Insert ",8}, {" Delete ",8}, {" Backspace ",11}, {" Tab ",5},
    {" Left ",6}, {" Up ",4}, {" Down ",6}, {" Right ",7}, {" PageUp ",8}, {" PageDown ",10}, {" Home ",6}, {" End ",5}, {" Space ",7},
    {" A ",3}, {" B ",3},{" C ",3},{" D ",3},{" E ",3},{" F ",3},{" G ",3},{" H ",3},{" I ",3},{" J ",3},{" K ",3},{" L ",3},{" M ",3},
    {" N ",3},{" O ",3},{" P ",3},{" Q ",3},{" R ",3},{" S ",3},{" T ",3},{" U ",3},{" V ",3},{" W ",3},{" X ",3},{" Y ",3},{" Z ",3},
    {" 0 ",3},{" 1 ",3},{" 2 ",3},{" 3 ",3},{" 4 ",3},{" 5 ",3},{" 6 ",3},{" 7 ",3},{" 8 ",3},{" 9 ",3},
};

const char*	                AppCUI::Utils::KeyUtils::GetKeyName(AppCUI::Input::Key::Type keyCode)
{
	unsigned int keyIndex = keyCode & 0xFF;
	if ((keyIndex >= (sizeof(_Key_Name) / sizeof(const char*))))
		return nullptr;
	return _Key_Name[keyIndex];
}
const char*	                AppCUI::Utils::KeyUtils::GetKeyNamePadded(AppCUI::Input::Key::Type keyCode, unsigned int * nameSize)
{
    unsigned int keyIndex = keyCode & 0xFF;
    if ((keyIndex >= (sizeof(_Key_Name_Padded) / sizeof(__KeyAndSize__))))
        return nullptr;
    __KeyAndSize__ * kas = _Key_Name_Padded + keyIndex;
    if (nameSize)
        (*nameSize) = kas->NameSize;
    return kas->Name;
}
const char*	                AppCUI::Utils::KeyUtils::GetKeyModifierName(AppCUI::Input::Key::Type keyCode, unsigned int * nameSize)
{
    unsigned int keyIndex = (keyCode >> KEY_SHIFT_BITS) & 0x7;
	if (keyIndex >7)
		return nullptr;
    __KeyAndSize__ * kas = _Key_Modifiers + keyIndex;
    if (nameSize)
        (*nameSize) = kas->NameSize;
    return kas->Name;
}
bool		                AppCUI::Utils::KeyUtils::ToString(AppCUI::Input::Key::Type keyCode, char* text, int maxTextSize)
{
	CHECK(text != nullptr, false, "");
	AppCUI::Utils::String s;
	CHECK(s.Create(text, maxTextSize, true), false, "");
	CHECK(ToString(keyCode,s), false, "");
	return true;
}
bool		                AppCUI::Utils::KeyUtils::ToString(AppCUI::Input::Key::Type keyCode, AppCUI::Utils::String *text)
{
	CHECK(text != nullptr, false, "");
	CHECK(ToString(keyCode,*text), false, "");
	return true;
}
bool		                AppCUI::Utils::KeyUtils::ToString(AppCUI::Input::Key::Type keyCode, AppCUI::Utils::String &text)
{
	const char * m = GetKeyModifierName(keyCode);
	const char * k = GetKeyName(keyCode);
	CHECK(text.Set(""), false, "");
	CHECK((m != nullptr) && (k != nullptr), false, "");
	CHECK(text.Set(m), false, "");
	CHECK(text.Add(k), false, "");
	return true;
}
AppCUI::Input::Key::Type     AppCUI::Utils::KeyUtils::FromString(const char * key)
{
	unsigned int code = 0;
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
	const char ** p = &_Key_Name[1];
	for (int tr = 1; tr < sizeof(_Key_Name) / sizeof(const char*); tr++,p++)
	{
		if (Utils::String::Equals(key, *p))
		{
			code = tr;
			break;
		}
	}
	if (code == 0)
		return AppCUI::Input::Key::None;
    return (AppCUI::Input::Key::Type)((modifier << KEY_SHIFT_BITS) | code);
}
AppCUI::Input::Key::Type     AppCUI::Utils::KeyUtils::FromString(AppCUI::Utils::String *text)
{
	CHECK(text != nullptr, AppCUI::Input::Key::None, "Invalid (nullptr) key code string representation !");
	return FromString(text->GetText());
}
AppCUI::Input::Key::Type     AppCUI::Utils::KeyUtils::FromString(AppCUI::Utils::String &text)
{
	return FromString(text.GetText());
}