#pragma once

// The decisions of the ncurses frontend that do not need a terminal: which byte sequences mean which key on the
// terminals AppCUI meets, and the decoding of UTF-8 input. Pure (no ncurses, no I/O), so the cases reported by users
// can be tested on every platform (Tests/TerminalKeys).

#include <cstddef>
#include <string>
#include <string_view>

namespace AppCUI::Internal::TerminalKeys
{
// macOS Terminal sends Shift+F5 .. Shift+F12 as the VT220 codes of F15 .. F20 (and two more), which the xterm terminal
// descriptions (TERM=xterm-256color) do not define: ncurses then returns ESC, '[', ... one by one and the key reads as
// Escape followed by typed characters. Each entry: the sequence and the function key it is Shift+ of (5 .. 12).
struct ShiftedFunctionKeySequence
{
    std::string_view sequence;
    unsigned int functionKey;
};
constexpr ShiftedFunctionKeySequence MAC_SHIFTED_FUNCTION_KEYS[] = {
    { "\x1b[25~", 5 },  { "\x1b[26~", 6 },  { "\x1b[28~", 7 },  { "\x1b[29~", 8 },
    { "\x1b[31~", 9 },  { "\x1b[32~", 10 }, { "\x1b[33~", 11 }, { "\x1b[34~", 12 },
};
// the ncurses function key number (KEY_F(n)) of Shift+F<functionKey> in the xterm convention (F13 .. F24 = Shift+F1 .. F12)
constexpr int ShiftedFunctionKeyNumber(unsigned int functionKey)
{
    return static_cast<int>(12 + functionKey);
}

// The xterm form of a modified key, CSI 1;<modifier><final> ("\x1b[1;3D" = Alt+Left; modifier 2 .. 8 as in xterm:
// 2 Shift, 3 Alt, 4 Alt+Shift, 5 Ctrl, 6 Ctrl+Shift, 7 Ctrl+Alt, 8 Ctrl+Alt+Shift). The terminal descriptions of xterm-like
// terminals list them (kLFT3, ...); an older description may not, and the macOS Terminal profile edit that makes
// Option+Left / Option+Right send Alt+Left / Alt+Right relies on them: the frontend adds the missing ones.
std::string XtermModifiedKeySequence(unsigned int modifier, char final);
// key codes the frontend gives to the sequences it adds itself (far above the codes ncurses allocates)
constexpr int PRIVATE_KEY_CODE_BASE = 0x6000;

// Whether byte 0x08 is the Backspace key (otherwise it is Ctrl+H). The erase character of the terminal (stty erase, what
// the Backspace key really sends) decides: 0x7F means Backspace sends DEL, so 0x08 is Ctrl+H - the case of macOS Terminal,
// whose xterm-256color description still says kbs=^H; 0x08 means Backspace sends ^H. Any other value (unknown): trust kbs.
bool IsCtrlHBackspace(bool kbsIsCtrlH, int eraseCharacter);

// UTF-8 input: the number of bytes of a sequence that starts with `lead` (2 .. 4), or 0 when `lead` does not start a
// multi-byte sequence (ASCII, a continuation byte, an overlong or out-of-range lead).
unsigned int Utf8SequenceLength(unsigned char lead);
// The code point of a complete sequence (as many bytes as Utf8SequenceLength says); false for a malformed sequence
// (bad continuation byte, overlong form, surrogate, beyond U+10FFFF).
bool DecodeUtf8(const unsigned char* bytes, size_t count, char32_t& codePoint);
} // namespace AppCUI::Internal::TerminalKeys
