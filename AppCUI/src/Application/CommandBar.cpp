#include "AppCUI.h"
#include "Internal.h"

using namespace AppCUI::Application;

CommandBar::CommandBar()
{
    this->Controller = nullptr;
}
void CommandBar::Init(void * _controller)
{
    if ((this->Controller == nullptr) && (_controller != nullptr))
        this->Controller = _controller;
}
bool CommandBar::SetCommand(AppCUI::Input::Key keyCode, const char* Name, int CommandID)
{
    CHECK(Controller, false, "Command bar controller has not been initialized !");
    return ((AppCUI::Internal::CommandBarController*)this->Controller)->Set(keyCode, Name, CommandID);
}