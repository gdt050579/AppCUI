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
    Reference<Tree> tree;
    Reference<Button> btnGoTo, btnClose, btnCloseAll, btnCloseDescendands, btnCancel;
    std::map<ItemHandle, WinItemInfo> rel;
    ItemHandle focusedItem;

  public:
    InternalWindowManager() : AppCUI::Controls::Window("Window manager", "d:c,w:72,h:20", WindowFlags::None)
    {
    }
    bool Create();
    bool AddItem(Window* w, const ItemHandle parent, ItemHandle& child);
    void Process(std::map<ItemHandle, WinItemInfo>& rel, ItemHandle id, const ItemHandle parent);
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
    const auto i = tree->GetCurrentItem();
    btnGoTo->SetEnabled(i != InvalidItemHandle);
    btnClose->SetEnabled(i != InvalidItemHandle);
    btnCloseDescendands->SetEnabled(false);
    if (i == InvalidItemHandle)
    {
        return;
    }

    const auto data = tree->GetItemData(i);
    if (data == nullptr || data->Pointer == nullptr)
    {
        return;
    }

    const auto Members = reinterpret_cast<WindowControlContext*>((reinterpret_cast<Window*>(data->Pointer)->Context));
    if (Members == nullptr)
    {
        return;
    }

    // check if there is at least one descendent
    for (const auto& itm : rel)
    {
        if (itm.second.Referal == Members->windowItemHandle)
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
    const auto current = tree->GetCurrentItem();
    if (current == InvalidItemHandle)
    {
        return false;
    }

    LocalUnicodeStringBuilder<256> tmp;
    tmp.Add("Close ");
    tmp.Add(tree->GetItemText(current));
    tmp.Add(" ?");
    if (MessageBox::ShowOkCancel("Close", tmp.ToStringView()) == Result::Ok)
    {
        if (auto data = tree->GetItemData(current); data != nullptr && data->Pointer != nullptr)
        {
            (reinterpret_cast<Window*>(data->Pointer))->RemoveMe();
        }
        return true;
    }
    tree->RemoveItem(current);

    return false;
}

bool InternalWindowManager::RemoveCurrentWindowAndDescendents()
{
    const auto i = tree->GetCurrentItem();
    if (i == InvalidItemHandle)
    {
        return false;
    }

    LocalUnicodeStringBuilder<256> tmp;
    CHECK(tmp.Add("Close "), false, "");
    CHECK(tmp.Add(tree->GetItemText(i)), false, "");
    CHECK(tmp.Add(" and all of its descendants ?"), false, "");
    if (MessageBox::ShowOkCancel("Close", tmp.ToStringView()) != Result::Ok)
    {
        return false;
    }

    if (const auto data = tree->GetItemData(i); data != nullptr && data->Pointer != nullptr)
    {
        if (const auto Members =
                  reinterpret_cast<WindowControlContext*>((reinterpret_cast<Window*>(data->Pointer)->Context));
            Members)
        {
            CloseDescendants(Members->windowItemHandle);
        }
    }

    tree->RemoveItem(i);

    return true;
}

bool InternalWindowManager::CloseAll()
{
    if (MessageBox::ShowOkCancel("Close", "Close all existing windows ?") != Result::Ok)
    {
        return false;
    }

    const size_t count = tree->GetItemsCount();
    for (size_t tr = 0; tr < count; tr++)
    {
        if (auto data = tree->GetItemData(tr); data != nullptr && data->Pointer != nullptr)
        {
            (reinterpret_cast<Window*>(data->Pointer))->RemoveMe();
        }
    }

    tree->ClearItems();

    return true;
}

void InternalWindowManager::GoToSelectedItem()
{
    if (auto data = tree->GetItemData(tree->GetCurrentItem()); data != nullptr && data->Pointer != nullptr)
    {
        (reinterpret_cast<Window*>(data->Pointer))->SetFocus();
    }
}

bool InternalWindowManager::OnEvent(Control* c, Event eventType, int id)
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

bool InternalWindowManager::AddItem(Window* w, const ItemHandle parent, ItemHandle& child)
{
    const auto wcc = reinterpret_cast<WindowControlContext*>(w->Context);
    child          = tree->AddItem(parent, { wcc->Text, *const_cast<CharacterBuffer*>(&w->GetTag()) }, "", w);
    return true;
}

void InternalWindowManager::Process(std::map<ItemHandle, WinItemInfo>& rel, ItemHandle id, const ItemHandle parent)
{
    ItemHandle child = 0xFFFFFFFF;
    AddItem(rel[id].wnd, parent, child);
    rel[id].added = true;

    // search for all children
    for (const auto& i : rel)
    {
        if (i.second.Referal == id)
        {
            Process(rel, i.first, child);
        }
    }
}

bool InternalWindowManager::Create()
{
    CHECK(tree = Factory::Tree::Create(this, "l:1,t:1,r:1,b:3", TreeFlags::None, 2), false, "");
    CHECK(btnGoTo = Factory::Button::Create(this, "&Goto", "l:1,b:0,w:13", BUTTON_ID_GOTO), false, "");
    CHECK(btnClose = Factory::Button::Create(this, "&Close", "l:15,b:0,w:13", BUTTON_ID_CLOSE), false, "");
    CHECK(btnCloseDescendands = Factory::Button::Create(this, "Close &desc", "l:29,b:0,w:13", BUTTON_ID_CLOSE_DESC),
          false,
          "");
    CHECK(btnCloseAll = Factory::Button::Create(this, "Close &All", "l:43,b:0,w:13", BUTTON_ID_CLOSE_ALL), false, "");
    CHECK(btnCancel = Factory::Button::Create(this, "Cancel", "l:57,b:0,w:13", BUTTON_ID_CANCEL), false, "");

    // add all existing windows
    const auto app = AppCUI::Application::GetApplication();
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

        for (const auto& i : rel)
        {
            if (rel.contains(i.second.Referal))
            {
                continue;
            }

            // add this item
            Process(rel, i.first, 0xFFFFFFFF);
        }
    }

    UpdateButtonsStatus();

    return true;
}

void AppCUI::Dialogs::WindowManager::Show()
{
    if (InternalWindowManager dlg; dlg.Create())
    {
        dlg.Show();
    }
}
