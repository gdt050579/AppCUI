#include "../../include/AppCUI.h"

static const char* _Key_Modifiers[8] = {
	/* 0 */ "",
	/* 1 */ "Alt",
	/* 2 */ "Ctrl",
	/* 3 */ "Ctrl+Alt",
	/* 4 */ "Shift",
	/* 5 */ "Alt+Shift",
	/* 6 */ "Ctrl+Shift",
	/* 7 */ "Ctrl+Alt+Shift",
};
static const char* _Key_Name[] = {
	nullptr,
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
	"Enter", "Escape", "Insert", "Delete", "Backspace", "Tab",
	"Left", "Up", "Down", "Righ", "PageUp", "PageDown", "Home", "End", "Space",
	"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
};
const char*	                AppCUI::Utils::KeyUtils::GetKeyName(AppCUI::Input::Key::Type keyCode)
{
	unsigned int keyIndex = keyCode & 0xFF;
	if ((keyIndex >= (sizeof(_Key_Name) / sizeof(const char*))))
		return nullptr;
	return _Key_Name[keyIndex];
}
const char*	                AppCUI::Utils::KeyUtils::GetKeyModifierName(AppCUI::Input::Key::Type keyCode)
{
    unsigned int keyIndex = (keyCode >> KEY_SHIFT_BITS) & 0x7;
	if (keyIndex >7)
		return nullptr;
	return _Key_Modifiers[keyIndex];
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
	if (m[0] != 0) {
		CHECK(text.AddChar('+'), false, "");
	}
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