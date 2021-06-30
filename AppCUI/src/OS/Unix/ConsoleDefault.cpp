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


bool Console::OnInit()
{

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
