#include "ControlContext.hpp"
#include "Internal.hpp"
#include <cstring>
#include <string.h>

namespace AppCUI
{
#define CTRLC ((ControlContext*) Context)

constexpr uint8 CHAR_TYPE_EOS       = 0;
constexpr uint8 CHAR_TYPE_OTHER     = 1;
constexpr uint8 CHAR_TYPE_WORD      = 2;
constexpr uint8 CHAR_TYPE_NUMBER    = 3;
constexpr uint8 CHAR_TYPE_SPACE     = 4;
constexpr uint8 CHAR_TYPE_EQ        = 5;
constexpr uint8 CHAR_TYPE_SEPARATOR = 6;
constexpr uint8 CHAR_TYPE_POINT     = 7;
constexpr uint8 CHAR_TYPE_MINUS     = 8;

//=========================================
// THIS CODE WAS AUTOMATICALLY GENERATED !
//=========================================

constexpr uint8 LAYOUT_KEY_NONE     = 0;
constexpr uint16 LAYOUT_KEY_X       = 1;
constexpr uint16 LAYOUT_FLAG_X      = 0x0001;
constexpr uint16 LAYOUT_KEY_Y       = 2;
constexpr uint16 LAYOUT_FLAG_Y      = 0x0002;
constexpr uint16 LAYOUT_KEY_LEFT    = 3;
constexpr uint16 LAYOUT_FLAG_LEFT   = 0x0004;
constexpr uint16 LAYOUT_KEY_RIGHT   = 4;
constexpr uint16 LAYOUT_FLAG_RIGHT  = 0x0008;
constexpr uint16 LAYOUT_KEY_TOP     = 5;
constexpr uint16 LAYOUT_FLAG_TOP    = 0x0010;
constexpr uint16 LAYOUT_KEY_BOTTOM  = 6;
constexpr uint16 LAYOUT_FLAG_BOTTOM = 0x0020;
constexpr uint16 LAYOUT_KEY_WIDTH   = 7;
constexpr uint16 LAYOUT_FLAG_WIDTH  = 0x0040;
constexpr uint16 LAYOUT_KEY_HEIGHT  = 8;
constexpr uint16 LAYOUT_FLAG_HEIGHT = 0x0080;
constexpr uint16 LAYOUT_KEY_ALIGN   = 9;
constexpr uint16 LAYOUT_FLAG_ALIGN  = 0x0100;
constexpr uint16 LAYOUT_KEY_DOCK    = 10;
constexpr uint16 LAYOUT_FLAG_DOCK   = 0x0200;

constexpr uint8 _layout_translate_map_[116] = {
    LAYOUT_KEY_NONE,  LAYOUT_KEY_ALIGN, LAYOUT_KEY_BOTTOM, LAYOUT_KEY_NONE,   LAYOUT_KEY_DOCK,  LAYOUT_KEY_NONE,
    LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,  LAYOUT_KEY_HEIGHT, LAYOUT_KEY_NONE,   LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,
    LAYOUT_KEY_LEFT,  LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,   LAYOUT_KEY_NONE,   LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,
    LAYOUT_KEY_RIGHT, LAYOUT_KEY_NONE,  LAYOUT_KEY_TOP,    LAYOUT_KEY_NONE,   LAYOUT_KEY_NONE,  LAYOUT_KEY_WIDTH,
    LAYOUT_KEY_X,     LAYOUT_KEY_Y,     LAYOUT_KEY_NONE,   LAYOUT_KEY_NONE,   LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,
    LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,   LAYOUT_KEY_NONE,   LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,
    LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,   LAYOUT_KEY_NONE,   LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,
    LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,   LAYOUT_KEY_DOCK,   LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,
    LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,   LAYOUT_KEY_NONE,   LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,
    LAYOUT_KEY_NONE,  LAYOUT_KEY_LEFT,  LAYOUT_KEY_NONE,   LAYOUT_KEY_TOP,    LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,
    LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,   LAYOUT_KEY_ALIGN,  LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,
    LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,   LAYOUT_KEY_NONE,   LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,
    LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,   LAYOUT_KEY_NONE,   LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,
    LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,   LAYOUT_KEY_NONE,   LAYOUT_KEY_RIGHT, LAYOUT_KEY_NONE,
    LAYOUT_KEY_WIDTH, LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,   LAYOUT_KEY_HEIGHT, LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,
    LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,   LAYOUT_KEY_NONE,   LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,
    LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,   LAYOUT_KEY_NONE,   LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,
    LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,   LAYOUT_KEY_NONE,   LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,
    LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,   LAYOUT_KEY_NONE,   LAYOUT_KEY_NONE,  LAYOUT_KEY_NONE,
    LAYOUT_KEY_NONE,  LAYOUT_KEY_BOTTOM
};

inline uint8 HashToLayoutKey(uint32 hash)
{
    if (hash >= 116)
        return LAYOUT_KEY_NONE;
    return _layout_translate_map_[hash];
};

constexpr uint8 _align_translate_map_[258] = { 0xFF,
                                               0xFF,
                                               (uint8) Alignament::Bottom,
                                               (uint8) Alignament::Center,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               (uint8) Alignament::Left,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               (uint8) Alignament::BottomLeft,
                                               0xFF,
                                               (uint8) Alignament::Right,
                                               0xFF,
                                               (uint8) Alignament::Top,
                                               0xFF,
                                               (uint8) Alignament::BottomRight,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               (uint8) Alignament::TopLeft,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               (uint8) Alignament::TopRight,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               (uint8) Alignament::Left,
                                               0xFF,
                                               (uint8) Alignament::Top,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               (uint8) Alignament::Right,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               (uint8) Alignament::Center,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               (uint8) Alignament::Bottom,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               (uint8) Alignament::TopLeft,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               (uint8) Alignament::TopRight,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               (uint8) Alignament::BottomLeft,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               0xFF,
                                               (uint8) Alignament::BottomRight };

inline bool HashToAlignament(uint32 hash, Alignament& align)
{
    if (hash >= 258)
        return false;
    auto ch = _align_translate_map_[hash];
    if (ch == 0xFF)
        return false;
    align = static_cast<Alignament>(ch);
    return true;
};

//=========================================
// END OF AUTOMATICALLY GENERATED CODE
//=========================================

// for gcc, building a field should look like var.field, not var.##field
// http://gcc.gnu.org/onlinedocs/cpp/Concatenation.html
#define SET_LAYOUT_INFO(flag, field)                                                                                   \
    {                                                                                                                  \
        inf.flags |= flag;                                                                                             \
        inf.field.Value = value;                                                                                       \
        inf.field.Type  = valueType;                                                                                   \
    }

#define TRANSLATE_VALUE(result, from, size, flag)                                                                      \
    if (this->Layout.Format.PercentageMask & flag)                                                                     \
        result = from * size / 10000;                                                                                  \
    else                                                                                                               \
        result = from;

struct LayoutKeyValueData
{
    const char* HashName;
    uint32 Hash;
    int n1, n2;
    uint32 StringValueHash;
    bool IsNegative;
    char ValueType;
};

uint8 __char_types__[256] = {
    CHAR_TYPE_EOS,    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_SPACE,  CHAR_TYPE_SPACE,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_SPACE,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_SPACE,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_SEPARATOR, CHAR_TYPE_MINUS,  CHAR_TYPE_POINT,  CHAR_TYPE_OTHER,
    CHAR_TYPE_NUMBER, CHAR_TYPE_NUMBER, CHAR_TYPE_NUMBER,    CHAR_TYPE_NUMBER, CHAR_TYPE_NUMBER, CHAR_TYPE_NUMBER,
    CHAR_TYPE_NUMBER, CHAR_TYPE_NUMBER, CHAR_TYPE_NUMBER,    CHAR_TYPE_NUMBER, CHAR_TYPE_EQ,     CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_EQ,     CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_WORD,
    CHAR_TYPE_WORD,   CHAR_TYPE_WORD,   CHAR_TYPE_WORD,      CHAR_TYPE_WORD,   CHAR_TYPE_WORD,   CHAR_TYPE_WORD,
    CHAR_TYPE_WORD,   CHAR_TYPE_WORD,   CHAR_TYPE_WORD,      CHAR_TYPE_WORD,   CHAR_TYPE_WORD,   CHAR_TYPE_WORD,
    CHAR_TYPE_WORD,   CHAR_TYPE_WORD,   CHAR_TYPE_WORD,      CHAR_TYPE_WORD,   CHAR_TYPE_WORD,   CHAR_TYPE_WORD,
    CHAR_TYPE_WORD,   CHAR_TYPE_WORD,   CHAR_TYPE_WORD,      CHAR_TYPE_WORD,   CHAR_TYPE_WORD,   CHAR_TYPE_WORD,
    CHAR_TYPE_WORD,   CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_WORD,   CHAR_TYPE_WORD,      CHAR_TYPE_WORD,   CHAR_TYPE_WORD,   CHAR_TYPE_WORD,
    CHAR_TYPE_WORD,   CHAR_TYPE_WORD,   CHAR_TYPE_WORD,      CHAR_TYPE_WORD,   CHAR_TYPE_WORD,   CHAR_TYPE_WORD,
    CHAR_TYPE_WORD,   CHAR_TYPE_WORD,   CHAR_TYPE_WORD,      CHAR_TYPE_WORD,   CHAR_TYPE_WORD,   CHAR_TYPE_WORD,
    CHAR_TYPE_WORD,   CHAR_TYPE_WORD,   CHAR_TYPE_WORD,      CHAR_TYPE_WORD,   CHAR_TYPE_WORD,   CHAR_TYPE_WORD,
    CHAR_TYPE_WORD,   CHAR_TYPE_WORD,   CHAR_TYPE_WORD,      CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,
    CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,  CHAR_TYPE_OTHER,     CHAR_TYPE_OTHER
};

// <xxx> (Arrow left, 3 character, Arrow right)
constexpr uint32 MINIM_SCORLL_BAR_LENGTH = 5;

bool ProcessLayoutKeyValueData(LayoutKeyValueData& l, LayoutInformation& inf, Application::Config*)
{
    int value = 0;
    LayoutValueType valueType;
    switch (l.ValueType)
    {
    case 0:
        // characters
        CHECK(l.n2 == 0, false, "Character offset values can not be floating values !");
        CHECK(l.n1 <= 30000, false, "A character offset must be smaller than 30000 (current value is: %d)", l.n1);
        CHECK(l.n1 >= -30000, false, "A character offset must be bigger than -30000 (current value is: %d)", l.n1);
        value     = l.n1;
        valueType = LayoutValueType::CharacterOffset;
        break;
    case '%':
        // percentage
        CHECK(l.n1 <= 300, false, "A percentage must be smaller than 300% (current value is: %d)", l.n1);
        CHECK(l.n1 >= -300, false, "A percentage offset must be bigger than -300% (current value is: %d)", l.n1);
        value     = l.n1 * 100 + l.n2;
        valueType = LayoutValueType::Percentage;
        break;
    default:
        RETURNERROR(false, "Invalid value format: %d (%c)", l.ValueType, l.ValueType);
    }
    if (l.IsNegative)
        value = -value;
    uint8 layoutKey = HashToLayoutKey(l.Hash);
    switch (layoutKey)
    {
    case LAYOUT_KEY_X:
        SET_LAYOUT_INFO(LAYOUT_FLAG_X, x);
        break;
    case LAYOUT_KEY_Y:
        SET_LAYOUT_INFO(LAYOUT_FLAG_Y, y);
        break;
    case LAYOUT_KEY_LEFT:
        SET_LAYOUT_INFO(LAYOUT_FLAG_LEFT, a_left);
        break;
    case LAYOUT_KEY_RIGHT:
        SET_LAYOUT_INFO(LAYOUT_FLAG_RIGHT, a_right);
        break;
    case LAYOUT_KEY_TOP:
        SET_LAYOUT_INFO(LAYOUT_FLAG_TOP, a_top);
        break;
    case LAYOUT_KEY_BOTTOM:
        SET_LAYOUT_INFO(LAYOUT_FLAG_BOTTOM, a_bottom);
        break;
    case LAYOUT_KEY_WIDTH:
        SET_LAYOUT_INFO(LAYOUT_FLAG_WIDTH, width);
        break;
    case LAYOUT_KEY_HEIGHT:
        SET_LAYOUT_INFO(LAYOUT_FLAG_HEIGHT, height);
        break;
    case LAYOUT_KEY_ALIGN:
        CHECK(HashToAlignament(l.StringValueHash, inf.align), false, "Fail to compute align value !");
        inf.flags |= LAYOUT_FLAG_ALIGN;
        break;
    case LAYOUT_KEY_DOCK:
        CHECK(HashToAlignament(l.StringValueHash, inf.dock), false, "Fail to compute dock value !");
        inf.flags |= LAYOUT_FLAG_DOCK;
        break;
    default:
        RETURNERROR(false, "Unknown key (hash:%08x) ==> %s", l.Hash, l.HashName);
    }
    return true;
}
inline const uint8* SkipSpaces(const uint8* start, const uint8* end)
{
    while ((start < end) && (__char_types__[*start] == CHAR_TYPE_SPACE))
        start++;
    return start;
}
inline const uint8* ComputeValueHash(const uint8* s, const uint8* e, uint32& hashValue)
{
    hashValue    = 0;
    uint32 index = 0;
    while ((s < e) && (__char_types__[*s] == CHAR_TYPE_WORD))
    {
        // hashValue = ((hashValue) << 2) ^ ((uint32) (('Z' + 1) - (((*start) & ((uint8) (~0x20))))));
        // hashValue += (uint32) ((uint32) (('Z' + 1) - (((*s) & ((uint8) (~0x20))))));
        hashValue += ((*s) & ((uint8) (~0x20))) - ((uint8) 'A') + 1;
        hashValue += index;
        s++;
        index += 2;
    }
    return s;
}
bool AnalyzeLayout(string_view layout, LayoutInformation& inf, Application::Config* Cfg)
{
    // format: key:value,[key:value],....
    const uint8* p     = (const uint8*) layout.data();
    const uint8* p_end = p + layout.size();
    CHECK(p, false, "Expecting a valid (non-null) layout string !");

    LayoutKeyValueData lkv;
    inf.a_left   = { 0, LayoutValueType::CharacterOffset };
    inf.a_bottom = { 0, LayoutValueType::CharacterOffset };
    inf.a_top    = { 0, LayoutValueType::CharacterOffset };
    inf.a_right  = { 0, LayoutValueType::CharacterOffset };
    inf.x        = { 0, LayoutValueType::CharacterOffset };
    inf.y        = { 0, LayoutValueType::CharacterOffset };
    inf.width    = { 1, LayoutValueType::CharacterOffset };
    inf.height   = { 1, LayoutValueType::CharacterOffset };
    inf.align    = Alignament::TopLeft;
    inf.dock     = Alignament::TopLeft;
    inf.flags    = 0;

    int cnt;

    p = SkipSpaces(p, p_end); // skip initial spaces
    while (p < p_end)
    {
        // compute value name hash
        lkv.HashName = (const char*) p;
        p            = ComputeValueHash(p, p_end, lkv.Hash);
        CHECK(lkv.Hash, false, "Invalid hash (expecting a valid key: %s)", p);
        p = SkipSpaces(p, p_end);
        CHECK(p < p_end, false, "Premature end of layout string --> expecting a ':' or '=' after key");
        CHECK(__char_types__[*p] == CHAR_TYPE_EQ, false, "Expecting ':' or '=' character (%s)", p);
        p++;
        p = SkipSpaces(p, p_end);
        CHECK(p < p_end, false, "Premature end of layout string --> expecting a value after ':' or '=' delimiter");

        // extract value
        lkv.n1 = lkv.n2     = 0;
        lkv.StringValueHash = 0;
        lkv.IsNegative      = false;
        lkv.ValueType       = 0;
        if ((*p) == '-')
        {
            lkv.IsNegative = true;
            p++;
            CHECK(p < p_end,
                  false,
                  "Premature end of layout string --> expecting a value after '-' (minus) declatartor");
        }
        if ((!lkv.IsNegative) && (__char_types__[*p] == CHAR_TYPE_WORD))
        {
            p = ComputeValueHash(p, p_end, lkv.StringValueHash);
            CHECK(lkv.StringValueHash, false, "Invalid value hash (expecting a valid key: %s)", p);
        }
        else
        {
            while ((p < p_end) && (__char_types__[*p] == CHAR_TYPE_NUMBER))
            {
                lkv.n1 = lkv.n1 * 10 + ((*p) - '0');
                p++;
            }
            if ((p < p_end) && ((*p) == '.'))
            {
                p++;
                cnt = 0;
                while ((p < p_end) && (__char_types__[*p] == CHAR_TYPE_NUMBER))
                {
                    if (cnt < 2)
                    {
                        lkv.n2 = lkv.n2 * 10 + ((*p) - '0');
                        cnt++;
                    }
                    p++;
                }
                if (cnt == 1)
                    lkv.n2 *= 10;
            }
        }
        p = SkipSpaces(p, p_end);
        if ((p < p_end) && (__char_types__[*p] != CHAR_TYPE_SEPARATOR))
        {
            lkv.ValueType = *p;
            while ((p < p_end) && (__char_types__[*p] != CHAR_TYPE_SEPARATOR))
                p++;
        }
        if ((p < p_end) && (__char_types__[*p] == CHAR_TYPE_SEPARATOR))
            p++;
        p = SkipSpaces(p, p_end);
        CHECK(ProcessLayoutKeyValueData(lkv, inf, Cfg), false, "Invalid layout params !");
    }
    return true;
}

ControlContext::ControlContext()
{
    this->Controls            = nullptr;
    this->ControlsCount       = 0;
    this->CurrentControlIndex = -1;
    this->Margins.Left = this->Margins.Right = this->Margins.Top = this->Margins.Bottom = 0;
    this->Parent                                                                        = nullptr;
    this->GroupID                                                                       = 0;
    this->HotKey                                                                        = Key::None;
    this->Layout.X = this->Layout.Y = this->Layout.Width = this->Layout.Height = 0;
    this->Layout.MinWidth = this->Layout.MinHeight = 0;
    this->Layout.MaxWidth = this->Layout.MaxHeight = 2000000000;
    this->ScreenClip.Visible                       = false;
    this->Inited                                   = false;
    this->ControlID                                = 0;
    this->Focused                                  = false;
    this->MouseIsOver                              = false;
    this->Cfg                                      = Application::GetAppConfig();
    this->HotKeyOffset                             = CharacterBuffer::INVALID_HOTKEY_OFFSET;
    this->ScrollBars.LeftMargin                    = 2;
    this->ScrollBars.TopMargin                     = 2;
    this->ScrollBars.VerticalValue                 = 0;
    this->ScrollBars.HorizontalValue               = 0;
    this->ScrollBars.MaxHorizontalValue            = 0;
    this->ScrollBars.MaxVerticalValue              = 0;
    this->ScrollBars.OutsideControl                = false;
    this->handlers                                 = nullptr;
}
bool ControlContext::ProcessDockedLayout(LayoutInformation& inf)
{
    // if dock is set --> X,Y, Left, Right, Top and Bottom should not be set
    CHECK((inf.flags & (LAYOUT_FLAG_LEFT | LAYOUT_FLAG_RIGHT | LAYOUT_FLAG_TOP | LAYOUT_FLAG_BOTTOM | LAYOUT_FLAG_X |
                        LAYOUT_FLAG_Y)) == 0,
          false,
          "When dock|d parameter is used, none of the position (x,y) or anchor (left,right,bottom,top) parameters can "
          "not be uesd");
    // similar - align can not be used
    CHECK((inf.flags & LAYOUT_FLAG_ALIGN) == 0,
          false,
          "When dock|d parameter is used, 'align' parameter can not be used !");
    // if width is not set --> default it to 100%
    if ((inf.flags & LAYOUT_FLAG_WIDTH) == 0)
        inf.width = { 10000, LayoutValueType::Percentage };
    // if height is not set --> default it to 100%
    if ((inf.flags & LAYOUT_FLAG_HEIGHT) == 0)
        inf.height = { 10000, LayoutValueType::Percentage };

    // set the layout (only width and height) will be copied (rest are 0)
    this->Layout.Format.LayoutMode = LayoutFormatMode::PointAndSize;
    this->Layout.Format.Width      = inf.width;
    this->Layout.Format.Height     = inf.height;
    this->Layout.Format.Align      = inf.dock;
    this->Layout.Format.Anchor     = inf.dock;
    this->Layout.Format.X          = { 0, LayoutValueType::CharacterOffset };
    this->Layout.Format.Y          = { 0, LayoutValueType::CharacterOffset };

    // all good
    return true;
}
bool ControlContext::ProcessXYWHLayout(LayoutInformation& inf)
{
    // it is assume that DOCK|D is not set (as it was process early in ProcessDockedLayout)
    // if X and Y are set --> Left, Right, Top and Bottom should not be set
    CHECK((inf.flags & (LAYOUT_FLAG_LEFT | LAYOUT_FLAG_RIGHT | LAYOUT_FLAG_TOP | LAYOUT_FLAG_BOTTOM)) == 0,
          false,
          "When (x,y) parameters are used, none of the anchor (left,right,bottom,top) parameters can not be used");
    // if align is not set --> default it to TopLeft
    if ((inf.flags & LAYOUT_FLAG_ALIGN) == 0)
        inf.align = Alignament::TopLeft;
    // if width is not present --> default it to 1 (character)
    if ((inf.flags & LAYOUT_FLAG_WIDTH) == 0)
        inf.width = { 1, LayoutValueType::CharacterOffset };
    // if height is not present --> default it to 1 (character)
    if ((inf.flags & LAYOUT_FLAG_HEIGHT) == 0)
        inf.height = { 1, LayoutValueType::CharacterOffset };

    this->Layout.Format.LayoutMode = LayoutFormatMode::PointAndSize;
    this->Layout.Format.X          = inf.x;
    this->Layout.Format.Y          = inf.y;
    this->Layout.Format.Width      = inf.width;
    this->Layout.Format.Height     = inf.height;
    this->Layout.Format.Align      = inf.align;
    this->Layout.Format.Anchor     = Alignament::TopLeft; // for (X,Y) anchor is always TopLeft

    return true;
}
bool ControlContext::ProcessCornerAnchorLayout(LayoutInformation& inf, Alignament anchor)
{
    CHECK((inf.flags & (LAYOUT_FLAG_X | LAYOUT_FLAG_Y)) == 0,
          false,
          "When a corner anchor is being use (top,left,righ,bottom) , (X,Y) coordonates can not be used");
    // if align is not set --> default it to TopLeft
    if ((inf.flags & LAYOUT_FLAG_ALIGN) == 0)
        inf.align = Alignament::TopLeft;
    // if width is not present --> default it to 1 (character)
    if ((inf.flags & LAYOUT_FLAG_WIDTH) == 0)
        inf.width = { 1, LayoutValueType::CharacterOffset };
    // if height is not present --> default it to 1 (character)
    if ((inf.flags & LAYOUT_FLAG_HEIGHT) == 0)
        inf.height = { 1, LayoutValueType::CharacterOffset };
    this->Layout.Format.LayoutMode = LayoutFormatMode::PointAndSize;
    this->Layout.Format.Width      = inf.width;
    this->Layout.Format.Height     = inf.height;
    this->Layout.Format.Align      = anchor;
    this->Layout.Format.Anchor     = anchor;
    // copy anchor to (X,Y)
    switch (anchor)
    {
    case Alignament::TopLeft:
        this->Layout.Format.X = inf.a_left;
        this->Layout.Format.Y = inf.a_top;
        break;
    case Alignament::TopRight:
        this->Layout.Format.X = inf.a_right;
        this->Layout.Format.Y = inf.a_top;
        break;
    case Alignament::BottomRight:
        this->Layout.Format.X = inf.a_right;
        this->Layout.Format.Y = inf.a_bottom;
        break;
    case Alignament::BottomLeft:
        this->Layout.Format.X = inf.a_left;
        this->Layout.Format.Y = inf.a_bottom;
        break;
    default:
        RETURNERROR(false, "Invalid anchor value(%d) for ProcessCornerAnchorLayout (this is an internal error) !");
    }

    return true;
}
bool ControlContext::ProcessHorizontalParalelAnchors(LayoutInformation& inf)
{
    // horizontal (left-right) are provided
    CHECK((inf.flags & LAYOUT_FLAG_X) == 0,
          false,
          "When (left,right) parameters are used toghere, 'X' parameter can not be used");
    CHECK((inf.flags & LAYOUT_FLAG_WIDTH) == 0,
          false,
          "When (left,right) parameters are used toghere, width can not be used as it is deduced from left-right "
          "difference");

    // if "align" is not provided, it is defaulted to center
    if ((inf.flags & LAYOUT_FLAG_ALIGN) == 0)
        inf.align = Alignament::Center;
    else
    {
        // check layout
        CHECK((inf.align == Alignament::Top) || (inf.align == Alignament::Center) || (inf.align == Alignament::Bottom),
              false,
              "When (left,right) are provided, only Top(t), Center(c) and Bottom(b) alignament values are allowed !");
    }

    // if "height" is not provided, it is defaulted to 1
    if ((inf.flags & LAYOUT_FLAG_HEIGHT) == 0)
        this->Layout.Format.Height = { 1, LayoutValueType::CharacterOffset };
    else
        this->Layout.Format.Height = inf.height;

    // if "Y" is not provided, it is defaulted to 0
    if ((inf.flags & LAYOUT_FLAG_Y) == 0)
        this->Layout.Format.Y = { 0, LayoutValueType::CharacterOffset };
    else
        this->Layout.Format.Y = inf.y;

    // construct de layout
    this->Layout.Format.LayoutMode  = LayoutFormatMode::LeftRightAnchorsAndHeight;
    this->Layout.Format.AnchorLeft  = inf.a_left;
    this->Layout.Format.AnchorRight = inf.a_right;
    this->Layout.Format.Align       = inf.align;

    // all good
    return true;
}
bool ControlContext::ProcessVerticalParalelAnchors(LayoutInformation& inf)
{
    // vertical (up-down) are provided
    CHECK((inf.flags & LAYOUT_FLAG_Y) == 0,
          false,
          "When (top,down) parameters are used toghere, 'Y' parameter can not be used");
    CHECK((inf.flags & LAYOUT_FLAG_HEIGHT) == 0,
          false,
          "When (top,down) parameters are used toghere, height can not be used as it is deduced from bottom-top "
          "difference");

    // if "align" is not provided, it is defaulted to center
    if ((inf.flags & LAYOUT_FLAG_ALIGN) == 0)
        inf.align = Alignament::Center;
    else
    {
        // check layout
        CHECK((inf.align == Alignament::Left) || (inf.align == Alignament::Center) || (inf.align == Alignament::Right),
              false,
              "When (top,down) are provided, only Left(l), Center(c) and Right(r) alignament values are allowed !");
    }

    // if "width" is not provided, it is defaulted to 1
    if ((inf.flags & LAYOUT_FLAG_WIDTH) == 0)
        this->Layout.Format.Width = { 1, LayoutValueType::CharacterOffset };
    else
        this->Layout.Format.Width = inf.width;

    // if "X" is not provided, it is defaulted to 0
    if ((inf.flags & LAYOUT_FLAG_X) == 0)
        this->Layout.Format.X = { 0, LayoutValueType::CharacterOffset };
    else
        this->Layout.Format.X = inf.x;

    // construct de layout
    this->Layout.Format.LayoutMode   = LayoutFormatMode::TopBottomAnchorsAndWidth;
    this->Layout.Format.AnchorTop    = inf.a_top;
    this->Layout.Format.AnchorBottom = inf.a_bottom;
    this->Layout.Format.Align        = inf.align;

    // all good
    return true;
}
bool ControlContext::ProcessLTRAnchors(LayoutInformation& inf)
{
    CHECK((inf.flags & (LAYOUT_FLAG_X | LAYOUT_FLAG_Y | LAYOUT_FLAG_ALIGN | LAYOUT_FLAG_WIDTH)) == 0,
          false,
          "When (left,top,right) parameters are used together, 'X', 'Y' and 'A' parameters can not be used");

    if (!(inf.flags & LAYOUT_FLAG_HEIGHT))
        inf.height = { 1, LayoutValueType::CharacterOffset };

    this->Layout.Format.LayoutMode  = LayoutFormatMode::LeftTopRightAnchorsAndHeight;
    this->Layout.Format.AnchorLeft  = inf.a_left;
    this->Layout.Format.AnchorTop   = inf.a_top;
    this->Layout.Format.AnchorRight = inf.a_right;
    this->Layout.Format.Height      = inf.height;

    return true;
}
bool ControlContext::ProcessLBRAnchors(LayoutInformation& inf)
{
    CHECK((inf.flags & (LAYOUT_FLAG_X | LAYOUT_FLAG_Y | LAYOUT_FLAG_ALIGN | LAYOUT_FLAG_WIDTH)) == 0,
          false,
          "When (left,bottom,right) parameters are used together, 'X', 'Y' and 'A' parameters can not be used");

    if (!(inf.flags & LAYOUT_FLAG_HEIGHT))
        inf.height = { 1, LayoutValueType::CharacterOffset };

    this->Layout.Format.LayoutMode   = LayoutFormatMode::LeftBottomRightAnchorsAndHeight;
    this->Layout.Format.AnchorLeft   = inf.a_left;
    this->Layout.Format.AnchorBottom = inf.a_bottom;
    this->Layout.Format.AnchorRight  = inf.a_right;
    this->Layout.Format.Height       = inf.height;

    return true;
}
bool ControlContext::ProcessTLBAnchors(LayoutInformation& inf)
{
    CHECK((inf.flags & (LAYOUT_FLAG_X | LAYOUT_FLAG_Y | LAYOUT_FLAG_ALIGN | LAYOUT_FLAG_HEIGHT)) == 0,
          false,
          "When (top,left,bottom) parameters are used together, 'X', 'Y' and 'A' parameters can not be used");

    if (!(inf.flags & LAYOUT_FLAG_WIDTH))
        inf.width = { 1, LayoutValueType::CharacterOffset };

    this->Layout.Format.LayoutMode   = LayoutFormatMode::TopLeftBottomAnchorsAndWidth;
    this->Layout.Format.AnchorTop    = inf.a_top;
    this->Layout.Format.AnchorLeft   = inf.a_left;
    this->Layout.Format.AnchorBottom = inf.a_bottom;
    this->Layout.Format.Width        = inf.width;

    return true;
}
bool ControlContext::ProcessTRBAnchors(LayoutInformation& inf)
{
    CHECK((inf.flags & (LAYOUT_FLAG_X | LAYOUT_FLAG_Y | LAYOUT_FLAG_ALIGN | LAYOUT_FLAG_HEIGHT)) == 0,
          false,
          "When (top,right,bottom) parameters are used together, 'X', 'Y' and 'H' parameters can not be used");

    if (!(inf.flags & LAYOUT_FLAG_WIDTH))
        inf.width = { 1, LayoutValueType::CharacterOffset };

    this->Layout.Format.LayoutMode   = LayoutFormatMode::TopRightBottomAnchorsAndWidth;
    this->Layout.Format.AnchorTop    = inf.a_top;
    this->Layout.Format.AnchorRight  = inf.a_right;
    this->Layout.Format.AnchorBottom = inf.a_bottom;
    this->Layout.Format.Width        = inf.width;

    return true;
}
bool ControlContext::ProcessLTRBAnchors(LayoutInformation& inf)
{
    CHECK((inf.flags & (LAYOUT_FLAG_X | LAYOUT_FLAG_Y | LAYOUT_FLAG_ALIGN | LAYOUT_FLAG_HEIGHT | LAYOUT_FLAG_WIDTH)) ==
                0,
          false,
          "When (left,top,right,bottom) parameters are used together, 'X', 'Y', 'A', 'W' and 'H' parameters can not be "
          "used");

    this->Layout.Format.LayoutMode   = LayoutFormatMode::LeftTopRightBottomAnchors;
    this->Layout.Format.AnchorLeft   = inf.a_left;
    this->Layout.Format.AnchorTop    = inf.a_top;
    this->Layout.Format.AnchorRight  = inf.a_right;
    this->Layout.Format.AnchorBottom = inf.a_bottom;

    return true;
}
bool ControlContext::UpdateLayoutFormat(string_view format)
{
    LayoutInformation inf;
    CHECK(AnalyzeLayout(format, inf, this->Cfg), false, "Fail to load format data !");

    // check if layout params are OK
    // Step 1 ==> if dock option is present
    if (inf.flags & LAYOUT_FLAG_DOCK)
        return ProcessDockedLayout(inf);
    // Step 2 ==> check (X,Y) + (W,H) + (optional align)
    if ((inf.flags & (LAYOUT_FLAG_X | LAYOUT_FLAG_Y)) == (LAYOUT_FLAG_X | LAYOUT_FLAG_Y))
        return ProcessXYWHLayout(inf);

    // Step 3 ==> check different types of anchors
    auto anchorFlags = inf.flags & (LAYOUT_FLAG_LEFT | LAYOUT_FLAG_RIGHT | LAYOUT_FLAG_TOP | LAYOUT_FLAG_BOTTOM);
    switch (anchorFlags)
    {
    case LAYOUT_FLAG_LEFT | LAYOUT_FLAG_TOP:
        return ProcessCornerAnchorLayout(inf, Alignament::TopLeft);
    case LAYOUT_FLAG_RIGHT | LAYOUT_FLAG_TOP:
        return ProcessCornerAnchorLayout(inf, Alignament::TopRight);
    case LAYOUT_FLAG_RIGHT | LAYOUT_FLAG_BOTTOM:
        return ProcessCornerAnchorLayout(inf, Alignament::BottomRight);
    case LAYOUT_FLAG_LEFT | LAYOUT_FLAG_BOTTOM:
        return ProcessCornerAnchorLayout(inf, Alignament::BottomLeft);
    case LAYOUT_FLAG_RIGHT | LAYOUT_FLAG_LEFT:
        return ProcessHorizontalParalelAnchors(inf);
    case LAYOUT_FLAG_TOP | LAYOUT_FLAG_BOTTOM:
        return ProcessVerticalParalelAnchors(inf);
    case LAYOUT_FLAG_LEFT | LAYOUT_FLAG_TOP | LAYOUT_FLAG_RIGHT:
        return ProcessLTRAnchors(inf);
    case LAYOUT_FLAG_LEFT | LAYOUT_FLAG_BOTTOM | LAYOUT_FLAG_RIGHT:
        return ProcessLBRAnchors(inf);
    case LAYOUT_FLAG_TOP | LAYOUT_FLAG_LEFT | LAYOUT_FLAG_BOTTOM:
        return ProcessTLBAnchors(inf);
    case LAYOUT_FLAG_TOP | LAYOUT_FLAG_RIGHT | LAYOUT_FLAG_BOTTOM:
        return ProcessTRBAnchors(inf);
    case LAYOUT_FLAG_LEFT | LAYOUT_FLAG_TOP | LAYOUT_FLAG_RIGHT | LAYOUT_FLAG_BOTTOM:
        return ProcessLTRBAnchors(inf);
    }

    RETURNERROR(false, "Invalid keys combination: %08X", inf.flags);
}
void ControlContext::SetControlSize(uint32 width, uint32 heigh)
{
    this->Layout.Width  = (int) width;
    this->Layout.Height = (int) heigh;
    // check Width / Height values agains min...max values
    this->Layout.Width  = std::max<>(this->Layout.Width, this->Layout.MinWidth);
    this->Layout.Width  = std::min<>(this->Layout.Width, this->Layout.MaxWidth);
    this->Layout.Height = std::max<>(this->Layout.Height, this->Layout.MinHeight);
    this->Layout.Height = std::min<>(this->Layout.Height, this->Layout.MaxHeight);
}
bool ControlContext::RecomputeLayout_PointAndSize(const LayoutMetricData& md)
{
    SetControlSize(md.Width, md.Height);

    // compute (x,y) based on anchor
    switch (md.Anchor)
    {
    case Alignament::TopLeft:
        this->Layout.X = md.X;
        this->Layout.Y = md.Y;
        break;
    case Alignament::Top:
        this->Layout.X = md.ParentWidth / 2;
        this->Layout.Y = md.Y;
        break;
    case Alignament::TopRight:
        this->Layout.X = md.ParentWidth - md.X;
        this->Layout.Y = md.Y;
        break;
    case Alignament::Right:
        this->Layout.X = md.ParentWidth - md.X;
        this->Layout.Y = md.ParentHeigh / 2;
        break;
    case Alignament::BottomRight:
        this->Layout.X = md.ParentWidth - md.X;
        this->Layout.Y = md.ParentHeigh - md.Y;
        break;
    case Alignament::Bottom:
        this->Layout.X = md.ParentWidth / 2;
        this->Layout.Y = md.ParentHeigh - md.Y;
        break;
    case Alignament::BottomLeft:
        this->Layout.X = md.X;
        this->Layout.Y = md.ParentHeigh - md.Y;
        break;
    case Alignament::Left:
        this->Layout.X = md.X;
        this->Layout.Y = md.ParentHeigh / 2;
        break;
    case Alignament::Center:
        this->Layout.X = md.ParentWidth / 2;
        this->Layout.Y = md.ParentHeigh / 2;
        break;
    default:
        RETURNERROR(false, "Invalid anchor value: %d", md.Anchor);
    };
    // align (x,y) from the current position based on Width/Height
    switch (md.Align)
    {
    case Alignament::TopLeft:
        // do nothing
        break;
    case Alignament::Top:
        this->Layout.X -= this->Layout.Width / 2;
        break;
    case Alignament::TopRight:
        this->Layout.X -= this->Layout.Width;
        break;
    case Alignament::Right:
        this->Layout.X -= this->Layout.Width;
        this->Layout.Y -= this->Layout.Height / 2;
        break;
    case Alignament::BottomRight:
        this->Layout.X -= this->Layout.Width;
        this->Layout.Y -= this->Layout.Height;
        break;
    case Alignament::Bottom:
        this->Layout.X -= this->Layout.Width / 2;
        this->Layout.Y -= this->Layout.Height;
        break;
    case Alignament::BottomLeft:
        this->Layout.Y -= this->Layout.Height;
        break;
    case Alignament::Left:
        this->Layout.Y -= this->Layout.Height / 2;
        break;
    case Alignament::Center:
        this->Layout.X -= this->Layout.Width / 2;
        this->Layout.Y -= this->Layout.Height / 2;
        break;
    default:
        RETURNERROR(false, "Invalid alignament value: %d", md.Align);
    };
    return true;
}
bool ControlContext::RecomputeLayout_LeftRightAnchorsAndHeight(const LayoutMetricData& md)
{
    SetControlSize(md.ParentWidth - (md.AnchorLeft + md.AnchorRight), md.Height);

    // convert to PointAndSize
    this->Layout.X = md.AnchorLeft;
    switch (md.Align)
    {
    case Alignament::Top:
        this->Layout.Y = md.Y;
        break;
    case Alignament::Center:
        this->Layout.Y = md.Y - this->Layout.Height / 2;
        break;
    case Alignament::Bottom:
        this->Layout.Y = md.Y - this->Layout.Height;
        break;
    default:
        RETURNERROR(false, "Invalid alignamet (%d) --> only Top, Center and Bottom are allowed !", md.Align);
    }

    return true;
}
bool ControlContext::RecomputeLayout_TopBottomAnchorsAndWidth(const LayoutMetricData& md)
{
    SetControlSize(md.Width, md.ParentHeigh - (md.AnchorTop + md.AnchorBottom));

    // convert to PointAndSize
    this->Layout.Y = md.AnchorTop;
    switch (md.Align)
    {
    case Alignament::Left:
        this->Layout.X = md.X;
        break;
    case Alignament::Center:
        this->Layout.X = md.X - this->Layout.Width / 2;
        break;
    case Alignament::Right:
        this->Layout.X = md.X - this->Layout.Width;
        break;
    default:
        RETURNERROR(false, "Invalid alignamet (%d) --> only Left, Center and Right are allowed !", md.Align);
    }

    return true;
}

bool ControlContext::RecomputeLayout(Control* controlParent)
{
    LayoutMetricData md;
    Graphics::Size sz;

    if (controlParent == nullptr)
        controlParent = this->Parent;
    if (controlParent != nullptr)
    {
        controlParent->GetClientSize(sz);
    }
    else
    {
        CHECK(Application::GetDesktopSize(sz), false, "Fail to get desktop size !");
    }
    // translate values - X & Y Axes
    md.X            = this->Layout.Format.X.ToInt(sz.Width);
    md.Y            = this->Layout.Format.Y.ToInt(sz.Height);
    md.AnchorLeft   = this->Layout.Format.AnchorLeft.ToInt(sz.Width);
    md.AnchorTop    = this->Layout.Format.AnchorTop.ToInt(sz.Height);
    md.AnchorRight  = this->Layout.Format.AnchorRight.ToInt(sz.Width);
    md.AnchorBottom = this->Layout.Format.AnchorBottom.ToInt(sz.Height);
    md.Width        = this->Layout.Format.Width.ToInt(sz.Width);
    md.Height       = this->Layout.Format.Height.ToInt(sz.Height);

    // copy align & anchor
    md.Align       = this->Layout.Format.Align;
    md.Anchor      = this->Layout.Format.Anchor;
    md.ParentWidth = sz.Width;
    md.ParentHeigh = sz.Height;

    // compute position
    switch (this->Layout.Format.LayoutMode)
    {
    case LayoutFormatMode::PointAndSize:
        return RecomputeLayout_PointAndSize(md);
    case LayoutFormatMode::LeftRightAnchorsAndHeight:
        return RecomputeLayout_LeftRightAnchorsAndHeight(md);
    case LayoutFormatMode::TopBottomAnchorsAndWidth:
        return RecomputeLayout_TopBottomAnchorsAndWidth(md);
    case LayoutFormatMode::LeftTopRightAnchorsAndHeight:
        SetControlSize(md.ParentWidth - (md.AnchorLeft + md.AnchorRight), md.Height);
        this->Layout.X = md.AnchorLeft;
        this->Layout.Y = md.AnchorTop;
        return true;
    case LayoutFormatMode::LeftBottomRightAnchorsAndHeight:
        SetControlSize(md.ParentWidth - (md.AnchorLeft + md.AnchorRight), md.Height);
        this->Layout.X = md.AnchorLeft;
        this->Layout.Y = md.ParentHeigh - (md.AnchorBottom + this->Layout.Height);
        return true;
    case LayoutFormatMode::TopLeftBottomAnchorsAndWidth:
        SetControlSize(md.Width, md.ParentHeigh - (md.AnchorTop + md.AnchorBottom));
        this->Layout.X = md.AnchorLeft;
        this->Layout.Y = md.AnchorTop;
        return true;
    case LayoutFormatMode::TopRightBottomAnchorsAndWidth:
        SetControlSize(md.Width, md.ParentHeigh - (md.AnchorTop + md.AnchorBottom));
        this->Layout.X = md.ParentWidth - (md.AnchorRight + this->Layout.Width);
        this->Layout.Y = md.AnchorTop;
        return true;
    case LayoutFormatMode::LeftTopRightBottomAnchors:
        SetControlSize(
              md.ParentWidth - (md.AnchorLeft + md.AnchorRight), md.ParentHeigh - (md.AnchorTop + md.AnchorBottom));
        this->Layout.X = md.AnchorLeft;
        this->Layout.Y = md.AnchorTop;
        return true;
    default:
        RETURNERROR(false, "Unknwon layout format mode: %d", (int) this->Layout.Format.LayoutMode);
    }
}
void ControlContext::PaintScrollbars(Graphics::Renderer& renderer)
{
    int x, y;
    if (ScrollBars.OutsideControl)
    {
        x = Layout.Width;
        y = Layout.Height;
    }
    else
    {
        x = Layout.Width - 1;
        y = Layout.Height - 1;
    }

    if (Flags & GATTR_VSCROLL)
    {
        if (Layout.Height >= (int) (ScrollBars.TopMargin + 2 + MINIM_SCORLL_BAR_LENGTH))
        {
            renderer.FillVerticalLineWithSpecialChar(
                  x, ScrollBars.TopMargin, y - 2, SpecialChars::Block25, Cfg->ScrollBar.Bar);
            renderer.WriteSpecialCharacter(x, ScrollBars.TopMargin, SpecialChars::TriangleUp, Cfg->ScrollBar.Arrows);
            renderer.WriteSpecialCharacter(x, y - 2, SpecialChars::TriangleDown, Cfg->ScrollBar.Arrows);
            if (ScrollBars.MaxVerticalValue)
            {
                const auto sz  = static_cast<uint32>(y - (2 + ScrollBars.TopMargin + 2 /*two arrows*/));
                const auto poz = static_cast<uint32>((sz * ScrollBars.VerticalValue) / ScrollBars.MaxVerticalValue);
                renderer.WriteSpecialCharacter(
                      x, ScrollBars.TopMargin + 1 + poz, SpecialChars::BlockCentered, Cfg->ScrollBar.Position);
            }
        }
    }
    if (Flags & GATTR_HSCROLL)
    {
        if (Layout.Width >= (int) (ScrollBars.LeftMargin + 2 + MINIM_SCORLL_BAR_LENGTH))
        {
            renderer.FillHorizontalLineWithSpecialChar(
                  ScrollBars.LeftMargin, y, x - 2, SpecialChars::Block25, Cfg->ScrollBar.Bar);
            renderer.WriteSpecialCharacter(ScrollBars.LeftMargin, y, SpecialChars::TriangleLeft, Cfg->ScrollBar.Arrows);
            renderer.WriteSpecialCharacter(x - 2, y, SpecialChars::TriangleRight, Cfg->ScrollBar.Arrows);
            if (ScrollBars.MaxHorizontalValue)
            {
                const auto sz  = static_cast<uint32>(x - (2 + ScrollBars.LeftMargin + 2 /*two arrows*/));
                const auto poz = static_cast<uint32>((sz * ScrollBars.HorizontalValue) / ScrollBars.MaxHorizontalValue);
                renderer.WriteSpecialCharacter(
                      ScrollBars.LeftMargin + 1 + poz, y, SpecialChars::BlockCentered, Cfg->ScrollBar.Position);
            }
        }
    }
}
//=======================================================================================================================================================
Controls::Control::~Control()
{
    DELETE_CONTROL_CONTEXT(ControlContext);
}
Controls::Control::Control(void* context, const ConstString& caption, string_view layout, bool computeHotKey)
{
    ASSERT(context, "Expecting a valid context in Control::Control() ctor");
    Application::Config* cfg = Application::GetAppConfig();
    ASSERT(cfg != nullptr, "Unable to get config object !");
    this->Context = context;
    auto ctx      = reinterpret_cast<ControlContext*>(this->Context);
    ctx->Inited   = false;
    ASSERT(ctx->UpdateLayoutFormat(layout), "Invalid format !");

    ConstStringObject captionObj(caption);
    if (computeHotKey)
    {
        ctx->HotKeyOffset = CharacterBuffer::INVALID_HOTKEY_OFFSET;
        if (!captionObj.Data)
        {
            ASSERT(
                  ctx->Text.SetWithHotKey("", ctx->HotKeyOffset, ctx->HotKey, Key::Alt, NoColorPair),
                  "Fail to set text with UTF8 value");
        }
        else
        {
            ASSERT(
                  ctx->Text.SetWithHotKey(caption, ctx->HotKeyOffset, ctx->HotKey, Key::Alt, NoColorPair),
                  "Fail to set text with UTF8 value");
        }
    }
    else
    {
        if (!captionObj.Data)
        {
            ASSERT(ctx->Text.Set("", NoColorPair), "Fail to set text with UTF8 value");
        }
        else
        {
            ASSERT(ctx->Text.Set(caption, NoColorPair), "Fail to set text with UTF8 value");
        }
    }

    // all good
    ctx->Inited = true;
}

Reference<Control> Controls::Control::AddChildControl(unique_ptr<Control> ctrl)
{
    CHECK(ctrl, nullptr, "Invalid control (nullptr)");
    CHECK(ctrl->IsInitialized(), nullptr, "Control was not initialized before adding it to a parent control !");
    // LOG_INFO("AddControl(this=%p,Control=%p,Controls List=%p,Count=%d", this, ctrl, Controls, ControlsCount);
    if (CTRLC->Controls == nullptr) // first time
    {
        // LOG_INFO("Allocate 8 controls for current container %p", this);
        CTRLC->Controls = new Control*[8];
        CHECK(CTRLC->Controls != nullptr, nullptr, "");
        CTRLC->ControlsCount = 0;
    }
    else
    {
        if (((CTRLC->ControlsCount % 8) == 0) && (CTRLC->ControlsCount > 0))
        {
            // grow
            Control** tmp = new Control*[CTRLC->ControlsCount + 8];
            CHECK(tmp != nullptr, nullptr, "");
            for (uint32 tr = 0; tr < CTRLC->ControlsCount; tr++)
                tmp[tr] = CTRLC->Controls[tr];
            delete CTRLC->Controls;
            CTRLC->Controls = tmp;
        }
    }
    if (OnBeforeAddControl(ctrl.get()) == false)
        return nullptr;

    auto p_ctrl                                   = ctrl.release();
    CTRLC->Controls[CTRLC->ControlsCount++]       = p_ctrl;
    ((ControlContext*) (p_ctrl->Context))->Parent = this;
    OnAfterAddControl(p_ctrl);
    // Recompute layouts
    (reinterpret_cast<ControlContext*>(p_ctrl->Context))->RecomputeLayout(this);
    RecomputeLayout();
    // Force a recompute layout on the entire app
    auto app = Application::GetApplication();
    if (app)
        app->RepaintStatus = REPAINT_STATUS_ALL;
    return p_ctrl;
}
bool Controls::Control::RemoveControl(Control* control)
{
    uint32 index;
    if (GetChildIndex(control, index) == false)
        return false;
    return RemoveControlByID(index);
}
bool Controls::Control::RemoveControlByRef(Reference<Control> control)
{
    CHECK(control.IsValid(), false, "Emptry control reference !");
    Control** lst = CTRLC->Controls;
    Control** end = lst + (CTRLC->ControlsCount);
    if (lst == nullptr)
        return false;
    uint32 index   = 0xFFFFFFFF;
    uint32 c_index = 0;
    while (lst < end)
    {
        if (control == (*lst))
        {
            index = c_index;
            break;
        }
        c_index++;
        lst++;
    }
    if (index == 0xFFFFFFFF)
        return false;
    bool result = RemoveControlByID(index);
    if (result)
        control.Reset();
    return result;
}
bool Controls::Control::RemoveControlByID(uint32 index)
{
    CHECK(index < CTRLC->ControlsCount,
          false,
          "Invalid index for control child: %d (should be between 0 and %d)",
          index,
          CTRLC->ControlsCount - 1);
    Control** lst = CTRLC->Controls;
    CHECK(lst != nullptr, false, "Expecting a non-nullptr list of control !");
    this->OnControlRemoved(lst[index]);
    uint32 count = CTRLC->ControlsCount;
    index++;
    while (index < count)
    {
        lst[index - 1] = lst[index];
        index++;
    }
    CTRLC->ControlsCount--;
    CTRLC->CurrentControlIndex = 0;
    return true;
}
int Controls::Control::GetX() const
{
    return CTRLC->Layout.X;
}
int Controls::Control::GetY() const
{
    return CTRLC->Layout.Y;
}
int Controls::Control::GetWidth() const
{
    return CTRLC->Layout.Width;
}
int Controls::Control::GetHeight() const
{
    return CTRLC->Layout.Height;
}
void Controls::Control::GetSize(Graphics::Size& size)
{
    size.Width  = CTRLC->Layout.Width;
    size.Height = CTRLC->Layout.Height;
}
void Controls::Control::GetClientSize(Graphics::Size& size)
{
    int w = CTRLC->Layout.Width - (CTRLC->Margins.Left + CTRLC->Margins.Right);
    int h = CTRLC->Layout.Height - (CTRLC->Margins.Top + CTRLC->Margins.Bottom);
    if (w < 0)
        w = 0;
    if (h <= 0)
        h = 0;
    size.Width  = w;
    size.Height = h;
}
bool Controls::Control::IsMouseInControl(int x, int y)
{
    return (x >= 0) && (y >= 0) && (x < (CTRLC->Layout.Width)) && (y < (CTRLC->Layout.Height));
}
void Controls::Control::SetChecked(const bool value)
{
    if (value)
        CTRLC->Flags |= GATTR_CHECKED;
    else
        CTRLC->Flags -= ((CTRLC->Flags) & GATTR_CHECKED);
}
void Controls::Control::SetEnabled(const bool value)
{
    if (value)
        CTRLC->Flags |= GATTR_ENABLE;
    else
        CTRLC->Flags -= ((CTRLC->Flags) & GATTR_ENABLE);
}
void Controls::Control::SetVisible(const bool value)
{
    if (value)
        CTRLC->Flags |= GATTR_VISIBLE;
    else
        CTRLC->Flags -= ((CTRLC->Flags) & GATTR_VISIBLE);
}

bool Controls::Control::IsChecked() const
{
    return (((CTRLC->Flags) & GATTR_CHECKED) != 0);
}
bool Controls::Control::IsEnabled() const
{
    return (((CTRLC->Flags) & GATTR_ENABLE) != 0);
}
bool Controls::Control::IsVisible() const
{
    return (((CTRLC->Flags) & GATTR_VISIBLE) != 0);
}
bool Controls::Control::HasFocus() const
{
    return CTRLC->Focused;
}
bool Controls::Control::IsMouseOver() const
{
    return CTRLC->MouseIsOver;
}

Reference<Control> Controls::Control::GetParent()
{
    return CTRLC->Parent;
}
Control** Controls::Control::GetChildrenList()
{
    return CTRLC->Controls;
}
Reference<Control> Controls::Control::GetChild(uint32 index)
{
    CHECK(index < CTRLC->ControlsCount,
          nullptr,
          "Invalid index for control child: %d (should be between 0 and %d)",
          index,
          CTRLC->ControlsCount - 1);
    return CTRLC->Controls[index];
}
uint32 Controls::Control::GetChildrenCount()
{
    return CTRLC->ControlsCount;
}

bool Controls::Control::GetChildIndex(Reference<Control> control, uint32& index)
{
    Control** lst = CTRLC->Controls;
    Control** end = lst + (CTRLC->ControlsCount);
    if (lst == nullptr)
        return false;
    uint32 c_index = 0;
    while (lst < end)
    {
        if ((*lst) == control)
        {
            index = c_index;
            return true;
        }
        c_index++;
        lst++;
    }
    return false;
}

void Controls::Control::MoveTo(int newX, int newY)
{
    Application::Config* cfg = Application::GetAppConfig();
    if (!cfg)
        return;
    if ((newX == CTRLC->Layout.X) && (newY == CTRLC->Layout.Y))
        return;
    CTRLC->Layout.X = newX;
    CTRLC->Layout.Y = newY;
    Application::RecomputeControlsLayout();
    Application::Repaint();
}
bool Controls::Control::Resize(int newWidth, int newHeight)
{
    Application::Config* cfg = Application::GetAppConfig();
    CHECK(cfg != nullptr, false, "Unable to get config object !");

    if (newWidth < CTRLC->Layout.MinWidth)
        newWidth = CTRLC->Layout.MinWidth;
    if (newWidth > CTRLC->Layout.MaxWidth)
        newWidth = CTRLC->Layout.MaxWidth;

    if (newHeight < CTRLC->Layout.MinHeight)
        newHeight = CTRLC->Layout.MinHeight;
    if (newHeight > CTRLC->Layout.MaxHeight)
        newHeight = CTRLC->Layout.MaxHeight;

    if ((newWidth == CTRLC->Layout.Width) && (newHeight == CTRLC->Layout.Height))
        return true;
    if (newWidth < 1)
        newWidth = 1;
    if (newHeight < 1)
        newHeight = 1;

    if (OnBeforeResize(newWidth, newHeight) == false)
        return false;

    CTRLC->Layout.Width  = newWidth;
    CTRLC->Layout.Height = newHeight;
    RecomputeLayout();
    return true;
}
void Controls::Control::RecomputeLayout()
{
    for (uint32 tr = 0; tr < CTRLC->ControlsCount; tr++)
    {
        ((ControlContext*) (CTRLC->Controls[tr]->Context))->RecomputeLayout(this);
        CTRLC->Controls[tr]->RecomputeLayout();
    }
    OnAfterResize(CTRLC->Layout.Width, CTRLC->Layout.Height);

    Application::RecomputeControlsLayout();
}
bool Controls::Control::SetText(const ConstString& caption, bool updateHotKey)
{
    if (OnBeforeSetText(caption) == false)
        return false;
    if (updateHotKey)
    {
        if (CTRLC->Text.SetWithHotKey(caption, CTRLC->HotKeyOffset, CTRLC->HotKey, Key::Alt, NoColorPair) == false)
            return false;
    }
    else
    {
        if (CTRLC->Text.Set(caption, NoColorPair) == false)
            return false;
    }
    if (CTRLC->handlers)
    {
        if (CTRLC->handlers->OnAfterSetText.obj)
            CTRLC->handlers->OnAfterSetText.obj->OnAfterSetText(this);
        else
            OnAfterSetText();
    }
    else
        OnAfterSetText();
    return true;
}
bool Controls::Control::SetText(const Graphics::CharacterBuffer& text)
{
    if (OnBeforeSetText((CharacterView) text) == false)
        return false;
    if (CTRLC->Text.Set(text) == false)
        return false;
    if (CTRLC->handlers)
    {
        if (CTRLC->handlers->OnAfterSetText.obj)
            CTRLC->handlers->OnAfterSetText.obj->OnAfterSetText(this);
        else
            OnAfterSetText();
    }
    else
        OnAfterSetText();
    return true;
}
bool Controls::Control::SetTextWithHotKey(const ConstString& caption, uint32 hotKeyTextOffset)
{
    CHECK(SetText(caption), false, "");
    ConstStringObject txt(caption);
    bool result = false;
    if (hotKeyTextOffset < txt.Length)
    {
        switch (txt.Encoding)
        {
        case StringEncoding::Ascii:
            result = this->SetHotKey(((const char*) txt.Data)[hotKeyTextOffset]);
            break;
        case StringEncoding::Unicode16:
            result = this->SetHotKey(((const char16*) txt.Data)[hotKeyTextOffset]);
            break;
        case StringEncoding::CharacterBuffer:
            result = this->SetHotKey(((const Character*) txt.Data)[hotKeyTextOffset].Code);
            break;
        case StringEncoding::UTF8:
            result = this->SetHotKey(((const uint8*) txt.Data)[hotKeyTextOffset]);
            break;
        }
        if (result)
        {
            CTRLC->HotKeyOffset = hotKeyTextOffset;
        }
    }
    return true;
}
const Graphics::CharacterBuffer& Controls::Control::GetText()
{
    return CTRLC->Text;
}
void Controls::Control::UpdateHScrollBar(uint64 value, uint64 maxValue)
{
    if (value > maxValue)
        value = maxValue;
    CTRLC->ScrollBars.HorizontalValue    = value;
    CTRLC->ScrollBars.MaxHorizontalValue = maxValue;
}
void Controls::Control::UpdateVScrollBar(uint64 value, uint64 maxValue)
{
    if (value > maxValue)
        value = maxValue;
    CTRLC->ScrollBars.VerticalValue    = value;
    CTRLC->ScrollBars.MaxVerticalValue = maxValue;
}
int Controls::Control::GetGroup()
{
    return CTRLC->GroupID;
}
void Controls::Control::SetGroup(int newGroupID)
{
    if (newGroupID >= 0)
        CTRLC->GroupID = newGroupID;
}
void Controls::Control::ClearGroup()
{
    CTRLC->GroupID = 0;
}

bool Controls::Control::SetHotKey(char16 hotKey)
{
    CTRLC->HotKeyOffset = CharacterBuffer::INVALID_HOTKEY_OFFSET;
    CTRLC->HotKey       = Utils::KeyUtils::CreateHotKey(hotKey, Key::Alt);
    this->OnHotKeyChanged();
    return CTRLC->HotKey != Key::None;
}
bool Controls::Control::SetMargins(int left, int top, int right, int bottom)
{
    CHECK(left >= 0, false, "left margin must be positive values !");
    CHECK(top >= 0, false, "top margin must be positive values !");
    CHECK(right >= 0, false, "right margin must be positive values !");
    CHECK(bottom >= 0, false, "bottom margin must be positive values !");
    CTRLC->Margins.Left   = left;
    CTRLC->Margins.Right  = right;
    CTRLC->Margins.Top    = top;
    CTRLC->Margins.Bottom = bottom;
    return true;
}
Input::Key Controls::Control::GetHotKey()
{
    return CTRLC->HotKey;
}
void Controls::Control::ClearHotKey()
{
    CTRLC->HotKey       = Key::None;
    CTRLC->HotKeyOffset = CharacterBuffer::INVALID_HOTKEY_OFFSET;
}
uint32 Controls::Control::GetHotKeyTextOffset()
{
    return CTRLC->HotKeyOffset;
}
void Controls::Control::SetControlID(int newID)
{
    if (newID > 0)
        CTRLC->ControlID = newID;
    else
        CTRLC->ControlID = 0;
}
bool Controls::Control::SetFocus()
{
    Control* obj = this;
    Control* p;
    uint32 tr, count;

    /*if (CTRLC->Focused)
        return true;*/
    // verifica daca pot sa dau focus
    while (obj != nullptr)
    {
        if (((((ControlContext*) (obj->Context))->Flags) & (GATTR_ENABLE | GATTR_VISIBLE)) !=
            (GATTR_ENABLE | GATTR_VISIBLE))
            return false;
        obj = ((ControlContext*) (obj->Context))->Parent;
    }
    // setez focusul la toti
    obj = this;
    p   = ((ControlContext*) (obj->Context))->Parent;

    while ((obj != nullptr) && (p != nullptr))
    {
        // caut obiectul
        ((ControlContext*) (p->Context))->CurrentControlIndex = -1;
        count                                                 = ((ControlContext*) (p->Context))->ControlsCount;
        for (tr = 0; tr < count; tr++)
        {
            if (((ControlContext*) (p->Context))->Controls[tr] == obj)
            {
                ((ControlContext*) (p->Context))->CurrentControlIndex = tr;
                break;
            }
        }
        if (((ControlContext*) (p->Context))->CurrentControlIndex == static_cast<unsigned>(-1))
            return false;
        obj = p;
        p   = ((ControlContext*) (p->Context))->Parent;
    }
    Application::Repaint();
    // acceleratorii
    // UpdateCommandBar(this);
    return true;
}
bool Controls::Control::ShowToolTip(const ConstString& caption)
{
    auto app = Application::GetApplication();
    CHECK(app, false, "Application was not initialized !");
    return app->SetToolTip(this, caption);
}
bool Controls::Control::ShowToolTip(const ConstString& caption, int x, int y)
{
    auto app = Application::GetApplication();
    CHECK(app, false, "Application was not initialized !");
    return app->SetToolTip(this, caption, x, y);
}
void Controls::Control::HideToolTip()
{
    auto app = Application::GetApplication();
    if (app)
        app->ToolTip.Hide();
}
void Controls::Control::RaiseEvent(Event eventType)
{
    Application::RaiseEvent(this, this, eventType, CTRLC->ControlID);
}
void Controls::Control::RaiseEvent(Event eventType, int ID)
{
    Application::RaiseEvent(this, this, eventType, ID);
}
void Controls::Control::Paint(Graphics::Renderer& /*renderer*/)
{
}
bool Controls::Control::IsInitialized()
{
    CHECK(this->Context, false, "Control context was not initialized !");
    return CTRLC->Inited;
}
// Evenimente
bool Controls::Control::OnKeyEvent(Input::Key, char16)
{
    return false;
}
void Controls::Control::OnFocus()
{
}
void Controls::Control::OnLoseFocus()
{
}
void Controls::Control::OnMouseReleased(int, int, Input::MouseButton)
{
}
void Controls::Control::OnMousePressed(int, int, Input::MouseButton)
{
}
bool Controls::Control::OnMouseDrag(int, int, Input::MouseButton)
{
    return false;
}
bool Controls::Control::OnMouseOver(int, int)
{
    return false;
}
bool Controls::Control::OnMouseEnter()
{
    return false;
}
bool Controls::Control::OnMouseLeave()
{
    return false;
}
bool Controls::Control::OnMouseWheel(int, int, Input::MouseWheel)
{
    return false;
}
void Controls::Control::OnHotKey()
{
}
void Controls::Control::OnHotKeyChanged()
{
}
bool Controls::Control::OnEvent(Reference<Control>, Event, int)
{
    return false;
}
bool Controls::Control::OnUpdateCommandBar(Application::CommandBar&)
{
    return false;
}
bool Controls::Control::OnBeforeResize(int, int)
{
    return true;
}
bool Controls::Control::OnFrameUpdate()
{
    return false;
}
void Controls::Control::OnAfterResize(int, int)
{
}
bool Controls::Control::OnBeforeAddControl(Reference<Control> ctrl)
{
    return (ctrl != nullptr);
}
void Controls::Control::OnControlRemoved(Reference<Control>)
{
}
void Controls::Control::OnExpandView(Graphics::Clip&)
{
}
void Controls::Control::OnPackView()
{
}
void Controls::Control::OnAfterAddControl(Reference<Control>)
{
    // daca e primul - setez si tab-ul
    if (CTRLC->ControlsCount == 1)
        CTRLC->CurrentControlIndex = 0;
}
bool Controls::Control::OnBeforeSetText(const ConstString&)
{
    return true;
}
void Controls::Control::OnAfterSetText()
{
}
void Controls::Control::OnUpdateScrollBars()
{
}

Handlers::Control* Controls::Control::Handlers()
{
    GET_CONTROL_HANDLERS(Handlers::Control);
}

void Controls::Control::ExpandView()
{
    Application::GetApplication()->ExpandControl(this);
}
void Controls::Control::PackView()
{
    Application::GetApplication()->PackControl(true);
}
} // namespace AppCUI