#include "Internal.h"

using namespace AppCUI::Internal;
using namespace AppCUI::Utils;
using namespace AppCUI::Input;


void CommandBarController::ShiftKeyIndicator::Init(const char * _name)
{
    AppCUI::Console::Size sz;

    this->Visible = false;
    this->Name = _name;
    this->X = 0;
    this->Width = String::Len(_name);
}

void CommandBarController::Init(unsigned int desktopWidth,unsigned int desktopHeight,  AppCUI::Application::Config * cfg, bool visible)
{
    this->Cfg = cfg;
    Visible = visible;
    SetDesktopSize(desktopWidth, desktopHeight);
	CurrentVersion = 0xFFFFFFFF;
	Clear();
	CurrentShiftKey = AppCUI::Input::Key::None;
	PressedField = nullptr;
    HoveredField = nullptr;
	LastCommand = 0;
    

    // init shift keys
    Ctrl.Init("Ctrl");
    Alt.Init("Alt");
    Shift.Init("Shift");
}
void CommandBarController::SetDesktopSize(unsigned int desktopWidth, unsigned int desktopHeight)
{
    if (!Visible)
        return;
    this->BarLayout.Width = desktopWidth;
    this->BarLayout.Y = desktopHeight - 1;
    this->RecomputeScreenPos = true;
}
void CommandBarController::Clear()
{
	CurrentVersion++;
	if (CurrentVersion == 0)
	{
		// curat toate campurile
		for (int tr = 0; tr < MAX_COMMANDBAR_SHIFTSTATES; tr++)
		{
            CommandBarField *b = &Fields[tr][0];
            CommandBarField *e = b + Key::Count;
			while (b < e) {
				b->Version = 0;
				b++;
			}
		}
		// Versiunea nu poate fi niciodata 0 (ala e doar punct de reset - este tot timpul minim 1
		CurrentVersion = 1;
	}
	for (int tr = 0; tr < MAX_COMMANDBAR_SHIFTSTATES; tr++) {
		HasKeys[tr] = false;
		IndexesCount[tr] = 0;
	}
	RecomputeScreenPos = true;
}
bool CommandBarController::Set(AppCUI::Input::Key::Type keyCode, const char* Name, int Command)
{
	CHECK(Name != nullptr, false, "Accelator name should not be nullptr !");
	CHECK(Command >= 0, false, "Command should be bigger or equal to 0");
	CHECK(keyCode > 0, false, "Key code should be bigger than 0");
	unsigned int index = (keyCode & 0xFF);
	unsigned int shift = (keyCode >> AppCUI::Utils::KeyUtils::KEY_SHIFT_BITS);
	CHECK(index < AppCUI::Input::Key::Count, false, "Invalid key code !");
	CHECK((shift>=0) && (shift < MAX_COMMANDBAR_SHIFTSTATES), false, "Invalid shift combination !");
	
    CommandBarField *b = &Fields[shift][index];
	b->Command = Command;
	b->KeyCode = keyCode;
	b->Version = CurrentVersion;
	const char* nm = Name;
	char* s = &b->Name[0];
	char* e = s + (MAX_COMMANDBAR_FIELD_NAME-1);
	while ((s < e) && ((*nm) != 0))
	{
		(*s) = (*nm);
		s++;
		nm++;
	}
	(*s) = 0;
    // Precompute text sizes
    b->KeyName = AppCUI::Utils::KeyUtils::GetKeyName(b->KeyCode);
    b->NameWidth = String::Len(b->Name);
    b->KeyNameWidth = String::Len(b->KeyName);
	HasKeys[shift] = true;
	RecomputeScreenPos = true;
	return true;
}
void CommandBarController::Paint(AppCUI::Console::Renderer & renderer)
{
    if (!Visible)
        return;
    
    renderer.FillHorizontalLineSize(0, BarLayout.Y, BarLayout.Width, ' ', Cfg->CommandBar.BackgroundColor);
	if (RecomputeScreenPos)
		ComputeScreenPos();
	unsigned int shift = CurrentShiftKey >> AppCUI::Utils::KeyUtils::KEY_SHIFT_BITS;
	if (shift >= MAX_COMMANDBAR_SHIFTSTATES)
		return;
	if (HasKeys[shift] == false)
		return;
	
	CommandBarFieldIndex *bi = &VisibleFields[shift][0];
	CommandBarFieldIndex *ei = bi + IndexesCount[shift];
    CommandBarField *cmd;
    auto * colCfg = &this->Cfg->CommandBar.Normal;

    // Shift States
    //if (Ctrl.Visible)
    //    DRAW_SHIFT_KEY(Ctrl);
    //if (Alt.Visible)
    //    DRAW_SHIFT_KEY(Alt);
    //if (Shift.Visible)
    //    DRAW_SHIFT_KEY(Shift);

	while (bi < ei)
	{
		cmd = bi->Field;
        if (cmd == this->PressedField)
            colCfg = &this->Cfg->CommandBar.Pressed;
        else if (cmd == this->HoveredField)
            colCfg = &this->Cfg->CommandBar.Hover;
        else 
            colCfg = &this->Cfg->CommandBar.Normal;

        renderer.WriteSingleLineText(cmd->StartScreenPos + 1, BarLayout.Y, cmd->KeyName, colCfg->KeyColor, cmd->KeyNameWidth);
        renderer.WriteSingleLineText(cmd->StartScreenPos + 2 + cmd->KeyNameWidth, BarLayout.Y, cmd->Name, colCfg->NameColor, cmd->NameWidth);

        bi++;
	}
}
void CommandBarController::ComputeScreenPos()
{
    if (!Visible)
        return;

    int startPoz = 0;
    // validez shift state
    Ctrl.Visible = ((this->CurrentShiftKey & Key::Ctrl) != 0);
    Shift.Visible = ((this->CurrentShiftKey & Key::Shift) != 0);
    Alt.Visible = ((this->CurrentShiftKey & Key::Alt) != 0);
    if (Ctrl.Visible)
    {
        Ctrl.X = startPoz;
        startPoz += (Ctrl.Width + 4);
    }
    if (Alt.Visible)
    {
        Alt.X = startPoz;
        startPoz += (Alt.Width + 4);
    }
    if (Shift.Visible)
    {
        Shift.X = startPoz;
        startPoz += (Shift.Width + 4);
    }
    if (Ctrl.Visible || Shift.Visible || Alt.Visible)
        startPoz += 12;
	// creez lista secundara de pointeri
	bool *hasKeys = &HasKeys[0];
	for (int tr = 0; tr < MAX_COMMANDBAR_SHIFTSTATES; tr++,hasKeys++)
	{
		if ((*hasKeys) == false)
			continue;
		CommandBarField *bf = &Fields[tr][0];
		CommandBarField *ef = bf + Key::Count;
		CommandBarFieldIndex *current = &VisibleFields[tr][0];
		int* ic = &IndexesCount[tr];
		*ic = 0;
        int start = startPoz;
		while (bf<ef)
		{
			if (bf->Version == CurrentVersion) {
				current->Field = bf;
				current++;
				bf->StartScreenPos = start;
                start += bf->KeyNameWidth + bf->NameWidth + 3; // One space (to separate key from name) and 2 spaces (one before the key and one after)
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
bool CommandBarController::SetShiftKey(AppCUI::Input::Key::Type keyCode)
{
	if (keyCode != CurrentShiftKey)
	{
		CurrentShiftKey = keyCode;
        if (Visible)
            ComputeScreenPos();
		return true;
	}
	return false;
}
CommandBarField*	CommandBarController::MousePositionToField(int x, int y)
{
    if (!Visible)
        return nullptr;
	if (RecomputeScreenPos)
		ComputeScreenPos();
	unsigned int shift = CurrentShiftKey >> AppCUI::Utils::KeyUtils::KEY_SHIFT_BITS;
	CHECK(shift < MAX_COMMANDBAR_SHIFTSTATES, nullptr, "");
	if (HasKeys[shift] == false)
		return nullptr;
    if (y < this->BarLayout.Y)
        return nullptr;
	CommandBarFieldIndex *bi = &VisibleFields[shift][0];
	CommandBarFieldIndex *ei = bi + IndexesCount[shift];
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
bool CommandBarController::OnMouseOver(int x, int y, bool & repaint)
{
    repaint = false;
    if (!Visible)
        return false;
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
        repaint = true;
    }
    return true;
}
bool CommandBarController::OnMouseDown()
{
    if (!Visible)
        return false;
    if (this->HoveredField)
    {
        this->PressedField = this->HoveredField;
        return true;
    }
    return false;
}
bool CommandBarController::OnMouseUp(int & command)
{
    if (!Visible)
        return false;
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
int  CommandBarController::GetCommandForKey(AppCUI::Input::Key::Type keyCode)
{
	unsigned int index = (keyCode & 0xFF);
	unsigned int shift = (keyCode >> AppCUI::Utils::KeyUtils::KEY_SHIFT_BITS);
	CHECK(index < AppCUI::Input::Key::Count, -1, "Invalid key code !");
	CHECK((shift >= 0) && (shift < MAX_COMMANDBAR_SHIFTSTATES), -1, "Invalid shift combination !");
	CommandBarField *b = &Fields[shift][index];
	// verific daca e setat
	if (b->Version != CurrentVersion)
		return -1;
	return b->Command;
}