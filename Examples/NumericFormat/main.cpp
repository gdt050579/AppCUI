#include "AppCUI.hpp"
#include <iostream>

using namespace AppCUI;
using namespace AppCUI::Utils;

#define VALIDATE(x, y)                                                                                                 \
    if (x != y)                                                                                                        \
    {                                                                                                                  \
        std::cout << "Fail: " << #x << " -> expected: " << y << "\n";                                                  \
    }                                                                                                                  \
    else                                                                                                               \
    {                                                                                                                  \
        std::cout << "OK  : " << #x << "=>" << y << "\n";                                                              \
    }

int main()
{
    NumericFormatter n;
    VALIDATE(n.ToHex(100), "64");
    VALIDATE(n.ToHex(0xFFFFFFFF), "FFFFFFFF");
    VALIDATE(n.ToDec(12345U), "12345");
    VALIDATE(n.ToDec(98765), "98765");
    VALIDATE(n.ToDec(-1234567), "-1234567");
    VALIDATE(n.ToBin(0), "0");
    VALIDATE(n.ToBin(255), "11111111");
    VALIDATE(n.ToBin(24), "11000");
    VALIDATE(n.ToBin(0xFFFFFFFF), "11111111111111111111111111111111");
    VALIDATE(n.ToOct(5), "5");
    VALIDATE(n.ToOct(1234567), "4553207");
    VALIDATE(n.ToBase(130, 2), "10000010");
    VALIDATE(n.ToBase(65535, 16), "FFFF");
    VALIDATE(n.ToBase(123456789, 10), "123456789");
    VALIDATE(n.ToBase(-123456789, 10), "-123456789");

    VALIDATE(n.ToString(12345, { NumericFormatFlags::HexSuffix, 16 }), "3039h");
    VALIDATE(n.ToString(12345, { NumericFormatFlags::HexPrefix, 16 }), "0x3039");
    VALIDATE(n.ToString(12345, { NumericFormatFlags::HexPrefix, 16, 2, ':' }), "0x30:39");
    VALIDATE(n.ToString(12345, { NumericFormatFlags::HexPrefix, 16, 2, ':', 8 }), "0x00:00:30:39");
    VALIDATE(n.ToString(12345, { NumericFormatFlags::None, 2, 4, '-' }), "11-0000-0011-1001");
    VALIDATE(n.ToString(12345, { NumericFormatFlags::None, 2, 4, '-', 20 }), "0000-0011-0000-0011-1001");
    VALIDATE(n.ToString(12345, { NumericFormatFlags::None, 10, 3, ',' }), "12,345");
    VALIDATE(n.ToString(999, { NumericFormatFlags::None, 10, 3, ',' }), "999");
    VALIDATE(n.ToString(1234567890, { NumericFormatFlags::PlusSign, 10, 3, ',' }), "+1,234,567,890");
    VALIDATE(n.ToString(12, { NumericFormatFlags::PlusSign, 10, 3, '_', 6 }), "+000_012");
    VALIDATE(
          n.ToString(0xFFFFFFFFFFFFFFFFULL, { NumericFormatFlags::None, 2, 4, '-' }),
          "1111-1111-1111-1111-1111-1111-1111-1111-1111-1111-1111-1111-1111-1111-1111-1111");
    VALIDATE(n.ToString(-12, { NumericFormatFlags::None, 10}), "-12");
    // for (int base = 2;base<=16;base++)
    //{
    //    auto res = n.ToBase(0xFFFFFFFFFFFFFFFFULL, base);
    //    std::cout << res.length() << "/*" << base << "*/, ";
    //}

    return 0;
}
