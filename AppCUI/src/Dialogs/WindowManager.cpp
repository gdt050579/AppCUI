#include "Internal.hpp"
#include "ControlContext.hpp"
#include <map>

#ifdef MessageBox
#    undef MessageBox
#endif

using namespace AppCUI::Controls;
using namespace AppCUI::Dialogs;
using namespace AppCUI::Graphics;

constexpr int BUTTON_ID_GOTO       = 10000;
constexpr int BUTTON_ID_CLOSE      = 10001;
constexpr int BUTTON_ID_CLOSE_ALL  = 10002;
constexpr int BUTTON_ID_CLOSE_DESC = 10003;
constexpr int BUTTON_ID_CANCEL     = 10004;

struct WinItemInfo
{
    ItemHandle Referal;
    Window* wnd;
    bool added;
};

class InternalWindowManager : public AppCUI::Controls::Window
{
    ListView lst;
    Button btnGoTo, btnClose, btnCloseAll, btnCloseDescendands, btnCancel;
    std::map<ItemHandle, WinItemInfo> rel;
    ItemHandle focusedItem;

  public:
    bool Create();
    bool AddItem(Window* w, unsigned int offsetX);
    void Process(std::map<ItemHandle, WinItemInfo>& rel, ItemHandle id, unsigned int offsetX);
    bool OnEvent(Control* c, Event eventType, int id) override;
    void GoToSelectedItem();
    bool RemoveCurrentWindow();
    bool RemoveCurrentWindowAndDescendents();
    bool CloseAll();
    void UpdateButtonsStatus();
    void CloseDescendants(ItemHandle id);
};

void InternalWindowManager::UpdateButtonsStatus()
{
    btnCloseAll.SetEnabled(lst.GetItemsCount() > 0);
    auto i = lst.GetCurrentItem();
    btnGoTo.SetEnabled(i != InvalidItemHandle);
    btnClose.SetEnabled(i != InvalidItemHandle);
    btnCloseDescendands.SetEnabled(false);
    if (i == InvalidItemHandle)
        return;
    
    auto data = lst.GetItemData(i);
    if ((data) && (data->Pointer))
    {
        auto Members = reinterpret_cast<WindowControlContext*>((reinterpret_cast<Window*>(data->Pointer)->Context));
        if (Members)
        {
            // check if there is at least one descendent
            for (auto itm : rel)
            {
                if (itm.second.Referal == Members->windowItemHandle)
                {
                    btnCloseDescendands.SetEnabled(true);
                    break;
                }
            }
        }
    }
    
}
void InternalWindowManager::CloseDescendants(ItemHandle id)
{
    rel[id].wnd->RemoveMe();
    for (auto itm : rel)
    {
        if (itm.second.Referal == id)
            CloseDescendants(itm.first);
    }
}
bool InternalWindowManager::RemoveCurrentWindow()
{
    auto i = lst.GetCurrentItem();
    if (i == InvalidItemHandle)
        return false;
    LocalUnicodeStringBuilder<256> tmp;
    tmp.Add("Close ");
    tmp.Add(lst.GetItemText(i, 0));
    tmp.Add(" ?");
    if (MessageBox::ShowOkCancel("Close", tmp.ToStringView()) == Result::Ok)
    {
        auto data = lst.GetItemData(i);
        if ((data) && (data->Pointer))
            (reinterpret_cast<Window*>(data->Pointer))->RemoveMe();
        return true;
    }
    return false;
}
bool InternalWindowManager::RemoveCurrentWindowAndDescendents()
{
    auto i = lst.GetCurrentItem();
    if (i == InvalidItemHandle)
        return false;
    LocalUnicodeStringBuilder<256> tmp;
    CHECK(tmp.Add("Close "), false, "");
    CHECK(tmp.Add(lst.GetItemText(i, 0)), false, "");
    CHECK(tmp.Add(" and all of its descendants ?"), false, "");
    if (MessageBox::ShowOkCancel("Close", tmp.ToStringView()) != Result::Ok)
        return false;

    auto data = lst.GetItemData(i);
    if ((data) && (data->Pointer))
    {
        auto Members = reinterpret_cast<WindowControlContext*>((reinterpret_cast<Window*>(data->Pointer)->Context));
        if (Members)
        {
            CloseDescendants(Members->windowItemHandle);
        }
    }
    return true;

}
bool InternalWindowManager::CloseAll()
{
    if (MessageBox::ShowOkCancel("Close", "Close all existing windows ?") != Result::Ok)
        return false;
    const unsigned int count = lst.GetItemsCount();
    for (unsigned int tr = 0; tr < count; tr++)
    {
        auto data = lst.GetItemData(tr);
        if ((data) && (data->Pointer))
            (reinterpret_cast<Window*>(data->Pointer))->RemoveMe();
    }
    return true;
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
        case BUTTON_ID_CLOSE:
            if (RemoveCurrentWindow())
                Exit(Result::Ok);
            return true;
        case BUTTON_ID_CLOSE_DESC:
            if (RemoveCurrentWindowAndDescendents())
                Exit(Result::Ok);
            return true;
        case BUTTON_ID_CLOSE_ALL:
            if (CloseAll())
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
    if (eventType == Event::ListViewCurrentItemChanged)
    {
        UpdateButtonsStatus();
        return true;
    }
    return false;
}

bool InternalWindowManager::AddItem(Window* w, unsigned int offsetX)
{
    const auto Members = reinterpret_cast<WindowControlContext*>(w->Context);
    auto i             = lst.AddItem(Members->Text, (CharacterView) w->GetTag());
    lst.SetItemXOffset(i, offsetX);
    lst.SetItemData(i, ItemData(w));
    if (w->HasFocus())
    {
        lst.SetItemType(i, ListViewItemType::Highlighted);
        this->focusedItem = i;
    }
        
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
    CHECK(Window::Create("Window manager", "d:c,w:72,h:20"), false, "");
    CHECK(lst.Create(this, "l:1,t:1,r:1,b:3", ListViewFlags::SearchMode), false, "");
    CHECK(lst.AddColumn("Window caption", TextAlignament::Left, 56), false, "");
    CHECK(lst.AddColumn("TAG", TextAlignament::Left, 8), false, "");
    CHECK(btnGoTo.Create(this, "&Goto", "l:1,b:0,w:13", BUTTON_ID_GOTO), false, "");
    CHECK(btnClose.Create(this, "&Close", "l:15,b:0,w:13", BUTTON_ID_CLOSE), false, "");
    CHECK(btnCloseDescendands.Create(this, "Close &desc", "l:29,b:0,w:13", BUTTON_ID_CLOSE_DESC), false, "");
    CHECK(btnCloseAll.Create(this, "Close &All", "l:43,b:0,w:13", BUTTON_ID_CLOSE_ALL), false, "");
    CHECK(btnCancel.Create(this, "Cancel", "l:57,b:0,w:13", BUTTON_ID_CANCEL), false, "");

    // add all existing windows
    auto* app = AppCUI::Application::GetApplication();
    CHECK(app, false, "");
    CHECK(app->AppDesktop, false, "");
    CHECK(app->AppDesktop->Context, false, "");
    const auto desktopMembers = reinterpret_cast<ControlContext*>(app->AppDesktop->Context);
    auto wnd                  = desktopMembers->Controls;
    auto wEnd                 = wnd + desktopMembers->ControlsCount;
    this->focusedItem         = InvalidItemHandle;

    if (wnd)
    {
        while (wnd < wEnd)
        {
            const auto winMembers             = reinterpret_cast<WindowControlContext*>((*wnd)->Context);
            if (winMembers)
            {
                rel[winMembers->windowItemHandle] = { winMembers->referalItemHandle,
                                                      reinterpret_cast<Window*>(*wnd),
                                                      false };
            }
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

    if (this->focusedItem != InvalidItemHandle)
        lst.SetCurrentItem(this->focusedItem);

    UpdateButtonsStatus();

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