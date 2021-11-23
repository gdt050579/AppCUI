#include "AppCUI.hpp"

#include <iostream>

int main()
{
    AppCUI::Log::ErrorList el;
    el.AddError("Height is bigger than %d", 10);
    el.AddError("Name '%s' was not found in the database !", "John");
    std::cout << "Errors: " << el.GetErrorsCount() << " Warning:" << el.GetWarningsCount() << std::endl;
    for (auto i = 0U; i < el.GetErrorsCount(); i++)
        std::cout << "  #" << (i + 1) << " - " << el.GetError(i) << std::endl;

    return 0;
}
