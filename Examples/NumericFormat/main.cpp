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
    return 0;
}
