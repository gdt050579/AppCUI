#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;

#define BUTTON_COMPUTE_ODD   5678
#define BUTTON_COMPUTE_PRIME 5679

class MyWin : public AppCUI::Controls::Window
{
  public:
    MyWin() : Window("Progress status", "d:c,w:70,h:9", WindowFlags::None)
    {
        Factory::Button::Create(this, "Compute", "r:1,t:1,w:14", BUTTON_COMPUTE_ODD);
        Factory::Label::Create(
              this,
              "Compute how many odd numbers are between\n1 and 100.000 in a verry inneficient way ...",
              "x:1,y:1,w:50,h:2");

        Factory::Button::Create(this, "Compute", "r:1,t:4,w:14", BUTTON_COMPUTE_PRIME);
        Factory::Label::Create(
              this,
              "Compute the 10000 prime number using a slow method\n(check if there are no divisors)",
              "x:1,y:4,w:50,h:2");
    }
    bool IsPrime(uint64 value)
    {
        for (uint64 tr = 2; tr < value / 2; tr++)
            if ((value % tr) == 0)
                return false;
        return true;
    }
    bool IsOdd(uint64 value)
    {
        // very slow check to pass some time (instead of checking module % 2 we will convert to binary and check if the
        // last bit is 0)
        uint32 bits[64];
        for (uint32 tr = 0; tr < 64; tr++)
        {
            bits[tr] = value & 1;
            value    = value >> 1;
        }
        return bits[0] == 0;
    }
    void Compute_odd()
    {
        Utils::LocalString<128> tmp;
        uint64 value = 1;
        uint64 count = 0;

        // in this case we know the maximum value (100.000) so we can use it to initialize the progress status
        ProgressStatus::Init("Compute", 100000);

        while (value < 100000)
        {
            if (IsOdd(value))
            {
                tmp.Format("Found so far %d odd numbers", (uint32) count);
                count++;
            }
            value++;
            // if progress status Update method returns true, then 'Escape' has been hit and we stop the search
            if (ProgressStatus::Update(value, tmp.GetText()))
                break;
        }
    }

    void Compute_prime()
    {
        Utils::LocalString<128> tmp;
        uint64 value = 1;
        uint64 count = 0;

        // in this case we don't know what is the range (so we will not provide one)
        ProgressStatus::Init("Compute");

        while (count < 10000)
        {
            if (IsPrime(value))
            {
                tmp.Format(
                      "Found %d prime numbers (now testing value: %d)", (uint32) count, (uint32) value);
                count++;
            }
            value++;
            // if progress status Update method returns true, then 'Escape' has been hit and we stop the search
            if (ProgressStatus::Update(count, tmp.GetText()))
                break;
        }
    }

    bool OnEvent(Reference<Control>, Event eventType, int controlID) override
    {
        if (eventType == Event::WindowClose)
        {
            Application::Close();
            return true;
        }
        if (eventType == Event::ButtonClicked)
        {
            switch (controlID)
            {
            case BUTTON_COMPUTE_ODD:
                Compute_odd();
                break;
            case BUTTON_COMPUTE_PRIME:
                Compute_prime();
                break;
            }
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
