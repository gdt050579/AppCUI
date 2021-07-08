#include "ControlContext.h"
#include <string.h>
#include <cstring>

using namespace AppCUI::Controls;
using namespace AppCUI::Input;
using namespace AppCUI::Console;

#define CTRLC	                        ((ControlContext*)Context)

#define CHAR_TYPE_EOS                   0
#define CHAR_TYPE_OTHER                 1
#define CHAR_TYPE_WORD                  2
#define CHAR_TYPE_NUMBER                3
#define CHAR_TYPE_SPACE                 4
#define CHAR_TYPE_EQ                    5
#define CHAR_TYPE_SEPARATOR             6
#define CHAR_TYPE_POINT                 7
#define CHAR_TYPE_MINUS                 8


#define LAYOUT_FLAG_WIDTH               0x0001
#define LAYOUT_FLAG_HEIGHT              0x0002
#define LAYOUT_FLAG_ANCH_LEFT           0x0004
#define LAYOUT_FLAG_ANCH_RIGHT          0x0008
#define LAYOUT_FLAG_ANCH_TOP            0x0010
#define LAYOUT_FLAG_ANCH_BOTTOM         0x0020
#define LAYOUT_FLAG_ANCHOR              0x0040
#define LAYOUT_FLAG_X                   0x0100
#define LAYOUT_FLAG_Y                   0x0200



#define LAYOUT_MODE_TOP_LEFT            0 // X,Y + W,H
#define LAYOUT_MODE_TOP                 1
#define LAYOUT_MODE_TOP_RIGHT           2
#define LAYOUT_MODE_RIGHT               3
#define LAYOUT_MODE_BOTTOM_RIGHT        4
#define LAYOUT_MODE_BOTTOM              5
#define LAYOUT_MODE_BOTTOM_LEFT         6
#define LAYOUT_MODE_LEFT                7
#define LAYOUT_MODE_CENTER              8




#define SKIP_SPACES                     while (__char_types__[*p] == CHAR_TYPE_SPACE) p++;
#define UPDATE_STRING_HASH(resultValue) while (__char_types__[*p] == CHAR_TYPE_WORD) { resultValue = ((resultValue) << 2) ^ ((unsigned int)(('Z' + 1) - (((*p) & ((unsigned char)(~0x20)))))); p++; }

// for gcc, building a field should look like var.field, not var.##field
// http://gcc.gnu.org/onlinedocs/cpp/Concatenation.html
#define SET_LAYOUT_INFO(flag,field)     { inf.flags |= flag; inf.field = value; if (l.ValueType=='%') inf.percentagesMask |= flag; }

#define TRANSFER_MASK(fromBit,toBit) { \
    if (inf.percentagesMask & fromBit) \
        inf.percentagesMask |= toBit; \
    else \
        inf.percentagesMask -= (inf.percentagesMask & toBit); \
}

struct LayoutInformation
{
    unsigned int        flags;
    unsigned int        percentagesMask;
    int                 x, y;
    int                 width, height;
    int                 a_left, a_top, a_right, a_bottom;
    Alignament::Type    anchor;
};
struct LayoutMetricData
{
    int                 ParentWidth, ParentHeight;
    unsigned int        PercentagesMask;
};
struct LayoutKeyValueData
{
    const char *    HashName;
    unsigned int    Hash;
    int             n1, n2;
    unsigned int    StringValueHash;
    bool            IsNegative;
    char            ValueType;
};

unsigned char __char_types__[256] = { CHAR_TYPE_EOS,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_SPACE,CHAR_TYPE_SPACE,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_SPACE,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_SPACE,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_SEPARATOR,CHAR_TYPE_MINUS,CHAR_TYPE_POINT,CHAR_TYPE_OTHER,CHAR_TYPE_NUMBER,CHAR_TYPE_NUMBER,CHAR_TYPE_NUMBER,CHAR_TYPE_NUMBER,CHAR_TYPE_NUMBER,CHAR_TYPE_NUMBER,CHAR_TYPE_NUMBER,CHAR_TYPE_NUMBER,CHAR_TYPE_NUMBER,CHAR_TYPE_NUMBER,CHAR_TYPE_EQ,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_EQ,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_WORD,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER,CHAR_TYPE_OTHER };
 
#define HASH_KEYWORD_X                              0x00000003
#define HASH_KEYWORD_Y                              0x00000002
#define HASH_KEYWORD_ALIGN                          0x000018BD
#define HASH_KEYWORD_A                              0x0000001A
#define HASH_KEYWORD_ALIGNAMENT                     0x0062ECD3
#define HASH_KEYWORD_W                              0x00000004
#define HASH_KEYWORD_WIDTH                          0x000001FF
#define HASH_KEYWORD_H                              0x00000013
#define HASH_KEYWORD_HEIGHT                         0x00005F8B
#define HASH_KEYWORD_ANCH                           0x00000623
#define HASH_KEYWORD_ANCHOR                         0x00006209
#define HASH_KEYWORD_AN                             0x00000065
#define HASH_KEYWORD_ANC                            0x0000018C
#define HASH_KEYWORD_L                              0x0000000F
#define HASH_KEYWORD_LEFT                           0x000002F3
#define HASH_KEYWORD_AL                             0x00000067
#define HASH_KEYWORD_ANCHORLEFT                     0x00620BF3
#define HASH_KEYWORD_R                              0x00000009
#define HASH_KEYWORD_RIGHT                          0x00000C8B
#define HASH_KEYWORD_AR                             0x00000061
#define HASH_KEYWORD_ANCHORRIGHT                    0x0188288B
#define HASH_KEYWORD_T                              0x00000007
#define HASH_KEYWORD_TOP                            0x0000004B
#define HASH_KEYWORD_AT                             0x0000006F
#define HASH_KEYWORD_ANCHORTOP                      0x0018820B
#define HASH_KEYWORD_B                              0x00000019
#define HASH_KEYWORD_BOTTOM                         0x0000698E
#define HASH_KEYWORD_AB                             0x00000071
#define HASH_KEYWORD_ANCHORBOTTOM                   0x0620F98E

#define HASH_ALIGN_LEFT                             0x000002F3
#define HASH_ALIGN_L                                0x0000000F
#define HASH_ALIGN_TOP                              0x0000004B
#define HASH_ALIGN_T                                0x00000007
#define HASH_ALIGN_RIGHT                            0x00000C8B
#define HASH_ALIGN_R                                0x00000009
#define HASH_ALIGN_BOTTOM                           0x0000698E
#define HASH_ALIGN_B                                0x00000019
#define HASH_ALIGN_TOPLEFT                          0x000049F3
#define HASH_ALIGN_TL                               0x00000013
#define HASH_ALIGN_LEFTTOP                          0x0000BC8B
#define HASH_ALIGN_LT                               0x0000003B
#define HASH_ALIGN_TOPRIGHT                         0x0001208B
#define HASH_ALIGN_TR                               0x00000015
#define HASH_ALIGN_RIGHTTOP                         0x0003228B
#define HASH_ALIGN_RT                               0x00000023
#define HASH_ALIGN_BOTTOMRIGHT                      0x01A6348B
#define HASH_ALIGN_BR                               0x0000006D
#define HASH_ALIGN_RIGHTBOTTOM                      0x00C8D98E
#define HASH_ALIGN_RB                               0x0000003D
#define HASH_ALIGN_BOTTOMLEFT                       0x00698CF3
#define HASH_ALIGN_BL                               0x0000006B
#define HASH_ALIGN_LEFTBOTTOM                       0x002F598E
#define HASH_ALIGN_LB                               0x00000025
#define HASH_ALIGN_CENTER                           0x00007561
#define HASH_ALIGN_C                                0x00000018

// <xxx> (Arrow left, 3 character, Arrow right)
#define MINIM_SCORLL_BAR_LENGTH                     5

bool HashToAlignament(unsigned int hash, Alignament::Type & result)
{
    switch (hash)
    {
        case HASH_ALIGN_TOPLEFT:
        case HASH_ALIGN_TL:
        case HASH_ALIGN_LEFTTOP:
        case HASH_ALIGN_LT:
            result = Alignament::TopLeft;
            return true;
        case HASH_ALIGN_TOP:
        case HASH_ALIGN_T:
            result = Alignament::Top;
            return true;
        case HASH_ALIGN_TOPRIGHT:
        case HASH_ALIGN_TR:
        case HASH_ALIGN_RIGHTTOP:
        case HASH_ALIGN_RT:
            result = Alignament::TopRight;
            return true;
        case HASH_ALIGN_RIGHT:
        case HASH_ALIGN_R:
            result = Alignament::Right;
            return true;
        case HASH_ALIGN_BOTTOMRIGHT:
        case HASH_ALIGN_BR:
        case HASH_ALIGN_RIGHTBOTTOM:
        case HASH_ALIGN_RB:
            result = Alignament::BottomRight;
            return true;
        case HASH_ALIGN_BOTTOM:
        case HASH_ALIGN_B:
            result = Alignament::Bottom;
            return true;
        case HASH_ALIGN_BOTTOMLEFT:
        case HASH_ALIGN_BL:
        case HASH_ALIGN_LEFTBOTTOM:
        case HASH_ALIGN_LB:
            result = Alignament::BottomLeft;
            return true;
        case HASH_ALIGN_LEFT:
        case HASH_ALIGN_L:
            result = Alignament::Left;
            return true;
        case HASH_ALIGN_CENTER:
        case HASH_ALIGN_C:
            result = Alignament::Center;
            return true;
    }
    RETURNERROR(false, "Unknwon align hash: %08X", hash);
}

bool ProcessLayoutKeyValueData(LayoutKeyValueData & l, LayoutInformation & inf, AppCUI::Application::Config * Cfg)
{
    int value = 0;    
    switch (l.ValueType)
    {
        case 0   :
        case 'p' :
        case 'P' :
            // pixels
            CHECK(l.n2 == 0, false, "Pixel values can not be floating values !");
            value = l.n1;
            l.ValueType = 'p';
            break;
        case '%':
            // percentage
            value = l.n1 * 100 + l.n2;
            break;
        case 'u':
        case 'U':
            // units
            CHECK(l.n2 == 0, false, "Units values can not be floating values !");
            value = l.n1;
            l.ValueType = 'u';
            break;
        default:
            RETURNERROR(false, "Invalid value format: %d (%c)", l.ValueType, l.ValueType);
    }
    if (l.IsNegative)
        value = -value;
    switch(l.Hash)
    {
        case HASH_KEYWORD_X: 
            SET_LAYOUT_INFO(LAYOUT_FLAG_X, x);            
            break;
        case HASH_KEYWORD_Y:
            SET_LAYOUT_INFO(LAYOUT_FLAG_Y, y);            
            break;
        case HASH_KEYWORD_W:
        case HASH_KEYWORD_WIDTH:
            SET_LAYOUT_INFO(LAYOUT_FLAG_WIDTH, width);            
            break;
        case HASH_KEYWORD_H:
        case HASH_KEYWORD_HEIGHT:
            SET_LAYOUT_INFO(LAYOUT_FLAG_HEIGHT, height);            
            break;
        case HASH_KEYWORD_A:
        case HASH_KEYWORD_AN:
        case HASH_KEYWORD_ANC:
        case HASH_KEYWORD_ANCH:
        case HASH_KEYWORD_ANCHOR:
            CHECK(HashToAlignament(l.StringValueHash, inf.anchor), false, "Fail to compute anchor value !");
            break;
        case HASH_KEYWORD_L:
        case HASH_KEYWORD_AL:
        case HASH_KEYWORD_LEFT:
        case HASH_KEYWORD_ANCHORLEFT:
            SET_LAYOUT_INFO(LAYOUT_FLAG_ANCH_LEFT, a_left);
            break;
        case HASH_KEYWORD_R:
        case HASH_KEYWORD_AR:
        case HASH_KEYWORD_RIGHT:
        case HASH_KEYWORD_ANCHORRIGHT:
            SET_LAYOUT_INFO(LAYOUT_FLAG_ANCH_RIGHT, a_right);
            break;
        case HASH_KEYWORD_T:
        case HASH_KEYWORD_AT:
        case HASH_KEYWORD_TOP:
        case HASH_KEYWORD_ANCHORTOP:
            SET_LAYOUT_INFO(LAYOUT_FLAG_ANCH_TOP, a_top);
            break;
        case HASH_KEYWORD_B:
        case HASH_KEYWORD_AB:
        case HASH_KEYWORD_BOTTOM:
        case HASH_KEYWORD_ANCHORBOTTOM:
            SET_LAYOUT_INFO(LAYOUT_FLAG_ANCH_BOTTOM, a_bottom);
            break;
        default:
            RETURNERROR(false, "Unknown key (hash:%08x) ==> %s", l.Hash, l.HashName);
    }
    return true;
}
bool AnalyzeLayout(const char * layout, LayoutInformation & inf, AppCUI::Application::Config * Cfg)
{
    // format: key:value,[key:value],....
    CHECK(layout, false, "Expecting a valid layout");
    const unsigned char * p = (const unsigned char *)layout;
    LayoutKeyValueData lkv;
    inf.flags = inf.percentagesMask = 0;
    inf.x = inf.y = 0;
    inf.a_left = inf.a_bottom = inf.a_top = inf.a_right = 0;
    inf.width = 1;
    inf.height = 1;
    inf.anchor = Alignament::TopLeft;
    int cnt;
    while (*p)
    {
        SKIP_SPACES;
        lkv.Hash = 0;
        if (!(*p)) break;
        lkv.HashName = (const char *)p;
        UPDATE_STRING_HASH(lkv.Hash);
        CHECK(lkv.Hash, false, "Invalid hash (expecting a valid key: %s)", p);
        SKIP_SPACES;
        CHECK(__char_types__[*p] == CHAR_TYPE_EQ, false, "Expecting ':' or '=' character (%s)", p);
        p++;
        SKIP_SPACES;
        lkv.n1 = lkv.n2 = 0;
        lkv.StringValueHash = 0;
        lkv.IsNegative = false;
        lkv.ValueType = 0;
        if ((*p) == '-') { 
            lkv.IsNegative = true;
            p++; 
        }
        if ((!lkv.IsNegative) && (__char_types__[*p] == CHAR_TYPE_WORD))
        {
            UPDATE_STRING_HASH(lkv.StringValueHash);
            CHECK(lkv.StringValueHash, false, "Invalid value hash (expecting a valid key: %s)", p);
        } else
        {
            while (__char_types__[*p] == CHAR_TYPE_NUMBER)
            {
                lkv.n1 = lkv.n1 * 10 + ((*p) - '0');
                p++;
            }
            if ((*p) == '.')
            {
                p++;
                cnt = 0;
                while (__char_types__[*p] == CHAR_TYPE_NUMBER)
                {
                    if (cnt < 2) {
                        lkv.n2 = lkv.n2 * 10 + ((*p) - '0');
                        cnt++;
                    }
                    p++;
                }
                if (cnt == 1)
                    lkv.n2 *= 10;
            }
        }
        SKIP_SPACES;
        if (((*p) != 0) && (__char_types__[*p] != CHAR_TYPE_SEPARATOR))
        {
            lkv.ValueType = *p;
            while ((*p) && (__char_types__[*p] != CHAR_TYPE_SEPARATOR))
                p++;
        }
        if (__char_types__[*p] == CHAR_TYPE_SEPARATOR)
            p++;
        SKIP_SPACES;
        CHECK(ProcessLayoutKeyValueData(lkv, inf, Cfg), false, "Invalid layout params !");
    }
    return true;
}
int  ComputeXAxesValue(int formatValue, const LayoutMetricData & md, unsigned int layoutFlag)
{
    if (md.PercentagesMask & layoutFlag)
    {
        return (formatValue * md.ParentWidth) / 10000;
    }
    else {
        return formatValue;
    }
}
int  ComputeYAxesValue(int formatValue, const LayoutMetricData & md, unsigned int layoutFlag)
{
    if (md.PercentagesMask & layoutFlag)
    {
        return (formatValue * md.ParentHeight) / 10000;
    }
    else {
        return formatValue;
    }
}

ControlContext::ControlContext()
{
	this->Controls = nullptr;
	this->ControlsCount = 0;
	this->CurrentControlIndex = -1;
	this->Margins.Left = this->Margins.Right = this->Margins.Top = this->Margins.Bottom = 0;
	this->Parent = nullptr;
	this->GroupID = 0;
	this->HotKey = Key::None;
	this->Layout.X = this->Layout.Y = this->Layout.Width = this->Layout.Height = 0;
    this->Layout.MinWidth = this->Layout.MinHeight = 0;
    this->Layout.MaxWidth = this->Layout.MaxHeight = 2000000000; 
	this->ScreenClip.Visible = false;
	this->Inited = false;
	this->ControlID = 0;
	this->Focused = false;
    this->MouseIsOver = false;
    this->Cfg = AppCUI::Application::GetAppConfig();
    this->HotKeyOffset = 0xFFFFFFFF;
    this->ScrollBars.LeftMargin = 2;
    this->ScrollBars.TopMargin = 2;
    this->ScrollBars.VerticalValue = 0;
    this->ScrollBars.HorizontalValue = 0;
    this->ScrollBars.MaxHorizontalValue = 0;
    this->ScrollBars.MaxVerticalValue = 0;
	// curat automat
	memset(&this->Handlers, 0, sizeof(this->Handlers));
}
bool ControlContext::UpdateLayoutFormat(const char * format)
{
    CHECK(format != nullptr, false, "Expecting a valid (non-null) format !");
    LayoutInformation inf;    
    CHECK(AnalyzeLayout(format, inf, this->Cfg), false, "Fail to load format data !");
    
    // validez ca sunt ok 

    this->Layout.Format.AnchorLeft = inf.a_left;
    this->Layout.Format.AnchorRight = inf.a_right;
    this->Layout.Format.AnchorTop = inf.a_top;
    this->Layout.Format.AnchorBottom = inf.a_bottom;


    unsigned int anch_flags = inf.flags & (LAYOUT_FLAG_ANCH_BOTTOM | LAYOUT_FLAG_ANCH_TOP | LAYOUT_FLAG_ANCH_RIGHT | LAYOUT_FLAG_ANCH_LEFT);
    if ((inf.flags & (LAYOUT_FLAG_X | LAYOUT_FLAG_Y)) != 0)
    {
        switch (inf.anchor)
        {
            case Alignament::TopLeft:       
                inf.a_left = inf.x; TRANSFER_MASK(LAYOUT_FLAG_X, LAYOUT_FLAG_ANCH_LEFT); 
                inf.a_top = inf.y; TRANSFER_MASK(LAYOUT_FLAG_Y, LAYOUT_FLAG_ANCH_TOP);
                break;
            case Alignament::Top:           
                inf.a_top = inf.y; TRANSFER_MASK(LAYOUT_FLAG_Y, LAYOUT_FLAG_ANCH_TOP); 
                break;
            case Alignament::TopRight:      
                inf.a_right = inf.x; TRANSFER_MASK(LAYOUT_FLAG_X, LAYOUT_FLAG_ANCH_RIGHT);
                inf.a_top = inf.y; TRANSFER_MASK(LAYOUT_FLAG_Y, LAYOUT_FLAG_ANCH_TOP);
                break;
            case Alignament::Right:
                inf.a_right = inf.x; TRANSFER_MASK(LAYOUT_FLAG_X, LAYOUT_FLAG_ANCH_RIGHT);
                break;
            case Alignament::BottomRight:   
                inf.a_right = inf.x; TRANSFER_MASK(LAYOUT_FLAG_X, LAYOUT_FLAG_ANCH_RIGHT);
                inf.a_bottom = inf.y; TRANSFER_MASK(LAYOUT_FLAG_Y, LAYOUT_FLAG_ANCH_BOTTOM);
                break;
            case Alignament::Bottom:        
                inf.a_bottom = inf.y; TRANSFER_MASK(LAYOUT_FLAG_Y, LAYOUT_FLAG_ANCH_BOTTOM); 
                break;
            case Alignament::BottomLeft:    
                inf.a_left = inf.x; TRANSFER_MASK(LAYOUT_FLAG_X, LAYOUT_FLAG_ANCH_LEFT);
                inf.a_bottom = inf.y; TRANSFER_MASK(LAYOUT_FLAG_Y, LAYOUT_FLAG_ANCH_BOTTOM);
                break;                
            case Alignament::Left:          
                inf.a_left = inf.x; TRANSFER_MASK(LAYOUT_FLAG_X, LAYOUT_FLAG_ANCH_LEFT);
                break;
            case Alignament::Center:        break;
        };
    }
    this->Layout.Format.PercentageMask = inf.percentagesMask;
    this->Layout.Format.Width = inf.width;
    this->Layout.Format.Height = inf.height;
    this->Layout.Format.AnchorLeft = inf.a_left;
    this->Layout.Format.AnchorTop = inf.a_top;
    this->Layout.Format.AnchorRight = inf.a_right;
    this->Layout.Format.AnchorBottom = inf.a_bottom;
    switch (anch_flags)
    {
        case 0:
            switch (inf.anchor)
            {
                case Alignament::TopLeft:       this->Layout.Format.LayoutMode = LAYOUT_MODE_TOP_LEFT; break;
                case Alignament::Top:           this->Layout.Format.LayoutMode = LAYOUT_MODE_TOP; break;
                case Alignament::TopRight:      this->Layout.Format.LayoutMode = LAYOUT_MODE_TOP_RIGHT; break;
                case Alignament::Right:         this->Layout.Format.LayoutMode = LAYOUT_MODE_RIGHT; break;
                case Alignament::BottomRight:   this->Layout.Format.LayoutMode = LAYOUT_MODE_BOTTOM_RIGHT; break;
                case Alignament::Bottom:        this->Layout.Format.LayoutMode = LAYOUT_MODE_BOTTOM; break;
                case Alignament::BottomLeft:    this->Layout.Format.LayoutMode = LAYOUT_MODE_BOTTOM_LEFT; break;
                case Alignament::Left:          this->Layout.Format.LayoutMode = LAYOUT_MODE_LEFT; break;
                case Alignament::Center:        this->Layout.Format.LayoutMode = LAYOUT_MODE_CENTER; break;
            };
            break;
        case LAYOUT_FLAG_ANCH_TOP| LAYOUT_FLAG_ANCH_LEFT:
            this->Layout.Format.LayoutMode = LAYOUT_MODE_TOP_LEFT;
            break;
        case LAYOUT_FLAG_ANCH_TOP:
            this->Layout.Format.LayoutMode = LAYOUT_MODE_TOP;
            break;
        case LAYOUT_FLAG_ANCH_TOP | LAYOUT_FLAG_ANCH_RIGHT:
            this->Layout.Format.LayoutMode = LAYOUT_MODE_TOP_RIGHT;
            break;
        case LAYOUT_FLAG_ANCH_RIGHT:
            this->Layout.Format.LayoutMode = LAYOUT_MODE_RIGHT;
            break;
        case LAYOUT_FLAG_ANCH_BOTTOM | LAYOUT_FLAG_ANCH_RIGHT:
            this->Layout.Format.LayoutMode = LAYOUT_MODE_BOTTOM_RIGHT;
            break;
        case LAYOUT_FLAG_ANCH_BOTTOM:
            this->Layout.Format.LayoutMode = LAYOUT_MODE_BOTTOM;
            break;
        case LAYOUT_FLAG_ANCH_BOTTOM | LAYOUT_FLAG_ANCH_LEFT:
            this->Layout.Format.LayoutMode = LAYOUT_MODE_BOTTOM_LEFT;
            break;
        case LAYOUT_FLAG_ANCH_LEFT:
            this->Layout.Format.LayoutMode = LAYOUT_MODE_LEFT;
            break;
        default:
            RETURNERROR(false, "Invalid format: (%s) --> you have either missed or add to many values !");
    }
    return true;
}
bool ControlContext::RecomputeLayout(Control *controlParent)
{
    LayoutMetricData        md;
    AppCUI::Console::Size   sz;

    if (controlParent == nullptr)
        controlParent = this->Parent;
    if (controlParent != nullptr)
    {        
        controlParent->GetClientSize(sz);
    }
    else {
        CHECK(AppCUI::Application::GetDesktopSize(sz), false, "Fail to get desktop size !");
    }
    md.ParentWidth = sz.Width;
    md.ParentHeight = sz.Height;
    md.PercentagesMask = this->Layout.Format.PercentageMask;

    int a_l, a_r, a_t, a_b;
    this->Layout.Width = ComputeXAxesValue(this->Layout.Format.Width, md, LAYOUT_FLAG_WIDTH);
    if (this->Layout.Width < this->Layout.MinWidth)
        this->Layout.Width = this->Layout.MinWidth;
    if (this->Layout.Width > this->Layout.MaxWidth)
        this->Layout.Width = this->Layout.MaxWidth;

    this->Layout.Height = ComputeYAxesValue(this->Layout.Format.Height, md, LAYOUT_FLAG_HEIGHT);
    if (this->Layout.Height < this->Layout.MinHeight)
        this->Layout.Height = this->Layout.MinHeight;
    if (this->Layout.Height > this->Layout.MaxHeight)
        this->Layout.Height = this->Layout.MaxHeight;

    a_l = ComputeXAxesValue(this->Layout.Format.AnchorLeft, md, LAYOUT_FLAG_ANCH_LEFT);
    a_r = ComputeXAxesValue(this->Layout.Format.AnchorRight, md, LAYOUT_FLAG_ANCH_RIGHT);
    a_t = ComputeYAxesValue(this->Layout.Format.AnchorTop, md, LAYOUT_FLAG_ANCH_TOP);
    a_b = ComputeYAxesValue(this->Layout.Format.AnchorBottom, md, LAYOUT_FLAG_ANCH_BOTTOM);

    switch (this->Layout.Format.LayoutMode)
    {
        case LAYOUT_MODE_TOP_LEFT: 
            this->Layout.X = a_l;
            this->Layout.Y = a_t;
            break;
        case LAYOUT_MODE_TOP: 
            this->Layout.X = (md.ParentWidth - this->Layout.Width)/2; 
            this->Layout.Y = a_t;
            break;
        case LAYOUT_MODE_TOP_RIGHT: 
            this->Layout.X = (md.ParentWidth - a_r)-this->Layout.Width; 
            this->Layout.Y = a_t;
            break;
        case LAYOUT_MODE_RIGHT: 
            this->Layout.X = (md.ParentWidth - a_r) - this->Layout.Width; 
            this->Layout.Y = (md.ParentHeight - this->Layout.Height) / 2; 
            break;
        case LAYOUT_MODE_BOTTOM_RIGHT:
            this->Layout.X = (md.ParentWidth - a_r) - this->Layout.Width;
            this->Layout.Y = (md.ParentHeight - a_b) - this->Layout.Height;
            break;            
        case LAYOUT_MODE_BOTTOM: 
            this->Layout.X = (md.ParentWidth - this->Layout.Width) / 2;
            this->Layout.Y = (md.ParentHeight - a_b) - this->Layout.Height;
            break;            
        case LAYOUT_MODE_BOTTOM_LEFT:
            this->Layout.X = a_l;
            this->Layout.Y = (md.ParentHeight - a_b) - this->Layout.Height;
            break;
        case LAYOUT_MODE_LEFT:
            this->Layout.X = a_l;
            this->Layout.Y = (md.ParentHeight - this->Layout.Height) / 2;
            break;
        case LAYOUT_MODE_CENTER: 
            this->Layout.X = (md.ParentWidth - this->Layout.Width) / 2;
            this->Layout.Y = (md.ParentHeight - this->Layout.Height) / 2;
            break;            
        default:
            RETURNERROR(false, "Unknwon mode: %d", this->Layout.Format.LayoutMode);
    }
    return true;
}
void ControlContext::PaintScrollbars(Console::Renderer & renderer)
{
    int x = Layout.Width - 1;
    int y = Layout.Height - 1;    
    if (Flags & GATTR_VSCROLL)
    {
        if (Layout.Height >= (int)(ScrollBars.TopMargin + 2 + MINIM_SCORLL_BAR_LENGTH))
        {
            renderer.DrawVerticalLineWithSpecialChar(x, ScrollBars.TopMargin, y - 2, SpecialChars::Block25, Cfg->ScrollBar.Bar);
            renderer.WriteSpecialCharacter(x, ScrollBars.TopMargin, SpecialChars::TriangleUp, Cfg->ScrollBar.Arrows);
            renderer.WriteSpecialCharacter(x, y-2, SpecialChars::TriangleDown, Cfg->ScrollBar.Arrows);
            if (ScrollBars.MaxVerticalValue)
            {
                unsigned int sz = (unsigned int)(y - (2 + ScrollBars.TopMargin + 2/*two arrows*/));
                unsigned int poz = (unsigned int)((sz * ScrollBars.VerticalValue) / ScrollBars.MaxVerticalValue);
                renderer.WriteSpecialCharacter(x, ScrollBars.TopMargin + 1 + poz, SpecialChars::BlockCentered, Cfg->ScrollBar.Position);
            }
        }                    
    }
    if (Flags & GATTR_HSCROLL)
    {
        if (Layout.Width >= (int)(ScrollBars.LeftMargin + 2 + MINIM_SCORLL_BAR_LENGTH))
        {
            renderer.DrawHorizontalLineWithSpecialChar(ScrollBars.LeftMargin, y, x-2, SpecialChars::Block25, Cfg->ScrollBar.Bar);
            renderer.WriteSpecialCharacter(ScrollBars.LeftMargin, y, SpecialChars::TriangleLeft, Cfg->ScrollBar.Arrows);
            renderer.WriteSpecialCharacter(x - 2, y, SpecialChars::TriangleRight, Cfg->ScrollBar.Arrows);
            if (ScrollBars.MaxHorizontalValue)
            {
                unsigned int sz = (unsigned int)(x - (2 + ScrollBars.LeftMargin + 2/*two arrows*/));
                unsigned int poz = (unsigned int)((sz * ScrollBars.HorizontalValue) / ScrollBars.MaxHorizontalValue);
                renderer.WriteSpecialCharacter(ScrollBars.LeftMargin + 1 + poz, y, SpecialChars::BlockCentered, Cfg->ScrollBar.Position);
            }
        }
    }
}
//=======================================================================================================================================================
AppCUI::Controls::Control::Control()
{
	Context = nullptr;
}
AppCUI::Controls::Control::~Control()
{
	DELETE_CONTROL_CONTEXT(ControlContext);
}
bool AppCUI::Controls::Control::Init(Control *parent, const char *text, const char * layout, bool computeHotKey)
{
	CHECK(text != nullptr, false, "text parameter must not be nullptr !");
    AppCUI::Application::Config * cfg = AppCUI::Application::GetAppConfig();
    CHECK(cfg != nullptr, false, "Unable to get config object !");
    CHECK(CTRLC->UpdateLayoutFormat(layout), false, "Invalid format !");
    CHECK(CTRLC->RecomputeLayout(parent), false, "Unable to recompute layout !");

	CTRLC->Text.Set(text);

    if (computeHotKey)
    {        
        if (CTRLC->Text.SetWithHotKey(text, CTRLC->HotKeyOffset))
        {
            this->SetHotKey(text[CTRLC->HotKeyOffset + 1]);
        }
    }
    else {
        CTRLC->Text.Set(text);
    }
    CTRLC->Inited = true;
    // 

	if (parent != nullptr)
	{
		CHECK(parent->AddControl(this), false, "Unable to add control !");
	}		
	
	return true;
}
bool AppCUI::Controls::Control::AddControl(Control *ctrl)
{
	CHECK(ctrl != nullptr, false, "Invalid control (nullptr)");
    CHECK(ctrl->IsInitialized(), false, "Control was not initialized before adding it to a parent control !");
	//LOG_INFO("AddControl(this=%p,Control=%p,Controls List=%p,Count=%d", this, ctrl, Controls, ControlsCount);
	if (CTRLC->Controls == nullptr) // first time
	{
		//LOG_INFO("Allocate 8 controls for current container %p", this);
		CTRLC->Controls = new Control*[8];
		CHECK(CTRLC->Controls != nullptr, false, "");
		CTRLC->ControlsCount = 0;
	}
	else {
		if (((CTRLC->ControlsCount % 8) == 0) && (CTRLC->ControlsCount>0))
		{
			// grow
			Control	**tmp = new Control*[CTRLC->ControlsCount + 8];
			CHECK(tmp != nullptr, false, "");
			for (unsigned int tr = 0; tr<CTRLC->ControlsCount; tr++)
				tmp[tr] = CTRLC->Controls[tr];
			delete CTRLC->Controls;
			CTRLC->Controls = tmp;
		}
	}
	if (OnBeforeAddControl(ctrl) == false)
		return false;
	// fac linkul
	Control* oldParent = ((ControlContext*)(ctrl->Context))->Parent;
	CTRLC->Controls[CTRLC->ControlsCount++] = ctrl;
	((ControlContext*)(ctrl->Context))->Parent = this;
	OnAfterAddControl(ctrl);
	return true;
}
bool AppCUI::Controls::Control::RemoveControl(Control * control)
{
	unsigned int index;
	if (GetChildIndex(control, index) == false)
		return false;
	return RemoveControl(index);
}
bool AppCUI::Controls::Control::RemoveControl(unsigned int index)
{
	CHECK(index < CTRLC->ControlsCount, false, "Invalid index for control child: %d (should be between 0 and %d)", index, CTRLC->ControlsCount - 1);
	Control** lst = CTRLC->Controls;
	CHECK(lst != nullptr, false, "Expecting a non-nullptr list of control !");
	unsigned int count = CTRLC->ControlsCount;
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
int  AppCUI::Controls::Control::GetX()
{
	return CTRLC->Layout.X;
}
int  AppCUI::Controls::Control::GetY()
{
	return CTRLC->Layout.Y;
}
int  AppCUI::Controls::Control::GetWidth()
{
	return CTRLC->Layout.Width;
}
int	 AppCUI::Controls::Control::GetHeight()
{
	return CTRLC->Layout.Height;
}
void AppCUI::Controls::Control::GetSize(AppCUI::Console::Size & size)
{
    size.Width = CTRLC->Layout.Width;
    size.Height = CTRLC->Layout.Height;    
}
void AppCUI::Controls::Control::GetClientSize(AppCUI::Console::Size & size)
{
    int w = CTRLC->Layout.Width - (CTRLC->Margins.Left + CTRLC->Margins.Right);
    int h = CTRLC->Layout.Height - (CTRLC->Margins.Top + CTRLC->Margins.Bottom);
    if (w < 0)
        w = 0;
    if (h <= 0)
        h = 0;
    size.Width = w;
    size.Height = h;
}
bool AppCUI::Controls::Control::IsMouseInControl(int x, int y)
{
	return (x >= 0) && (y >= 0) && (x < (CTRLC->Layout.Width)) && (y < (CTRLC->Layout.Height));
}
void AppCUI::Controls::Control::SetChecked(bool value)
{
	if (value)
		CTRLC->Flags |= GATTR_CHECKED;
	else
		CTRLC->Flags -= ((CTRLC->Flags) & GATTR_CHECKED);
}
void AppCUI::Controls::Control::SetEnabled(bool value)
{
	if (value)
		CTRLC->Flags |= GATTR_ENABLE;
	else
		CTRLC->Flags -= ((CTRLC->Flags) & GATTR_ENABLE);
}
void AppCUI::Controls::Control::SetVisible(bool value)
{
	if (value)
		CTRLC->Flags |= GATTR_VISIBLE;
	else
		CTRLC->Flags -= ((CTRLC->Flags) & GATTR_VISIBLE);
}

bool AppCUI::Controls::Control::IsChecked()
{
	return (((CTRLC->Flags) & GATTR_CHECKED) != 0);
}
bool AppCUI::Controls::Control::IsEnabled()
{
	return (((CTRLC->Flags) & GATTR_ENABLE) != 0);
}
bool AppCUI::Controls::Control::IsVisible()
{
	return (((CTRLC->Flags) & GATTR_VISIBLE) != 0);
}
bool AppCUI::Controls::Control::HasFocus()
{
	return CTRLC->Focused;
}
bool AppCUI::Controls::Control::IsMouseOver()
{
    return CTRLC->MouseIsOver;
}


Control*     AppCUI::Controls::Control::GetParent()
{
	return CTRLC->Parent;
}
Control**	 AppCUI::Controls::Control::GetChildrenList()
{
	return CTRLC->Controls;
}
Control*	 AppCUI::Controls::Control::GetChild(unsigned int index)
{
	CHECK(index < CTRLC->ControlsCount, nullptr, "Invalid index for control child: %d (should be between 0 and %d)", index, CTRLC->ControlsCount - 1);
	return CTRLC->Controls[index];
}
unsigned int AppCUI::Controls::Control::GetChildernCount()
{
	return CTRLC->ControlsCount;
}

bool AppCUI::Controls::Control::GetChildIndex(Control *control, unsigned int &index)
{
	Control** lst = CTRLC->Controls;
	Control** end = lst + (CTRLC->ControlsCount);
	if (lst == nullptr)
		return false;
	unsigned int c_index = 0;
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


void AppCUI::Controls::Control::MoveTo(int newX, int newY)
{
    AppCUI::Application::Config * cfg = AppCUI::Application::GetAppConfig();
    if (!cfg)
        return;
	if ((newX == CTRLC->Layout.X) && (newY == CTRLC->Layout.Y))
		return;
	CTRLC->Layout.X = newX;
	CTRLC->Layout.Y = newY;
	AppCUI::Application::RecomputeControlsLayout();
    AppCUI::Application::Repaint();
	if (CTRLC->Handlers.OnAfterMoveHandler != nullptr)
		CTRLC->Handlers.OnAfterMoveHandler(this, CTRLC->Layout.X, CTRLC->Layout.Y, CTRLC->Handlers.OnAfterMoveHandlerContext);
}
bool AppCUI::Controls::Control::Resize(int newWidth, int newHeight)
{
    AppCUI::Application::Config * cfg = AppCUI::Application::GetAppConfig();
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
	if (newWidth < 1) newWidth = 1;
	if (newHeight < 1) newHeight = 1;
	if (CTRLC->Handlers.OnBeforeResizeHandler != nullptr)
	{
		if (CTRLC->Handlers.OnBeforeResizeHandler(this, newWidth, newHeight, CTRLC->Handlers.OnBeforeResizeHandlerContext) == false)
			return false;
	}
	else {
		if (OnBeforeResize(newWidth, newHeight) == false)
			return false;
	}
	CTRLC->Layout.Width = newWidth;
	CTRLC->Layout.Height = newHeight;
	RecomputeLayout();
	return true;
}
void AppCUI::Controls::Control::RecomputeLayout()
{
    for (unsigned int tr = 0; tr < CTRLC->ControlsCount; tr++)
    {
        ((ControlContext*)(CTRLC->Controls[tr]->Context))->RecomputeLayout(this);
        CTRLC->Controls[tr]->RecomputeLayout();
    }
	OnAfterResize(CTRLC->Layout.Width, CTRLC->Layout.Height);
	if (CTRLC->Handlers.OnAfterResizeHandler != nullptr)
		CTRLC->Handlers.OnAfterResizeHandler(this, CTRLC->Layout.Width, CTRLC->Layout.Height, CTRLC->Handlers.OnAfterResizeHandlerContext);

    AppCUI::Application::RecomputeControlsLayout();
}
bool AppCUI::Controls::Control::SetText(const char * text, bool updateHotKey)
{
	if (OnBeforeSetText(text) == false)
		return false;
    if (updateHotKey)
    {
        if (CTRLC->Text.SetWithHotKey(text,CTRLC->HotKeyOffset) == false)
            return false;
    } else {
        if (CTRLC->Text.Set(text) == false)
            return false;
    }
	OnAfterSetText(text);
	return true;
}
bool AppCUI::Controls::Control::SetText(AppCUI::Utils::String *text, bool updateHotKey)
{
	if (text == nullptr)
		return false;
	if (OnBeforeSetText(text->GetText()) == false)
		return false;

    if (updateHotKey)
    {
        if (CTRLC->Text.SetWithHotKey(text->GetText(),CTRLC->HotKeyOffset,NoColorPair,text->Len()) == false)
            return false;
    } else {
        if (CTRLC->Text.Set(text->GetText(), NoColorPair, text->Len()) == false)
            return false;
    }

	OnAfterSetText(text->GetText());
	return true;
}
bool AppCUI::Controls::Control::SetText(AppCUI::Utils::String &text, bool updateHotKey)
{
	if (OnBeforeSetText(text.GetText()) == false)
		return false;
    if (updateHotKey)
    {
        if (CTRLC->Text.SetWithHotKey(text.GetText(),CTRLC->HotKeyOffset, NoColorPair,text.Len()) == false)
            return false;
    } else {
        if (CTRLC->Text.Set(text.GetText(), NoColorPair, text.Len()) == false)
            return false;
    }
	OnAfterSetText(text.GetText());
	return true;
}
void AppCUI::Controls::Control::UpdateHScrollBar(unsigned long long value, unsigned long long maxValue)
{
    if (value > maxValue)
        value = maxValue;
    CTRLC->ScrollBars.HorizontalValue = value;
    CTRLC->ScrollBars.MaxHorizontalValue = maxValue;
}
void AppCUI::Controls::Control::UpdateVScrollBar(unsigned long long value, unsigned long long maxValue)
{
    if (value > maxValue)
        value = maxValue;
    CTRLC->ScrollBars.VerticalValue = value;
    CTRLC->ScrollBars.MaxVerticalValue = maxValue;
}
int	 AppCUI::Controls::Control::GetGroup()
{
	return CTRLC->GroupID;
}
void AppCUI::Controls::Control::SetGroup(int newGroupID)
{
	if (newGroupID >= 0)
		CTRLC->GroupID = newGroupID;
}
void AppCUI::Controls::Control::ClearGroup()
{
	CTRLC->GroupID = 0;
}

bool AppCUI::Controls::Control::SetHotKey(char hotKey)
{
	hotKey |= 0x20;
	CHECK((((hotKey >= 'a') && (hotKey <= 'z')) || ((hotKey >= '0') && (hotKey <= '9'))), false, "Invalid hot key - accepted values are ['A'-'Z'] and ['0'-'9']");
	if ((hotKey >= 'a') && (hotKey <= 'z'))
	{
		CTRLC->HotKey = (Key::Type)(Key::Alt | ((unsigned int)Key::A + (hotKey - 'a')));
		return true;
	}
	if ((hotKey >= '0') && (hotKey <= '9'))
	{
		CTRLC->HotKey = (Key::Type)(Key::Alt | ((unsigned int)Key::N0 + (hotKey - '0')));
		return true;
	}
    return false;
}
bool AppCUI::Controls::Control::SetMargins(int left, int top, int right, int bottom)
{
	CHECK(left >= 0, false, "left margin must be positive values !");
    CHECK(top >= 0, false, "top margin must be positive values !");
    CHECK(right >= 0, false, "right margin must be positive values !");
    CHECK(bottom >= 0, false, "bottom margin must be positive values !");
    CTRLC->Margins.Left = left;
    CTRLC->Margins.Right = right;
    CTRLC->Margins.Top = top;
    CTRLC->Margins.Bottom = bottom;
	return true;
}
AppCUI::Input::Key::Type	 AppCUI::Controls::Control::GetHotKey()
{
	return CTRLC->HotKey;
}
void AppCUI::Controls::Control::ClearHotKey()
{
    CTRLC->HotKey = Key::None;
    CTRLC->HotKeyOffset = 0xFFFFFFFF;
}
void AppCUI::Controls::Control::SetControlID(int newID)
{
	if (newID > 0)
		CTRLC->ControlID = newID;
	else
		CTRLC->ControlID = 0;
}
bool AppCUI::Controls::Control::SetFocus()
{
	Control			*obj = this;
	Control			*p;
	unsigned int	tr,count;

	/*if (CTRLC->Focused)
		return true;*/
	// verifica daca pot sa dau focus
	while (obj != nullptr)
	{
		if (((((ControlContext*)(obj->Context))->Flags) & (GATTR_ENABLE | GATTR_VISIBLE)) != (GATTR_ENABLE | GATTR_VISIBLE))
			return false;
		obj = ((ControlContext*)(obj->Context))->Parent;
	}
	// setez focusul la toti
	obj = this;
	p = ((ControlContext*)(obj->Context))->Parent;

	while ((obj != nullptr) && (p != nullptr))
	{
		// caut obiectul
		((ControlContext*)(p->Context))->CurrentControlIndex = -1;
		count = ((ControlContext*)(p->Context))->ControlsCount;
		for (tr = 0; tr < count; tr++)
		{
			if (((ControlContext*)(p->Context))->Controls[tr] == obj)
			{
				((ControlContext*)(p->Context))->CurrentControlIndex = tr;
				break;
			}
		}
		if (((ControlContext*)(p->Context))->CurrentControlIndex == -1)
			return false;
		obj = p;
		p = ((ControlContext*)(p->Context))->Parent;
	}
    AppCUI::Application::Repaint();
	// acceleratorii
	//UpdateCommandBar(this);
	return true;
}
void AppCUI::Controls::Control::RaiseEvent(Event::Type eventType)
{
	AppCUI::Application::RaiseEvent(this, this, eventType, CTRLC->ControlID);
}
void AppCUI::Controls::Control::RaiseEvent(Event::Type eventType, int ID)
{
    AppCUI::Application::RaiseEvent(this, this, eventType, ID);
}
void AppCUI::Controls::Control::Paint(Console::Renderer & renderer)
{
}
bool AppCUI::Controls::Control::IsInitialized()
{
    CHECK(this->Context, false, "Control context was not initialized !");
	return CTRLC->Inited;
}
// Evenimente
bool AppCUI::Controls::Control::OnKeyEvent(AppCUI::Input::Key::Type keyCode, char AsciiCode)
{
	return false;
}
void AppCUI::Controls::Control::OnFocus()
{
}
void AppCUI::Controls::Control::OnLoseFocus()
{
}
void AppCUI::Controls::Control::OnMouseReleased(int x, int y, int Button)
{
}
void AppCUI::Controls::Control::OnMousePressed(int x, int y, int Button)
{
}
bool AppCUI::Controls::Control::OnMouseDrag(int x, int y, int Button)
{
	return false;
}
bool AppCUI::Controls::Control::OnMouseOver(int x, int y)
{
    return false;
}
bool AppCUI::Controls::Control::OnMouseEnter()
{
    return false;
}
bool AppCUI::Controls::Control::OnMouseLeave()
{
    return false;
}
void AppCUI::Controls::Control::OnMouseWheel(int direction)
{
}
void AppCUI::Controls::Control::OnHotKey()
{
}
bool AppCUI::Controls::Control::OnEvent(const void* sender, Event::Type eventType, int controlID)
{
	return false;
}
bool AppCUI::Controls::Control::OnUpdateCommandBar(AppCUI::Application::CommandBar & commandBar)
{
	return false;
}
bool AppCUI::Controls::Control::OnBeforeResize(int newWidth,int newHeight)
{
	return true;
}
void AppCUI::Controls::Control::OnAfterResize(int newWidth, int newHeight)
{
}
bool AppCUI::Controls::Control::OnBeforeAddControl(AppCUI::Controls::Control *ctrl)
{
	return (ctrl != nullptr);
}
void AppCUI::Controls::Control::OnAfterAddControl(AppCUI::Controls::Control *ctrl)
{
	// daca e primul - setez si tab-ul
	if (CTRLC->ControlsCount == 1)
		CTRLC->CurrentControlIndex = 0;
}
bool AppCUI::Controls::Control::OnBeforeSetText(const char * text)
{
	return true; 
}
void AppCUI::Controls::Control::OnAfterSetText(const char * text)
{
}
void AppCUI::Controls::Control::OnUpdateScrollBars()
{
}
void AppCUI::Controls::Control::SetOnBeforeResizeHandler(Handlers::BeforeResizeHandler handler, void *objContext)
{
	CTRLC->Handlers.OnBeforeResizeHandler = handler;
	CTRLC->Handlers.OnBeforeResizeHandlerContext = objContext;
}
void AppCUI::Controls::Control::SetOnAfterResizeHandler(Handlers::AfterResizeHandler handler, void *objContext)
{
	CTRLC->Handlers.OnAfterResizeHandler = handler;
	CTRLC->Handlers.OnAfterResizeHandlerContext = objContext;
}
void AppCUI::Controls::Control::SetOnAfterMoveHandler(Handlers::AfterMoveHandler handler, void *objContext)
{
	CTRLC->Handlers.OnAfterMoveHandler = handler;
	CTRLC->Handlers.OnAfterMoveHandlerContext = objContext;
}
void AppCUI::Controls::Control::SetOnUpdateCommandBarHandler(Handlers::UpdateCommandBarHandler handler, void* objContext)
{
	CTRLC->Handlers.OnUpdateCommandBarHandler = handler;
	CTRLC->Handlers.OnUpdateCommandBarHandlerContext = objContext;
}
void AppCUI::Controls::Control::SetOnKeyEventHandler(Handlers::KeyEventHandler handler, void* objContext)
{
	CTRLC->Handlers.OnKeyEventHandler = handler;
	CTRLC->Handlers.OnKeyEventHandlerContext = objContext;
}
void AppCUI::Controls::Control::SetPaintHandler(Handlers::PaintHandler handler, void* objContext)
{
	CTRLC->Handlers.OnPaintHandler = handler;
	CTRLC->Handlers.OnPaintHandlerContext = objContext;
}
void AppCUI::Controls::Control::SetOnFocusHandler(Handlers::PaintHandler handler, void* objContext)
{
	CTRLC->Handlers.OnFocusHandler = handler;
	CTRLC->Handlers.OnFocusHandlerContext = objContext;
}
void AppCUI::Controls::Control::SetOnLoseFocusHandler(Handlers::PaintHandler handler, void* objContext)
{
    CTRLC->Handlers.OnLoseFocusHandler = handler;
    CTRLC->Handlers.OnLoseFocusHandlerContext = objContext;
}
void AppCUI::Controls::Control::SetEventHandler(Handlers::EventHandler handler, void* objContext)
{
	CTRLC->Handlers.OnEventHandler = handler;
	CTRLC->Handlers.OnEventHandlerContext = objContext;
}
void AppCUI::Controls::Control::SetMousePressedHandler(Handlers::MousePressedHandler handler, void* objContext)
{
	CTRLC->Handlers.OnMousePressedHandler = handler;
	CTRLC->Handlers.OnMousePressedHandlerContext = objContext;
}
void AppCUI::Controls::Control::SetMouseReleasedHandler(Handlers::MouseReleasedHandler handler, void* objContext)
{
	CTRLC->Handlers.OnMouseReleasedHandler = handler;
	CTRLC->Handlers.OnMouseReleasedHandlerContext = objContext;
}
void AppCUI::Controls::Control::SetMouseHandler(Handlers::MousePressedHandler mousePressedHandler, Handlers::MouseReleasedHandler mouseReleasedHandler, void *objContext)
{
	SetMousePressedHandler(mousePressedHandler, objContext);
	SetMouseReleasedHandler(mouseReleasedHandler, objContext);
}
