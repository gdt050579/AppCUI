#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <cstdlib>
#include <stdio.h>
#include <array>

#include "os.h"
#include "Internal.h"

/*
    Temporary solution for cases with missing ncurses
    This will compile but it will crash, since OnInit isn't implemented properly
*/

using namespace AppCUI::Internal;

Console::Console()
{
    std::cout << "[+] I am a default console and I'll crash now" << std::endl;
    std::cout << "[+] If you see me, you probably don't have ncurses on your system" << std::endl;
    std::cout << "[+] The build worked for CI and legacy purposes" << std::endl;
    std::cout << std::endl;
    std::cout << "[+] So...what do?" << std::endl;
    std::cout << "[+] Try telling cmake the path to your ncurses" << std::endl;
    std::cout << "[+] This is done by setting the -DCMAKE_PREFIX_PATH during configure" << std::endl;
    std::cout << "[+] For example: cmake -DCMAKE_PREFIX_PATH=/opt/homebrew/Cellar/ncurses/6.2" << std::endl;
    std::cout << "[+] Also rm -rf the build folder and reconfigure again to remove the cache" << std::endl;
    std::cout << std::endl;
}

bool Console::OnInit()
{
    return true;
}

void Console::OnUninit() 
{

}

void Console::OnFlushToScreen() 
{
    std::cout << "I don't have ncurses!" << std::endl;
}


bool Console::OnUpdateCursor()
{
    return true;
}

Console::~Console() 
{

}
