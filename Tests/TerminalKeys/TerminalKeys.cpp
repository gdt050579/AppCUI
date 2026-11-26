// Checks the terminal key decisions of the ncurses frontend (AppCUI/src/Terminal/NcursesTerminal/TerminalKeyDecoding.hpp)
// against the byte sequences terminals really send. No terminal needed: runs on every platform. Exit code 0 = all passed.

#include "../../AppCUI/src/Terminal/NcursesTerminal/TerminalKeyDecoding.hpp"

#include <cstdio>
#include <set>

using namespace AppCUI::Internal::TerminalKeys;

static int failures = 0;
#define CHECK_TRUE(cond)                                                                                                    \
    do                                                                                                                       \
    {                                                                                                                        \
        if (!(cond))                                                                                                         \
        {                                                                                                                    \
            std::printf("FAILED %s:%d: %s\n", __FILE__, __LINE__, #cond);                                                    \
            failures++;                                                                                                      \
        }                                                                                                                    \
    } while (0)

static bool Decode(std::string_view utf8, char32_t& cp)
{
    return DecodeUtf8(reinterpret_cast<const unsigned char*>(utf8.data()), utf8.size(), cp);
}

static void ShiftedFunctionKeys()
{
    // macOS Terminal (Profiles -> Keyboard) sends these for Shift+F5 .. Shift+F12
    CHECK_TRUE(sizeof(MAC_SHIFTED_FUNCTION_KEYS) / sizeof(MAC_SHIFTED_FUNCTION_KEYS[0]) == 8);
    CHECK_TRUE(MAC_SHIFTED_FUNCTION_KEYS[0].sequence == "\x1b[25~" && MAC_SHIFTED_FUNCTION_KEYS[0].functionKey == 5);
    CHECK_TRUE(MAC_SHIFTED_FUNCTION_KEYS[2].sequence == "\x1b[28~" && MAC_SHIFTED_FUNCTION_KEYS[2].functionKey == 7);
    CHECK_TRUE(MAC_SHIFTED_FUNCTION_KEYS[4].sequence == "\x1b[31~" && MAC_SHIFTED_FUNCTION_KEYS[4].functionKey == 9);
    CHECK_TRUE(MAC_SHIFTED_FUNCTION_KEYS[7].sequence == "\x1b[34~" && MAC_SHIFTED_FUNCTION_KEYS[7].functionKey == 12);
    std::set<std::string_view> sequences;
    std::set<unsigned int> keys;
    for (const auto& k : MAC_SHIFTED_FUNCTION_KEYS)
    {
        sequences.insert(k.sequence);
        keys.insert(k.functionKey);
        CHECK_TRUE(k.functionKey >= 5 && k.functionKey <= 12);
        CHECK_TRUE(k.sequence.size() == 5 && k.sequence[0] == '\x1b' && k.sequence[1] == '[' && k.sequence[4] == '~');
    }
    CHECK_TRUE(sequences.size() == 8 && keys.size() == 8);
    // the xterm block convention: F13 .. F24 are Shift+F1 .. Shift+F12
    CHECK_TRUE(ShiftedFunctionKeyNumber(5) == 17);
    CHECK_TRUE(ShiftedFunctionKeyNumber(12) == 24);
}

static void ModifiedArrows()
{
    // what the macOS Terminal profile edit for Option+Left / Option+Right sends (Alt = modifier 3)
    CHECK_TRUE(XtermModifiedKeySequence(3, 'D') == "\x1b[1;3D");
    CHECK_TRUE(XtermModifiedKeySequence(3, 'C') == "\x1b[1;3C");
    CHECK_TRUE(XtermModifiedKeySequence(5, 'A') == "\x1b[1;5A"); // Ctrl+Up
    CHECK_TRUE(XtermModifiedKeySequence(2, 'B') == "\x1b[1;2B"); // Shift+Down
    CHECK_TRUE(PRIVATE_KEY_CODE_BASE > 0x1FF);                    // above KEY_MAX (0777) and what ncurses allocates
}

static void CtrlHOrBackspace()
{
    // macOS Terminal: Backspace sends DEL (erase = 0x7F) while xterm-256color says kbs=^H -> 0x08 is Ctrl+H
    CHECK_TRUE(!IsCtrlHBackspace(true, 0x7F));
    CHECK_TRUE(!IsCtrlHBackspace(false, 0x7F));
    // a terminal whose Backspace sends ^H (stty erase ^H)
    CHECK_TRUE(IsCtrlHBackspace(false, 0x08));
    CHECK_TRUE(IsCtrlHBackspace(true, 0x08));
    // unknown erase character: the terminal description decides
    CHECK_TRUE(IsCtrlHBackspace(true, -1));
    CHECK_TRUE(!IsCtrlHBackspace(false, -1));
}

static void Utf8Input()
{
    char32_t cp = 0;
    // what macOS Terminal sends for Option+F (f with hook) and the Romanian letters students type
    CHECK_TRUE(Utf8SequenceLength(0xC6) == 2 && Decode("\xC6\x92", cp) && cp == 0x0192);
    CHECK_TRUE(Decode("\xC8\x99", cp) && cp == 0x0219); // s with comma below
    CHECK_TRUE(Decode("\xC4\x83", cp) && cp == 0x0103); // a with breve
    CHECK_TRUE(Decode("\xE2\x82\xAC", cp) && cp == 0x20AC); // euro sign
    CHECK_TRUE(Decode("\xF0\x9F\x98\x80", cp) && cp == 0x1F600); // beyond the BMP: decoded (the caller decides)
    // not lead bytes
    CHECK_TRUE(Utf8SequenceLength('a') == 0);
    CHECK_TRUE(Utf8SequenceLength(0x80) == 0);
    CHECK_TRUE(Utf8SequenceLength(0xBF) == 0);
    CHECK_TRUE(Utf8SequenceLength(0xC0) == 0); // overlong leads
    CHECK_TRUE(Utf8SequenceLength(0xC1) == 0);
    CHECK_TRUE(Utf8SequenceLength(0xF5) == 0);
    CHECK_TRUE(Utf8SequenceLength(0xFF) == 0);
    CHECK_TRUE(Utf8SequenceLength(0xE2) == 3 && Utf8SequenceLength(0xF0) == 4);
    // malformed
    CHECK_TRUE(!Decode("\xC6\x22", cp));         // bad continuation
    CHECK_TRUE(!Decode("\xE0\x80\xAF", cp));     // overlong '/'
    CHECK_TRUE(!Decode("\xED\xA0\x80", cp));     // surrogate
    CHECK_TRUE(!Decode("\xF4\x90\x80\x80", cp)); // beyond U+10FFFF
    CHECK_TRUE(!Decode("\xE2\x82", cp));         // truncated (count does not match the lead)
    CHECK_TRUE(!DecodeUtf8(nullptr, 2, cp));
}

int main()
{
    ShiftedFunctionKeys();
    ModifiedArrows();
    CtrlHOrBackspace();
    Utf8Input();
    if (failures == 0)
        std::printf("TerminalKeys: all checks passed\n");
    return failures == 0 ? 0 : 1;
}
