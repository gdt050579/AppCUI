#include "ControlContext.hpp"

#ifdef MessageBox
#    undef MessageBox
#endif

namespace AppCUI::Dialogs
{
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

class InternalWindowManager : public Controls::Window
{
    Reference<TreeView> tree;
    Reference<Button> btnGoTo, btnClose, btnCloseAll, btnCloseDescendands, btnCancel;
    std::map<ItemHandle, WinItemInfo> rel;
    ItemHandle focusedItem;

  public:
    InternalWindowManager() : Controls::Window("Window manager", "d:c,w:72,h:20", WindowFlags::None)
    {
    }
    bool Create();
    bool AddItem(Window* w, TreeViewItem& parent, TreeViewItem& child);
    void Process(std::map<ItemHandle, WinItemInfo>& rel, ItemHandle id, ItemHandle handleParent);
    bool OnEvent(Reference<Control> c, Event eventType, int id) override;
    void GoToSelectedItem();
    bool RemoveCurrentWindow();
    bool RemoveCurrentWindowAndDescendents();
    bool CloseAll();
    void UpdateButtonsStatus();
    void CloseDescendants(ItemHandle id);
};

void InternalWindowManager::UpdateButtonsStatus()
{
    const auto i = tree->GetCurrentItem();
    btnGoTo->SetEnabled(i.IsValid());
    btnClose->SetEnabled(i.IsValid());
    btnCloseDescendands->SetEnabled(false);
    CHECKRET(i.IsValid(), "");

    auto data = i.GetData<Window>();
    CHECKRET(data.IsValid(), "");

    const auto wcc = reinterpret_cast<WindowControlContext*>(data->Context);
    CHECKRET(wcc != nullptr, "");

    // check if there is at least one descendent
    for (const auto& itm : rel)
    {
        if (itm.second.Referal == wcc->windowItemHandle)
        {
            btnCloseDescendands->SetEnabled(true);
            break;
        }
    }
}

void InternalWindowManager::CloseDescendants(ItemHandle id)
{
    rel[id].wnd->RemoveMe();
    for (const auto& itm : rel)
    {
        if (itm.second.Referal == id)
        {
            CloseDescendants(itm.first);
        }
    }
}

bool InternalWindowManager::RemoveCurrentWindow()
{
    auto current = tree->GetCurrentItem();
    CHECK(current.IsValid(), false, "");

    LocalUnicodeStringBuilder<256> tmp;
    tmp.Add("Close ");
    tmp.Add(current.GetText());
    tmp.Add(" ?");
    if (MessageBox::ShowOkCancel("Close", tmp.ToStringView()) == Result::Ok)
    {
        if (auto win = current.GetData<Window>(); win.IsValid())
        {
            win->RemoveMe();
        }
        return true;
    }
    tree->RemoveItem(current);

    return false;
}

bool InternalWindowManager::RemoveCurrentWindowAndDescendents()
{
    auto current = tree->GetCurrentItem();
    CHECK(current.IsValid(), false, "");

    LocalUnicodeStringBuilder<256> tmp;
    CHECK(tmp.Add("Close "), false, "");
    CHECK(tmp.Add(current.GetText()), false, "");
    CHECK(tmp.Add(" and all of its descendants ?"), false, "");
    CHECK(MessageBox::ShowOkCancel("Close", tmp.ToStringView()) == Result::Ok, false, "");

    if (auto win = current.GetData<Window>(); win.IsValid())
    {
        if (const auto wcc = reinterpret_cast<WindowControlContext*>(win->Context); wcc)
        {
            CloseDescendants(wcc->windowItemHandle);
        }
    }

    tree->RemoveItem(current);

    return true;
}

bool InternalWindowManager::CloseAll()
{
    if (MessageBox::ShowOkCancel("Close", "Close all existing windows ?") != Result::Ok)
    {
        return false;
    }

    const auto count = tree->GetItemsCount();
    for (auto i = 0U; i < count; i++)
    {
        if (const auto item = tree->GetItemByIndex(i); item.IsValid())
        {
            if (auto win = item.GetData<Window>(); win.IsValid())
            {
                win->RemoveMe();
            }
        }
    }

    tree->ClearItems();

    return true;
}

void InternalWindowManager::GoToSelectedItem()
{
    auto current = tree->GetCurrentItem();
    if (auto win = current.GetData<Window>(); win.IsValid())
    {
        win->SetFocus();
    }
}

bool InternalWindowManager::OnEvent(Reference<Control> c, Event eventType, int id)
{
    if (Window::OnEvent(c, eventType, id))
    {
        return true;
    }

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
            {
                Exit(Result::Ok);
            }
            return true;
        case BUTTON_ID_CLOSE_DESC:
            if (RemoveCurrentWindowAndDescendents())
            {
                Exit(Result::Ok);
            }
            return true;
        case BUTTON_ID_CLOSE_ALL:
            if (CloseAll())
            {
                Exit(Result::Ok);
            }
            return true;
        case BUTTON_ID_CANCEL:
            Exit(Result::Cancel);
            return true;
        }
    }

    if (eventType == Event::ListViewItemPressed)
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

bool InternalWindowManager::AddItem(Window* w, TreeViewItem& parent, TreeViewItem& child)
{
    const auto wcc = reinterpret_cast<WindowControlContext*>(w->Context);
    if (parent.GetHandle() == TreeView::RootHandle)
    {
        child = tree->AddItem(wcc->Text.operator std::string(), true);
    }
    else
    {
        child = parent.AddChild(wcc->Text.operator std::string(), true);
    }
    child.SetValues({ w->GetTag().operator std::string() });
    child.SetData(Reference<Window>(w));
    return true;
}

void InternalWindowManager::Process(std::map<ItemHandle, WinItemInfo>& rel, ItemHandle id, ItemHandle handleParent)
{
    TreeViewItem child{};
    TreeViewItem parent = tree->GetItemByHandle(handleParent);
    AddItem(rel[id].wnd, parent, child);
    rel[id].added = true;

    // search for all children
    for (const auto& i : rel)
    {
        if (i.second.Referal == id)
        {
            Process(rel, i.first, child.GetHandle());
        }
    }
}

bool InternalWindowManager::Create()
{
    CHECK((tree = Factory::TreeView::Create(
                 this, "l:1,t:1,r:1,b:3", { "n:Window,w:fill", "n:Tag,w:10" }, TreeViewFlags::Searchable))
                .IsValid(),
          false,
          "");
    CHECK((btnGoTo = Factory::Button::Create(this, "&Goto", "l:1,b:0,w:13", BUTTON_ID_GOTO)).IsValid(), false, "");
    CHECK((btnClose = Factory::Button::Create(this, "&Close", "l:15,b:0,w:13", BUTTON_ID_CLOSE)).IsValid(), false, "");
    CHECK((btnCloseDescendands = Factory::Button::Create(this, "Close &desc", "l:29,b:0,w:13", BUTTON_ID_CLOSE_DESC))
                .IsValid(),
          false,
          "");
    CHECK((btnCloseAll = Factory::Button::Create(this, "Close &All", "l:43,b:0,w:13", BUTTON_ID_CLOSE_ALL)).IsValid(),
          false,
          "");
    CHECK((btnCancel = Factory::Button::Create(this, "Cancel", "l:57,b:0,w:13", BUTTON_ID_CANCEL)).IsValid(),
          false,
          "");

    // add all existing windows
    const auto app = Application::GetApplication();
    CHECK(app != nullptr, false, "");
    CHECK(app->AppDesktop != nullptr, false, "");
    CHECK(app->AppDesktop->Context != nullptr, false, "");
    const auto desktopMembers = reinterpret_cast<ControlContext*>(app->AppDesktop->Context);
    this->focusedItem         = InvalidItemHandle;

    if (auto wnd = desktopMembers->Controls; wnd)
    {
        auto wEnd = wnd + desktopMembers->ControlsCount;

        while (wnd < wEnd)
        {
            if (const auto winMembers = reinterpret_cast<WindowControlContext*>((*wnd)->Context); winMembers)
            {
                rel[winMembers->windowItemHandle] = { winMembers->referalItemHandle,
                                                      reinterpret_cast<Window*>(*wnd),
                                                      false };
            }
            wnd++;
        }

        for (const auto& [handle, info] : rel)
        {
            if (rel.contains(info.Referal))
            {
                continue;
            }

            // add this item
            Process(rel, handle, TreeView::RootHandle);
        }
    }

    UpdateButtonsStatus();

    return true;
}

void WindowManager::Show()
{
    if (InternalWindowManager dlg; dlg.Create())
    {
        dlg.Show();
    }
}
} // namespace AppCUI::Dialogs