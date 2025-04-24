#include "ControlContext.hpp"

#ifdef MessageBox
#    undef MessageBox
#endif

namespace AppCUI::Dialogs
{
constexpr int BUTTON_ID_GOTO        = 10000;
constexpr int BUTTON_ID_CLOSE       = 10001;
constexpr int BUTTON_ID_CLOSE_ALL   = 10002;
constexpr int BUTTON_ID_CLOSE_DESC  = 10003;
constexpr int BUTTON_ID_CANCEL      = 10004;
constexpr int BUTTON_ID_COPY_NOTE   = 10005;
constexpr int BUTTON_ID_EDIT_NOTE   = 10006;
constexpr int BUTTON_ID_REMOVE_NOTE = 10007;

struct WinItemInfo
{
    ItemHandle referal;
    Window* wnd;
    bool added;
};

class InternalWindowManager : public Controls::Window, Handlers::OnTreeViewCurrentItemChangedInterface
{
    enum class NoteOperation
    {
        Copy,
        Edit,
        Delete
    };

    Reference<TreeView> windowTree, notesTree;
    Reference<Button> btnGoTo, btnClose, btnCloseAll, btnCloseDescendands, btnCancel;
    std::map<ItemHandle, WinItemInfo> rel;
    ItemHandle focusedItem{ InvalidItemHandle };
    Reference<Splitter> windowSplit;
    Reference<Panel> lPanel, rPanel;
    Reference<Label> rPanelWindowNameLabel;
    Reference<Button> btnCopyNote, btnEditNote, btnDeleteNote;

  public:
    InternalWindowManager() : Controls::Window("Window manager", "d:c,w:85,h:20", WindowFlags::Sizeable)
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
    void CopyEditOrDeleteNote(NoteOperation operation);

    virtual void OnTreeViewCurrentItemChanged(Reference<Controls::TreeView> tree, TreeViewItem& item) override;
};

class SingleLineWindow : public Controls::Window
{
    constexpr static int BUTTON_ID_OK    = 10000;
    constexpr static int BUTTON_ID_CLOSE = 10001;

    CharacterBuffer data;
    Reference<TextField> input;

  public:
    SingleLineWindow(CharacterBuffer noteData)
        : Window("Note edit", "d:c,w:30,h:8", WindowFlags::Sizeable), data(std::move(noteData))
    {
        input    = Factory::TextField::Create(this, data, "l:1,t:1,r:1",TextFieldFlags::None);
        Factory::Button::Create(this, "OK", "l:6,b:0,w:10", BUTTON_ID_OK);
        Factory::Button::Create(this, "Close", "l:16,b:0,w:10", BUTTON_ID_CLOSE);
        input->SetFocus();
    }

    bool OnEvent(Reference<Control> c, Event eventType, int id) override
    {
        if (eventType == Event::WindowClose || eventType == Event::WindowAccept)
        {
            Exit(Result::Cancel);
            return true;
        }
        if (eventType != Event::ButtonClicked)
            return true;
        switch (id)
        {
        case BUTTON_ID_OK:
            if (input->GetText().Len() > 0)
            {
                data = input->GetText();
                Exit(Result::Ok);
            }
            else
                MessageBox::ShowError("Error", "Note cannot be empty !");
            return true;
        case BUTTON_ID_CLOSE:
            Exit(Result::Cancel);
            return true;
        default:
            return true;
        }
    }

    const CharacterBuffer& GetNote() const
    {
        return data;
    }
};

void InternalWindowManager::CopyEditOrDeleteNote(NoteOperation operation)
{
    const auto currentNoteItem = notesTree->GetCurrentItem();
    if (!currentNoteItem.IsValid())
        return;
    const auto& note = currentNoteItem.GetText();
    if (!note.IsValid())
        return;
    if (operation == NoteOperation::Copy)
    {
        OS::Clipboard::SetText(note.operator std::string());
        return;
    }

    auto currentWindowItem = windowTree->GetCurrentItem();
    if (!currentWindowItem.IsValid())
        return;
    auto win = currentWindowItem.GetData<Window>();
    if (!win.IsValid())
        return;
    const auto& allNotes = win->GetNotes();
    for (uint32 i = 0; i < allNotes.size(); i++)
    {
        if (allNotes[i].CompareWith(note) == 0)
        {
            if (operation == NoteOperation::Delete)
            {
                win->RemoveNote(i);
            }
            else if (operation == NoteOperation::Edit)
            {
                SingleLineWindow noteEditWindow(note);
                const auto result = noteEditWindow.Show();
                if (result != Result::Ok)
                    break;
                std::u16string newNodeStr;
                if (!noteEditWindow.GetNote().ToString(newNodeStr))
                    break;
                win->UpdateNote(i, newNodeStr);
            }
            OnTreeViewCurrentItemChanged(windowTree, currentWindowItem);
            break;
        }
    }
}

void InternalWindowManager::UpdateButtonsStatus()
{
    const auto i = windowTree->GetCurrentItem();
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
        if (itm.second.referal == wcc->windowItemHandle)
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
        if (itm.second.referal == id)
        {
            CloseDescendants(itm.first);
        }
    }
}

void InternalWindowManager::OnTreeViewCurrentItemChanged(Reference<Controls::TreeView> tree, TreeViewItem& item)
{
    auto win = item.GetData<Window>();
    if (!win.IsValid())
        return;
    notesTree->ClearItems();
    rPanelWindowNameLabel->SetText(win->GetText());
    const auto& notes = win->GetNotes();
    for (const auto& note : notes)
        notesTree->AddItem(note.operator std::string(), false);
    const bool buttonsEnabled = notesTree->GetItemsCount() > 0;
    btnCopyNote->SetEnabled(buttonsEnabled);
    btnEditNote->SetEnabled(buttonsEnabled);
    btnDeleteNote->SetEnabled(buttonsEnabled);
}

bool InternalWindowManager::RemoveCurrentWindow()
{
    auto current = windowTree->GetCurrentItem();
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
    windowTree->RemoveItem(current);

    return false;
}

bool InternalWindowManager::RemoveCurrentWindowAndDescendents()
{
    auto current = windowTree->GetCurrentItem();
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

    windowTree->RemoveItem(current);

    return true;
}

bool InternalWindowManager::CloseAll()
{
    CHECK(MessageBox::ShowOkCancel("Close", "Close all existing windows ?") == Result::Ok, false, "");

    const auto count = windowTree->GetItemsCount();
    for (auto i = 0U; i < count; i++)
    {
        if (const auto item = windowTree->GetItemByIndex(i); item.IsValid())
        {
            if (auto win = item.GetData<Window>(); win.IsValid())
            {
                win->RemoveMe();
            }
        }
    }

    windowTree->ClearItems();

    return true;
}

void InternalWindowManager::GoToSelectedItem()
{
    auto current = windowTree->GetCurrentItem();
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
        case BUTTON_ID_COPY_NOTE:
            CopyEditOrDeleteNote(NoteOperation::Copy);
            return true;
        case BUTTON_ID_EDIT_NOTE:
            CopyEditOrDeleteNote(NoteOperation::Edit);
            return true;
        case BUTTON_ID_REMOVE_NOTE:
            CopyEditOrDeleteNote(NoteOperation::Delete);
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
        child = windowTree->AddItem(wcc->Text.operator std::string(), false);
    }
    else
    {
        CHECK(parent.SetExpandable(true), false, "");
        child = parent.AddChild(wcc->Text.operator std::string(), false);
    }
    child.SetValues({ w->GetTag().operator std::string(), w->GetCreationProcess().operator std::string() });
    child.SetData(Reference<Window>(w));

    if (w && w->HasFocus())
    {
        this->focusedItem = child.GetHandle();
    }

    return true;
}

void InternalWindowManager::Process(std::map<ItemHandle, WinItemInfo>& rel, ItemHandle id, ItemHandle handleParent)
{
    TreeViewItem child{};
    TreeViewItem parent = windowTree->GetItemByHandle(handleParent);
    AddItem(rel[id].wnd, parent, child);
    rel[id].added = true;

    // search for all children
    for (const auto& i : rel)
    {
        if (i.second.referal == id)
        {
            Process(rel, i.first, child.GetHandle());
        }
    }
}

bool InternalWindowManager::Create()
{
    windowSplit = Factory::Splitter::Create(this, "l:0,t:0,r:0,b:2", SplitterFlags::Vertical);
    lPanel      = Factory::Panel::Create(windowSplit, "l:50%,t:0,b:0");

    Factory::Label::Create(lPanel, "All existing windows", "l:1,t:0,w:30");
    CHECK((windowTree = Factory::TreeView::Create(
                 lPanel,
                 "l:0,t:1,r:0,b:0",
                 { "n:Window,w:fill", "n:Tag,w:6", "n:Creation,w:20" },
                 TreeViewFlags::Searchable))
                .IsValid(),
          false,
          "");
    windowTree->Handlers()->OnCurrentItemChanged = this;
    rPanel                                       = Factory::Panel::Create(windowSplit, "r:50%,t:0,b:0");
    Factory::Label::Create(rPanel, "Window notes", "l:1,t:0,w:14");
    rPanelWindowNameLabel = Factory::Label::Create(rPanel, "Not selected", "l:15,t:0,w:30");

    CHECK((notesTree =
                 Factory::TreeView::Create(rPanel, "l:0,t:1,r:0,b:2", { "n:Notes,w:fill" }, TreeViewFlags::Searchable))
                .IsValid(),
          false,
          "");

    CHECK((btnCopyNote = Factory::Button::Create(rPanel, "&Copy", "l:1,b:0,w:6", BUTTON_ID_COPY_NOTE)).IsValid(),
          false,
          "");
    CHECK((btnEditNote = Factory::Button::Create(rPanel, "&Edit", "l:7,b:0,w:8", BUTTON_ID_EDIT_NOTE)).IsValid(),
          false,
          "");
    CHECK((btnDeleteNote = Factory::Button::Create(rPanel, "&Delete", "l:16,b:0,w:9", BUTTON_ID_REMOVE_NOTE)).IsValid(),
          false,
          "");
    btnCopyNote->SetEnabled(false);
    btnEditNote->SetEnabled(false);
    btnDeleteNote->SetEnabled(false);

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
            if (rel.contains(info.referal))
            {
                continue;
            }

            // add this item
            Process(rel, handle, TreeView::RootHandle);
        }
    }

    UpdateButtonsStatus();

    windowTree->GetCurrentItem().UnfoldAll();
    windowTree->GetItemByHandle(this->focusedItem).SetCurrent();

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
