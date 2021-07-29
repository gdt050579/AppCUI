#include <stdio.h>
#include <iostream>
#include <term.h>
/*
    https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
*/
int main()
{
    std::cout << "\033[1mthis is going to be bold, "
              << "\033[0m" << std::endl // Reset formatting
              << "\033[3mbut this will be italic, "
              << "\033[0m" << std::endl
              << "\033[9mthis text should be strikethrough"
              << "\033[0m" << std::endl
              << "\033[31m\033[47mthis text should have red foreground and white background"
              << "\033[0m" << std::endl
              << "this text will not be formatted";

    return 0;
}