#include "Internal.hpp"

namespace AppCUI::Internal
{
using namespace Input;

CommandBarController::CommandBarController(
      uint32 desktopWidth, uint32 desktopHeight, Application::Config* cfg)
{
    this->Cfg = cfg;
    SetDesktopSize(desktopWidth, desktopHeight);
    ClearCommandUniqueID = 0;
    for (uint32 tr = 0; tr < MAX_COMMANDBAR_SHIFTSTATES; tr++)
    {
        CommandBarField* b = &Fields[tr][0];
        CommandBarField* e = b + (uint32) Key::Count;
        while (b < e)
        {
            b->ClearCommandUniqueID = ClearCommandUniqueID;
            b++;
        }
    }
    CurrentShiftKey = Input::Key::None;
    PressedField    = nullptr;
    HoveredField    = nullptr;
    LastCommand     = 0;
    ShiftStatus     = string_view("", 0);
    Clear();
}
void CommandBarController::SetDesktopSize(uint32 desktopWidth, uint32 desktopHeight)
{
    this->BarLayout.Width    = desktopWidth;
    this->BarLayout.Y        = desktopHeight - 1;
    this->RecomputeScreenPos = true;
}
void CommandBarController::Clear()
{
    // always obtain a new unique ID (whenever a clear command is called)
    // this allows us NOT to reset all of the previous command (thus being more fast)
    // as CommandBarController::Clear() is called every time Focus changes -> this technique is important for
    // performance
    ClearCommandUniqueID++;

    // Clear shift keys for fast process
    for (uint32 tr = 0; tr < MAX_COMMANDBAR_SHIFTSTATES; tr++)
    {
        HasKeys[tr]      = false;
        IndexesCount[tr] = 0;
    }
    HoveredField       = nullptr;
    RecomputeScreenPos = true;
}
bool CommandBarController::Set(Input::Key keyCode, const ConstString& caption, int Command)
{
    CHECK(Command >= 0, false, "Command should be bigger or equal to 0");
    CHECK(keyCode != Key::None, false, "Key code should be bigger than 0");
    uint32 index = (((uint32) keyCode) & 0xFF);
    uint32 shift = ((uint32) keyCode) >> ((uint32) Utils::KeyUtils::KEY_SHIFT_BITS);
    CHECK(index < (uint32) Input::Key::Count, false, "Invalid key code !");
    CHECK(shift < MAX_COMMANDBAR_SHIFTSTATES, false, "Invalid shift combination !");

    CommandBarField* b = &Fields[shift][index];
    CHECK(b->Name.Set(caption), false, "Fail to copy caption");
    CHECK(b->Name.Add(" "), false, "Fail to add extra step !");

    b->Command              = Command;
    b->KeyCode              = keyCode;
    b->ClearCommandUniqueID = ClearCommandUniqueID;
    b->KeyName              = Utils::KeyUtils::GetKeyNamePadded(b->KeyCode);
    HasKeys[shift]          = true;
    RecomputeScreenPos      = true;
    return true;
}
void CommandBarController::Paint(Graphics::Renderer& renderer)
{
    renderer.FillHorizontalLineSize(0, BarLayout.Y, BarLayout.Width, ' ', Cfg->Menu.Text.Normal);
    if (RecomputeScreenPos)
        ComputeScreenPos();

    if (ShiftStatus.length() > 0)
        renderer.WriteSingleLineText(0, BarLayout.Y, ShiftStatus, Cfg->Menu.Text.Inactive);

    uint32 shift = ((uint32) CurrentShiftKey) >> ((uint32) Utils::KeyUtils::KEY_SHIFT_BITS);
    if (shift >= MAX_COMMANDBAR_SHIFTSTATES)
        return;
    if (HasKeys[shift] == false)
        return;

    CommandBarFieldIndex* bi = &VisibleFields[shift][0];
    CommandBarFieldIndex* ei = bi + IndexesCount[shift];
    CommandBarField* cmd;

    Graphics::ColorPair colText, colHotKey;

    while (bi < ei)
    {
        cmd = bi->Field;
        if (cmd == this->PressedField)
        {
            colText = Cfg->Menu.Text.PressedOrSelected;
            colHotKey = Cfg->Menu.ShortCut.PressedOrSelected;
        }
        else if (cmd == this->HoveredField)
        {
            colText   = Cfg->Menu.Text.Hovered;
            colHotKey = Cfg->Menu.ShortCut.Hovered;
        }
        else
        {
            colText   = Cfg->Menu.Text.Normal;
            colHotKey = Cfg->Menu.ShortCut.Normal;
        }

        renderer.WriteSingleLineText(cmd->StartScreenPos, BarLayout.Y, cmd->KeyName, colHotKey);
        renderer.WriteSingleLineText(
              cmd->StartScreenPos + (int) cmd->KeyName.length(), BarLayout.Y, cmd->Name, colText);

        bi++;
    }
}
void CommandBarController::ComputeScreenPos()
{
    int startPoz;
    // validez shift state
    ShiftStatus = Utils::KeyUtils::GetKeyModifierName(this->CurrentShiftKey);
    startPoz    = (int) ShiftStatus.length();
    if (startPoz > 0)
        startPoz++;

    // creez lista secundara de pointeri
    bool* hasKeys = &HasKeys[0];
    for (uint32 tr = 0; tr < MAX_COMMANDBAR_SHIFTSTATES; tr++, hasKeys++)
    {
        if ((*hasKeys) == false)
            continue;
        CommandBarField* bf           = &Fields[tr][0];
        CommandBarField* ef           = bf + (uint32) Key::Count;
        CommandBarFieldIndex* current = &VisibleFields[tr][0];
        int* ic                       = &IndexesCount[tr];
        *ic                           = 0;
        int start                     = startPoz;
        while (bf < ef)
        {
            // we consider valid only items that were added with a specific ClearCommandUniqueID ID
            if (bf->ClearCommandUniqueID == this->ClearCommandUniqueID)
            {
                current->Field = bf;
                current++;
                bf->StartScreenPos = start;
                start += (int) (bf->KeyName.length() + bf->Name.Len());
                bf->EndScreenPos = start;

                if (start > this->BarLayout.Width)
                    break;
                (*ic)++;
            }
            bf++;
        }
    }
    this->HoveredField = nullptr;
    this->PressedField = nullptr;
    RecomputeScreenPos = false;
}
bool CommandBarController::SetShiftKey(Input::Key keyCode)
{
    if (keyCode != CurrentShiftKey)
    {
        CurrentShiftKey = keyCode;
        ComputeScreenPos();
        return true;
    }
    return false;
}
CommandBarField* CommandBarController::MousePositionToField(int x, int y)
{
    if (RecomputeScreenPos)
        ComputeScreenPos();
    uint32 shift = ((uint32) CurrentShiftKey) >> ((uint32) Utils::KeyUtils::KEY_SHIFT_BITS);
    CHECK(shift < MAX_COMMANDBAR_SHIFTSTATES, nullptr, "");
    if (HasKeys[shift] == false)
        return nullptr;
    if (y < this->BarLayout.Y)
        return nullptr;
    CommandBarFieldIndex* bi = &VisibleFields[shift][0];
    CommandBarFieldIndex* ei = bi + IndexesCount[shift];
    while (bi < ei)
    {
        if ((x >= bi->Field->StartScreenPos) && (x < bi->Field->EndScreenPos))
            return (bi->Field);
        bi++;
    }
    return nullptr;
}
bool CommandBarController::CleanFieldStatus()
{
    if ((this->HoveredField) || (this->PressedField))
    {
        this->HoveredField = nullptr;
        this->PressedField = nullptr;
        return true;
    }
    return false;
}
bool CommandBarController::OnMouseMove(int x, int y, bool& repaint)
{
    repaint = false;
    if (y < this->BarLayout.Y)
    {
        repaint = CleanFieldStatus();
        return false; // sunt in afara lui
    }
    if (this->HoveredField)
    {
        // cached position
        if ((x >= this->HoveredField->StartScreenPos) && (x < this->HoveredField->EndScreenPos))
            return true;
    }
    CommandBarField* field = MousePositionToField(x, y);
    if (field != this->HoveredField)
    {
        this->HoveredField = field;
        repaint            = true;
    }
    return true;
}
bool CommandBarController::OnMouseDown()
{
    if (this->HoveredField)
    {
        this->PressedField = this->HoveredField;
        return true;
    }
    return false;
}
bool CommandBarController::OnMouseUp(int& command)
{
    if (this->PressedField)
    {
        command = this->PressedField->Command;
        if (command < 0)
            command = -1;
        this->PressedField = nullptr;
        return true;
    }
    command = -1;
    return false;
}
int CommandBarController::GetCommandForKey(Input::Key keyCode)
{
    uint32 index = (((uint32) keyCode) & 0xFF);
    uint32 shift = (((uint32) keyCode) >> Utils::KeyUtils::KEY_SHIFT_BITS);
    CHECK(index < (uint32) Input::Key::Count, -1, "Invalid key code !");
    CHECK((shift < MAX_COMMANDBAR_SHIFTSTATES), -1, "Invalid shift combination !");
    CommandBarField* b = &Fields[shift][index];
    // if ClearCommandUniqueID is not thee same as the current one, then its an old item and we discard it
    if (b->ClearCommandUniqueID != ClearCommandUniqueID)
        return -1;
    return b->Command;
}
} // namespace AppCUI::Internal
