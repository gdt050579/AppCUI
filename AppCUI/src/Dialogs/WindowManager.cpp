#include "Internal.hpp"
#include "ControlContext.hpp"
#include <map>

#ifdef MessageBox
#    undef MessageBox
#endif

using namespace AppCUI::Controls;
using namespace AppCUI::Dialogs;
using namespace AppCUI::Graphics;

constexpr int BUTTON_ID_GOTO   = 10000;
constexpr int BUTTON_ID_DELETE = 10001;
constexpr int BUTTON_ID_CANCEL = 10002;

struct WinItemInfo
{
    ItemHandle Referal;
    Window* wnd;
    bool added;
};

class InternalWindowManager : public AppCUI::Controls::Window
{
    ListView lst;
    Button btnGoTo, btnDelete, btnCancel;

  public:
    bool Create();
    bool AddItem(Window* w, unsigned int offsetX);
    void Process(std::map<ItemHandle, WinItemInfo>& rel, ItemHandle id, unsigned int offsetX);
    bool OnEvent(Control* c, Event eventType, int id) override;
    void GoToSelectedItem();
    void RemoveCurrentWindow();
};

void InternalWindowManager::RemoveCurrentWindow()
{
    auto i = lst.GetCurrentItem();
    if (i == InvalidItemHandle)
        return;
    LocalUnicodeStringBuilder<256> tmp;
    tmp.Add("Kill ");
    tmp.Add(lst.GetItemText(i,0));
    tmp.Add(" ?");
    if (MessageBox::ShowOkCancel("Terminate",tmp.ToStringView())==Result::Ok)
    {
        
    }
}
void InternalWindowManager::GoToSelectedItem()
{
    auto data = lst.GetItemData(lst.GetCurrentItem());
    if ((!data) || (!data->Pointer))
        return;
    (reinterpret_cast<Window*>(data->Pointer))->SetFocus();
}
bool InternalWindowManager::OnEvent(Control* c, Event eventType, int id)
{
    if (Window::OnEvent(c, eventType, id))
        return true;
    if (eventType == Event::ButtonClicked)
    {
        switch (id)
        {
        case BUTTON_ID_GOTO:
            GoToSelectedItem();
            Exit(Result::Ok);
            return true;
        case BUTTON_ID_DELETE:
            RemoveCurrentWindow();
            Exit(Result::Ok);
            return true;
        case BUTTON_ID_CANCEL:
            Exit(Result::Cancel);
            return true;
        }
    }
    if (eventType == Event::ListViewItemClicked)
    {
        GoToSelectedItem();
        Exit(Result::Ok);
        return true;
    }
    return false;
}

bool InternalWindowManager::AddItem(Window* w, unsigned int offsetX)
{
    const auto Members = reinterpret_cast<WindowControlContext*>(w->Context);
    auto i             = lst.AddItem(Members->Text,(CharacterView)w->GetTag());
    lst.SetItemXOffset(i, offsetX);
    lst.SetItemData(i, ItemData(w));
    if (w->HasFocus())
        lst.SetItemType(i, ListViewItemType::Highlighted);
    return true;
}
void InternalWindowManager::Process(std::map<ItemHandle, WinItemInfo>& rel, ItemHandle id, unsigned int offsetX)
{
    AddItem(rel[id].wnd, offsetX);
    rel[id].added = true;
    // search for all children
    for (auto i : rel)
    {
        if (i.second.Referal == id)
            Process(rel, i.first, offsetX + 2);
    }
}
bool InternalWindowManager::Create()
{
    CHECK(Window::Create("Window manager", "d:c,w:70,h:20"), false, "");
    CHECK(lst.Create(this, "l:1,t:1,r:1,b:3", ListViewFlags::SearchMode), false, "");
    CHECK(lst.AddColumn("Window caption", TextAlignament::Left, 50), false, "");
    CHECK(lst.AddColumn("TAG", TextAlignament::Left, 20), false, "");
    CHECK(btnGoTo.Create(this, "&Goto", "l:15,b:0,w:11", BUTTON_ID_GOTO), false, "");
    CHECK(btnDelete.Create(this, "&Delete", "l:27,b:0,w:11", BUTTON_ID_DELETE), false, "");
    CHECK(btnCancel.Create(this, "&Cancel", "l:39,b:0,w:11", BUTTON_ID_CANCEL), false, "");

    // add all existing windows
    auto* app = AppCUI::Application::GetApplication();
    CHECK(app, false, "");
    CHECK(app->AppDesktop, false, "");
    CHECK(app->AppDesktop->Context, false, "");
    const auto desktopMembers = reinterpret_cast<ControlContext*>(app->AppDesktop->Context);
    auto wnd                  = desktopMembers->Controls;
    auto wEnd                 = wnd + desktopMembers->ControlsCount;

    
    if (wnd)
    {      
        std::map<ItemHandle, WinItemInfo> rel;
        while (wnd < wEnd)
        {
            const auto winMembers             = reinterpret_cast<WindowControlContext*>((*wnd)->Context);
            rel[winMembers->windowItemHandle] = { winMembers->referalItemHandle,
                                                  reinterpret_cast<Window*>(*wnd),
                                                  false };
            wnd++;
        }
        for (auto i : rel)
        {
            if (rel.contains(i.second.Referal))
                continue;
            // add this item
            Process(rel, i.first, 0);
        }
    }

    return true;
}

void AppCUI::Dialogs::WindowManager::Show()
{
    InternalWindowManager dlg;
    if (dlg.Create())
    {
        dlg.Show();
    }
}