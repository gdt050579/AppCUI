#include "AppCUI.h"

using namespace AppCUI::Controls;
using namespace AppCUI::Dialogs;

#define MSGBOX_BUTTONS_OK				1
#define MSGBOX_BUTTONS_OK_CANCEL		2
#define MSGBOX_BUTTONS_YES_NO_CANCEL	3

bool MessageBoxWindowEventHandler(Control *control, const void* sender, AppCUI::Controls::Event eventType, int controlID,void * Context)
{
	switch (eventType)
	{
		case Event::EVENT_WINDOW_CLOSE:
			((Window*)control)->Exit((int)DialogResult::RESULT_CANCEL);
            return true;
		case Event::EVENT_BUTTON_CLICKED:
			((Window*)control)->Exit(controlID);
            return true;
        default:
            return false;
	}
    return false;
}

bool CreateMessageBoxWindow(const char* title, const char * content, WindowFlags::Type flags, int buttonsType, int *result)
{
	Label		lbInfo;
	Button		btnOK, btnCancel, btnYes, btnNo;
	Window		wnd;

	CHECK(wnd.Create(title, "a:c,w:60,h:10", flags), false, "");
	

	switch (buttonsType)
	{
	case MSGBOX_BUTTONS_OK:
		btnOK.Create(&wnd, "&Ok", "x:23,y:6,w:15", (int)DialogResult::RESULT_OK);
		btnOK.SetFocus();
		break;
	case MSGBOX_BUTTONS_OK_CANCEL:
		btnOK.Create(&wnd, "&Ok", "x:15,y:6,w:15", (int)DialogResult::RESULT_OK);
		btnCancel.Create(&wnd, "&Cancel", "x:31,y:6,w:15", (int)DialogResult::RESULT_CANCEL);
		btnOK.SetFocus();
		break;
	case MSGBOX_BUTTONS_YES_NO_CANCEL:
		btnYes.Create(&wnd, "&Yes", "x:7,y:6,w:15", (int)DialogResult::RESULT_YES);
		btnNo.Create(&wnd, "&No", "x:23,y:6,w:15", (int)DialogResult::RESULT_NO);
		btnCancel.Create(&wnd, "&Cancel", "x:39,y:6,w:15", (int)DialogResult::RESULT_CANCEL);
		btnYes.SetFocus();
		break;
	}
	lbInfo.Create(&wnd, content, "x:1,y:1,w:56,h:3");
	wnd.SetEventHandler(MessageBoxWindowEventHandler);
	if (result != nullptr)
		(*result) = wnd.Show();
	return true;
}
void MessageBox::ShowError(const char * title, const char * message)
{
	if ((title == nullptr) || (message == nullptr))
		return;
	int result;
	CreateMessageBoxWindow(title, message, WindowFlags::ERRORBOX, MSGBOX_BUTTONS_OK, &result);
}
void  MessageBox::ShowNotification(const char *title, const char *message)
{
    if ((title == nullptr) || (message == nullptr))
        return;
	int result;
	CreateMessageBoxWindow(title, message, WindowFlags::NOTIFYBOX, MSGBOX_BUTTONS_OK, &result);
}
void MessageBox::ShowWarning(const char *title, const char *message)
{
    if ((title == nullptr) || (message == nullptr))
        return;
	int result;
	CreateMessageBoxWindow(title, message, WindowFlags::WARNINGBOX, MSGBOX_BUTTONS_OK, &result);
}
DialogResult  MessageBox::ShowYesNoCancel(const char *title, const char *message)
{
	if ((title == nullptr) || (message == nullptr))
		return DialogResult::RESULT_CANCEL;
	int result;
	if (CreateMessageBoxWindow(title, message, WindowFlags::NOTIFYBOX, MSGBOX_BUTTONS_YES_NO_CANCEL, &result) == false)
		return DialogResult::RESULT_CANCEL;
	return (DialogResult)result;
}
DialogResult  MessageBox::ShowOkCancel(const char *title, const char *message)
{
    if ((title == nullptr) || (message == nullptr))
        return DialogResult::RESULT_CANCEL;
    int result;
    if (CreateMessageBoxWindow(title, message, WindowFlags::NOTIFYBOX, MSGBOX_BUTTONS_OK_CANCEL, &result) == false)
        return DialogResult::RESULT_CANCEL;
    return (DialogResult)result;
}