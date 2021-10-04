#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

#define RADIO_GROUP_ID_1 10 // any positive number will do as long as
#define RADIO_GROUP_ID_2 22 // every group has differnt IDs

class MyWin : public Window
{
  public:
    MyWin() : Window("Radiobox example", "d:c,w:60,h:8", WindowFlags::None)
    {
        Factory::RadioBox::Create(this, "&TCP connection", "x:1,y:1,w:15", RADIO_GROUP_ID_1);
        Factory::RadioBox::Create(this, "&UDP connection", "x:1,y:2,w:15", RADIO_GROUP_ID_1);
        Factory::RadioBox::Create(this, "&HTTP connection", "x:1,y:3,w:15", RADIO_GROUP_ID_1);

        Factory::RadioBox::Create(this, u8"Sum (∑) &1", "x:30,y:1,w:15", RADIO_GROUP_ID_2);
        Factory::RadioBox::Create(this, u8"Math symbol (∫) &2", "x:30,y:2,w:25", RADIO_GROUP_ID_2);
        Factory::RadioBox::Create(this, "&3rd option with\nmultiple lines", "x:30,y:3,w:20,h:2", RADIO_GROUP_ID_2);
    }
};
int main()
{
    if (!Application::Init())
        return 1;
    Application::AddWindow(std::make_unique<MyWin>());
    Application::Run();
    return 0;
}
