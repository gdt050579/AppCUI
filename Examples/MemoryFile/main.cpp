#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::OS;
using namespace AppCUI::Controls;

#include <iostream>

#define LOG(op) std::cout << #op << " = " << op << std::endl;
int main()
{
    MemoryFile m;
    LOG(m.Create(40));
    LOG(m.Write("Some text"));
    LOG(m.GetSize());
    LOG(m.Write("123456789012345678901234567890"));
    LOG(m.Write("123456789012345678901234567890"));
    LOG(m.Write("123456789012345678901234567890"));
    LOG(m.Write("123456789012345678901234567890"));
    LOG(m.GetSize());
    char temp[128];
    uint32 read;
    LOG(m.Read(10, temp, 3, read));
    temp[3] = 0;
    std::cout << temp << " " << read << std::endl;
    m.Close();
    return 0;
}
