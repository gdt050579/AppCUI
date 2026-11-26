#include "TerminalKeyDecoding.hpp"

namespace AppCUI::Internal::TerminalKeys
{
std::string XtermModifiedKeySequence(unsigned int modifier, char final)
{
    std::string sequence = "\x1b[1;";
    sequence += std::to_string(modifier);
    sequence.push_back(final);
    return sequence;
}

bool IsCtrlHBackspace(bool kbsIsCtrlH, int eraseCharacter)
{
    if (eraseCharacter == 0x7F)
        return false;
    if (eraseCharacter == 0x08)
        return true;
    return kbsIsCtrlH;
}

unsigned int Utf8SequenceLength(unsigned char lead)
{
    if (lead >= 0xC2 && lead <= 0xDF)
        return 2;
    if (lead >= 0xE0 && lead <= 0xEF)
        return 3;
    if (lead >= 0xF0 && lead <= 0xF4)
        return 4;
    return 0; // ASCII, a continuation byte, C0 / C1 (overlong) or F5 .. FF (beyond U+10FFFF)
}

bool DecodeUtf8(const unsigned char* bytes, size_t count, char32_t& codePoint)
{
    if (bytes == nullptr || count == 0 || Utf8SequenceLength(bytes[0]) != count)
        return false;
    char32_t cp = 0;
    switch (count)
    {
    case 2:
        cp = bytes[0] & 0x1F;
        break;
    case 3:
        cp = bytes[0] & 0x0F;
        break;
    default:
        cp = bytes[0] & 0x07;
        break;
    }
    for (size_t i = 1; i < count; i++)
    {
        if ((bytes[i] & 0xC0) != 0x80)
            return false;
        cp = (cp << 6) | (bytes[i] & 0x3F);
    }
    constexpr char32_t MIN_FOR_LENGTH[5] = { 0, 0, 0x80, 0x800, 0x10000 };
    if (cp < MIN_FOR_LENGTH[count] || cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF))
        return false;
    codePoint = cp;
    return true;
}
} // namespace AppCUI::Internal::TerminalKeys
