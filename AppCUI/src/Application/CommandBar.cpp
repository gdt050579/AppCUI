#include "Internal.hpp"

namespace AppCUI::Application
{
CommandBar::CommandBar()
{
    this->Controller = nullptr;
}
void CommandBar::Init(void* _controller)
{
    if ((this->Controller == nullptr) && (_controller != nullptr))
        this->Controller = _controller;
}
bool CommandBar::SetCommand(Input::Key keyCode, const ConstString& caption, int CommandID)
{
    CHECK(Controller, false, "Command bar controller has not been initialized !");
    return ((Internal::CommandBarController*) this->Controller)->Set(keyCode, caption, CommandID);
}
bool CommandBar::SetCommand(const Input::KeyBinding& binding, int CommandID)
{
    if (binding.Key == Input::Key::None)
        return false; // unassigned -> not shown
    CHECK(binding.Caption, false, "Expecting a valid caption for the key binding !");
    return SetCommand(binding.Key, binding.Caption, CommandID);
}
} // namespace AppCUI::Application
