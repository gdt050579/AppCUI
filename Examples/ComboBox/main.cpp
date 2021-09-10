#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

class MyWin : public AppCUI::Controls::Window
{
    ComboBox cb1, cb2, cb3;
    Label inf, inf2, inf3;
    TextField col;

  public:
    MyWin()
    {
        this->Create("ComboBox example", "d:c,w:56,h:11");
        inf.Create(this, "Select a color", "x:2,y:1,w:15");
        cb1.Create(this, "x:22,y:1,w:30", "White,Blue,Red,Aqua,Metal,Yellow,Green,Orange");
        inf2.Create(this, "Select a word", "x:2,y:3,w:15");
        cb2.Create(this, "x:22,y:3,w:30", u8"Déjà vu,Schön,Groß,Fähig,Любовь,Кошка,Улыбаться");
        inf3.Create(this, "Select a vehicle", "x:2,y:5,w:18");
        cb3.Create(this, "x:22,y:5,w:30");
        col.Create(this, "", "x:2,y:7,w:50");

        cb3.AddSeparator("Cars");
        cb3.AddItem("Mercedes");
        cb3.AddItem("Skoda");
        cb3.AddItem("Toyota");
        cb3.AddItem("Ford");
        cb3.AddSeparator("Motorcycles");
        cb3.AddItem("BMW");
        cb3.AddItem("Ducatti");
        
    }
    bool OnEvent(Control* sender, Event eventType, int) override
    {
        if (eventType == Event::WindowClose)
        {
            Application::Close();
            return true;
        }
        if (eventType == Event::ComboBoxSelectedItemChanged)
        {
            ComboBox* c = reinterpret_cast<ComboBox*>(sender);
            AppCUI::Utils::LocalUnicodeStringBuilder<128> temp;
            temp.Add(c->GetCurrentItemText());
            temp.Add(" => Index: ");
            AppCUI::Utils::LocalString<32> value;
            value.Format("%u", c->GetCurrentItemIndex());
            temp.Add(value.GetText());
            col.SetText(temp);
        }
        return false;
    }
};

int main()
{
    Log::ToOutputDebugString();
    if (!Application::Init())
        return 1;
    Application::AddWindow(new MyWin());
    Application::Run();
    return 0;
}
