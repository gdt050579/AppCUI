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
    
    return 0;
}
