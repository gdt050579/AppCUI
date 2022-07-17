#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

class SimpleWin : public Window
{
  public:
    SimpleWin() : Window("Test", "d:c,w:40,h:10", WindowFlags::None)
    {
        Factory::CheckBox::Create(this, "A checkbox", "x:1,y:1,w:20");
        Factory::TextField::Create(this, "", "x:1,y:3,w:30");
        Factory::TextField::Create(this, "", "x:1,y:5,w:30");
    }
};
bool ValidateMethod(std::string_view script)
{
    CHECK(Application::InitForTests(60, 20, InitializationFlags::None, true), false, "");
    Application::AddWindow(std::make_unique<SimpleWin>());
    return Application::RunTestScript(script);
}
int main()
{
    auto result = ValidateMethod(R"SCRIPT(
            Print()
            # click on the checkbox
            Mouse.Click(14,7,Left)
            # move to the next control
            Key.Press(Tab)
            # type something in the text field
            Key.Type(Hello)
            Print()
            ValidateScreenHash(0x49bc5622afba79e6, false)
            # copy last two characters
            Key.PressMultipleTimes(Shift+Left,2)
            Key.Press(Ctrl+Insert)
            Key.Press(Tab)
            # now we are at the second text field --> lets paste the data
            Key.Press(Shift+Insert)
            Print()  
            ValidateScreenHash(0xb627729d103bcc1d, false)     
            # lets move the window
            Mouse.Drag(20,5,18,3)
            Print()  
            PrintScreenHash(false)
            ValidateScreenHash(0xe3e92e8b72e49439, false)     
    )SCRIPT");
    if (result)
    {
        LOG_INFO("Script was validated !")
    }
    else
    {
        LOG_INFO("Errors validating testing script");
    }
    return 0;
}
