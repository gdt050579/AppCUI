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
    Tree tree;
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
    auto i = tree.GetCurrentItem();
    btnGoTo.SetEnabled(i != InvalidItemHandle);
    btnClose.SetEnabled(i != InvalidItemHandle);
    btnCloseDescendands.SetEnabled(false);
    if (i == InvalidItemHandle)
        return;

    auto data = tree.GetItemData(i);
    if ((data) && (data->Pointer))
    {
        auto Members = reinterpret_cast<WindowControlContext*>((reinterpret_cast<Window*>(data->Pointer)->Context));
        if (Members)
        {
            // check if there is at least one descendent
            for (const auto& itm : rel)
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
    for (const auto& itm : rel)
    {
        if (itm.second.Referal == id)
            CloseDescendants(itm.first);
    }
}
bool InternalWindowManager::RemoveCurrentWindow()
{
    const auto current = tree.GetCurrentItem();
    if (current == InvalidItemHandle)
        return false;

    LocalUnicodeStringBuilder<256> tmp;
    tmp.Add("Close ");
    tmp.Add(tree.GetItemText(current));
    tmp.Add(" ?");
    if (MessageBox::ShowOkCancel("Close", tmp.ToStringView()) == Result::Ok)
    {
        auto data = tree.GetItemData(current);
        if ((data) && (data->Pointer))
            (reinterpret_cast<Window*>(data->Pointer))->RemoveMe();
        return true;
    }
    tree.RemoveItem(current);

    return false;
}
bool InternalWindowManager::RemoveCurrentWindowAndDescendents()
{
    auto i = tree.GetCurrentItem();
    if (i == InvalidItemHandle)
        return false;

    LocalUnicodeStringBuilder<256> tmp;
    CHECK(tmp.Add("Close "), false, "");
    CHECK(tmp.Add(tree.GetItemText(i)), false, "");
    CHECK(tmp.Add(" and all of its descendants ?"), false, "");
    if (MessageBox::ShowOkCancel("Close", tmp.ToStringView()) != Result::Ok)
        return false;

    auto data = tree.GetItemData(i);
    if ((data) && (data->Pointer))
    {
        auto Members = reinterpret_cast<WindowControlContext*>((reinterpret_cast<Window*>(data->Pointer)->Context));
        if (Members)
        {
            CloseDescendants(Members->windowItemHandle);
        }
    }

    tree.RemoveItem(i);

    return true;
}
bool InternalWindowManager::CloseAll()
{
    if (MessageBox::ShowOkCancel("Close", "Close all existing windows ?") != Result::Ok)
        return false;

    const size_t count = tree.GetItemsCount();
    for (size_t tr = 0; tr < count; tr++)
    {
        auto data = tree.GetItemData(tr);
        if ((data) && (data->Pointer))
            (reinterpret_cast<Window*>(data->Pointer))->RemoveMe();
    }

    tree.ClearItems();

    return true;
}
void InternalWindowManager::GoToSelectedItem()
{
    auto data = tree.GetItemData(tree.GetCurrentItem());
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
    const auto wcc = reinterpret_cast<WindowControlContext*>(w->Context);
    const std::u16string itemTitle(wcc->Text);
    const auto i = tree.AddItem(wcc->referalItemHandle, { itemTitle }, w);
    return true;
}
void InternalWindowManager::Process(std::map<ItemHandle, WinItemInfo>& rel, ItemHandle id, unsigned int offsetX)
{
    AddItem(rel[id].wnd, offsetX);
    rel[id].added = true;
    // search for all children
    for (const auto& i : rel)
    {
        if (i.second.Referal == id)
            Process(rel, i.first, offsetX + 2);
    }
}
bool InternalWindowManager::Create()
{
    CHECK(Window::Create("Window manager", "d:c,w:72,h:20"), false, "");
    CHECK(tree.Create(this, "l:1,t:1,r:1,b:3"), false, "");
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
            const auto winMembers = reinterpret_cast<WindowControlContext*>((*wnd)->Context);
            if (winMembers)
            {
                rel[winMembers->windowItemHandle] = { winMembers->referalItemHandle,
                                                      reinterpret_cast<Window*>(*wnd),
                                                      false };
            }
            wnd++;
        }
        for (const auto& i : rel)
        {
            if (rel.contains(i.second.Referal))
                continue;
            // add this item
            Process(rel, i.first, 0);
        }
    }

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