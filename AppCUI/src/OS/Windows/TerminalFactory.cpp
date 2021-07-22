#include "os.h"
#include <string.h>

using namespace AppCUI::Internal;

AbstractTerminal* AbstractTerminal::Create(const InitializationData & initData)
{
    AbstractTerminal * term = nullptr;
    switch (initData.FrontEnd)
    {
        case TerminalType::Default:
        case TerminalType::Windows:
            term = new WindowsTerminal();
            break;
        default:
            RETURNERROR(nullptr, "Unsuported terminal type for Windows OS (%d)", (unsigned int)initData.FrontEnd);
    }
    CHECK(term, false, "Fail to allocate memory for a terminal !");
    if (term->Init(initData) == false)
    {
        delete term;
        RETURNERROR(nullptr, "Fail to initialize the terminal !");
    }
    return term;
}
