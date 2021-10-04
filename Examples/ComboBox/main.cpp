#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

class MyWin : public Window
{
    TextField *col;

  public:
    MyWin() : Window("ComboBox example", "d:c,w:56,h:11", WindowFlags::None)
    {
        Factory::Label::Create(this, "Select a color", "x:2,y:1,w:15");
        Factory::ComboBox::Create(this, "x:22,y:1,w:30", "White,Blue,Red,Aqua,Metal,Yellow,Green,Orange");
        Factory::Label::Create(this, "Select a word", "x:2,y:3,w:15");
        Factory::ComboBox::Create(this, "x:22,y:3,w:30", u8"Déjà vu,Schön,Groß,Fähig,Любовь,Кошка,Улыбаться");
        Factory::Label::Create(this, "Select a vehicle", "x:2,y:5,w:18");
        auto cb = Factory::ComboBox::Create(this, "x:22,y:5,w:30");
        col = Factory::TextField::Create(this, "", "x:2,y:7,w:50");

        cb->AddSeparator("Cars");
        cb->AddItem("Mercedes");
        cb->AddItem("Skoda");
        cb->AddItem("Toyota");
        cb->AddItem("Ford");
        cb->AddSeparator("Motorcycles");
        cb->AddItem("BMW");
        cb->AddItem("Ducatti");
        
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
            col->SetText(temp);
        }
        return false;
    }
};

int main()
{
    Log::ToOutputDebugString();
    if (!Application::Init())
        return 1;
    Application::AddWindow(std::make_unique<MyWin>());
    Application::Run();
    return 0;
}
