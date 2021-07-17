#include "AppCUI.h"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Console;


#define BUTTON_COMPUTE     5678

class MyWin : public AppCUI::Controls::Window
{
    Button but;
    Label inf;
public:
    MyWin()
    {
        this->Create("Progress status", "a:c,w:60,h:20");
        but.Create(this, "&Compute", "x:1,y:3,w:14", BUTTON_COMPUTE);
        inf.Create(this, "Pressing 'Compute' button will compute the first 1000 prime numbers", "x:1,y:1,w:56,h:2");

    }
    bool IsPrim(unsigned long long value)
    {
        for (unsigned long long tr = 0; tr < value / 2; tr++)
            if ((value % tr) == 0)
                return false;
        return true;
    }
    void Compute()
    {
        ProgressStatus::Init("Compute first 1000 prime numbers", 1000);
        unsigned long long value = 2;
        unsigned long long count = 0;
        while (count < 1000)
        {
            if (IsPrim(value))
                count++;
            value++;
            if (ProgressStatus::Update(count, "computing ..."))
                break;

        }
    }
    bool OnEvent(const void* sender, Event::Type eventType, int controlID) override
    {
        if (eventType == Event::EVENT_WINDOW_CLOSE)
        {
            Application::Close();
            return true;
        }
        if (eventType == Event::EVENT_BUTTON_CLICKED)
        {
            switch (controlID)
            {
                case BUTTON_COMPUTE: Compute(); break;
            }
            return true;
        }
        return false;
    }
};

int main()
{
    Application::Init();
    Application::AddWindow(new MyWin());
    Application::Run();
    return 0;
}
