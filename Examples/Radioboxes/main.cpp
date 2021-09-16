#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

#define RADIO_GROUP_ID_1 10 // any positive number will do as long as
#define RADIO_GROUP_ID_2 22 // every group has differnt IDs

class MyWin : public AppCUI::Controls::Window
{
    RadioBox r1, r2, r3;
    RadioBox r4, r5, r6;

  public:
    MyWin()
    {
        this->Create("Radiobox example", "d:c,w:60,h:8");
        r1.Create(this, "&TCP connection", "x:1,y:1,w:15", RADIO_GROUP_ID_1);
        r2.Create(this, "&UDP connection", "x:1,y:2,w:15", RADIO_GROUP_ID_1);
        r3.Create(this, "&HTTP connection", "x:1,y:3,w:15", RADIO_GROUP_ID_1);

        r4.Create(this, u8"Sum (∑) &1", "x:30,y:1,w:15", RADIO_GROUP_ID_2);
        r5.Create(this, u8"Math symbol (∫) &2", "x:30,y:2,w:25", RADIO_GROUP_ID_2);
        r6.Create(this, "&3rd option with\nmultiple lines", "x:30,y:3,w:20,h:2", RADIO_GROUP_ID_2);
    }
    bool OnEvent(Control*, Event eventType, int) override
    {
        if (eventType == Event::WindowClose)
        {
            Application::Close();
            return true;
        }
        return false;
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
