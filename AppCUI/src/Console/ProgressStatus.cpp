#include "Internal.h"
#include <chrono>

using namespace AppCUI::Console;
using namespace AppCUI::Internal;
using namespace std::chrono;

#define MAX_PROGRESS_STATUS_TITLE		16
#define MAX_PROGRESS_STATUS_TEXT		32
#define PROGRESS_STATUS_PANEL_WIDTH		40
#define MAX_PROGRESS_TIME_TEXT			50
#define PROGRESS_STATUS_PANEL_HEIGHT	8

// remove MessageBox definition that comes with Windows.h header
#ifdef MessageBox
#   undef MessageBox
#endif

struct ProgressStatusData
{
	char				                Title[MAX_PROGRESS_STATUS_TITLE];
	char				                Text[MAX_PROGRESS_STATUS_TEXT];
	unsigned long long	                MaxValue;
	bool				                Showed;
    time_point<steady_clock>            LastTime;
	unsigned long long	                Ellapsed,LastEllapsed;
	Clip				                WindowClip;
    Application                         *App;
	char				                progressString[5];
	char				                timeString[MAX_PROGRESS_TIME_TEXT];
	int					                timeStringSize;
	unsigned int		                Progress;
};

static ProgressStatusData PSData = { 0 };

void ProgressStatus_Paint_Panel()
{
    auto canvas = &PSData.App->terminal->ScreenCanvas;

    canvas->DarkenScreen();
    canvas->SetAbsoluteClip(PSData.WindowClip);
    canvas->HideCursor();
    
	canvas->FillRectSize(0, 0, PROGRESS_STATUS_PANEL_WIDTH, PROGRESS_STATUS_PANEL_HEIGHT,' ', PSData.App->config.Window.ActiveColor);
	canvas->DrawRectSize(0, 0, PROGRESS_STATUS_PANEL_WIDTH , PROGRESS_STATUS_PANEL_HEIGHT, PSData.App->config.Window.ActiveColor, false);
    canvas->WriteSingleLineText(5, 0, PSData.Title, PROGRESS_STATUS_PANEL_WIDTH - 10, DefaultColorPair, TextAlignament::Center);
    canvas->WriteSingleLineText(10, 6, "Hit 'ESC' to cancel", DefaultColorPair, 19);
	
	PSData.Showed = true;
    PSData.App->terminal->Update();
}

void ProgressStatus_Paint_Status()
{
    auto canvas = &PSData.App->terminal->ScreenCanvas;
    canvas->SetAbsoluteClip(PSData.WindowClip);


    
    PSData.App->terminal->Update();
}
void ProgressStatus_ComputeTime(unsigned long long time)
{
	if (time == 0)
	{
		PSData.timeString[0] = 0;
		PSData.timeStringSize = 0;
		return;
	}
	time = time / 1000; 
	unsigned int days = (unsigned int)(time / (24 * 60 * 60));
	time = time % (24 * 60 * 60);
	unsigned int hours = (unsigned int)(time / (60 * 60));
	time = time % (60 * 60);
	unsigned int min = (unsigned int)(time / 60);
	unsigned int sec = (unsigned int)(time % 60);
	if (days >= 10)
	{
		AppCUI::Utils::String::Set(PSData.timeString, "More than 10 days", MAX_PROGRESS_TIME_TEXT);
		PSData.timeStringSize = 17;
		return;
	}
	int index = 0;
	if (days > 0)
	{
		PSData.timeString[0] = '0' + days;
		PSData.timeString[1] = 'd';
		PSData.timeString[2] = ' ';
		index = 3;
	}
	PSData.timeString[index++] = '0' + hours / 10;
	PSData.timeString[index++] = '0' + hours % 10;
	PSData.timeString[index++] = ':';
	PSData.timeString[index++] = '0' + min / 10;
	PSData.timeString[index++] = '0' + min % 10;
	PSData.timeString[index++] = ':';
	PSData.timeString[index++] = '0' + sec / 10;
	PSData.timeString[index++] = '0' + sec % 10;
	PSData.timeString[index] = 0;
	PSData.timeStringSize = index;
}
void ProgressStatus::Init(const char * Title, unsigned long long maxValue)
{
    Size appSize = { 0,0 };
    Application::GetApplicationSize(appSize);
	PSData.Title[0] = 0;
	PSData.MaxValue = maxValue;
	PSData.Showed = false;	
	PSData.WindowClip.Reset();
	PSData.WindowClip.Set((appSize.Width- PROGRESS_STATUS_PANEL_WIDTH) / 2, (appSize.Height - PROGRESS_STATUS_PANEL_HEIGHT) / 2, PROGRESS_STATUS_PANEL_WIDTH, PROGRESS_STATUS_PANEL_HEIGHT);
    PSData.App = AppCUI::Application::GetApplication();
	PSData.progressString[0] = ' ';
	PSData.progressString[1] = ' ';
	PSData.progressString[2] = '0';
	PSData.progressString[3] = '%';
	PSData.progressString[4] = 0;
	PSData.Progress = 0;
	PSData.Ellapsed = 0;
	PSData.LastEllapsed = 0;
	PSData.timeString[0] = 0;
	if (Title != nullptr)
	{
		int sz = MINVALUE(Utils::String::Len(Title), MAX_PROGRESS_STATUS_TITLE - 3);
		Utils::String::Set(&PSData.Title[1], Title, MAX_PROGRESS_STATUS_TITLE, sz);
		PSData.Title[0] = ' ';
		PSData.Title[sz + 1] = ' ';
		PSData.Title[sz + 2] = 0;
	}
    PSData.App->RepaintStatus = REPAINT_STATUS_ALL; // once the progress is over, all screen will be re-drawn
    PSData.LastTime = std::chrono::steady_clock::now();
}
bool ProgressStatus::Update(unsigned long long value, const char * text)
{
	unsigned int newProgress;
	bool showStatus = false;
	if (PSData.MaxValue > 0)
	{		
		if (value < PSData.MaxValue)
			newProgress = (unsigned int)((value * 100) / PSData.MaxValue);
		else 
			newProgress = 100;
		if (newProgress != PSData.Progress)
		{
			PSData.Progress = newProgress;
			if (newProgress >= 100) 
			{ 
				PSData.progressString[0] = '1'; 
				PSData.progressString[1] = '0'; 
				PSData.progressString[2] = '0';
			}
			else
			{
				PSData.progressString[0] = ' ';
				if (newProgress>9) 
				{ 
					PSData.progressString[1] = (newProgress / 10) + '0'; 
					newProgress %= 10; 
				}
				else {
					PSData.progressString[1] = ' ';
				}
				PSData.progressString[2] = newProgress + '0';
			}			
			showStatus = true;
		}
	}
	if (text)
	{
		int sz = 0;
		char *p = &PSData.Text[0];
		while ((sz < MAX_PROGRESS_STATUS_TEXT - 1) && ((*text) != 0))
		{
			if ((*p) != (*text))
			{
				(*p) = (*text);
				showStatus = true;
			}
			p++;
			text++;
			sz++;
		}
		if ((*p) != (*text))
			showStatus = true;
		(*p) = 0;
	}
    auto currenTime = std::chrono::steady_clock::now();
    PSData.Ellapsed += std::chrono::duration_cast<std::chrono::milliseconds>(currenTime - PSData.LastTime).count();
	PSData.LastTime = currenTime;
	if ((PSData.Ellapsed - PSData.LastEllapsed) >= 1000)
	{
		if (PSData.MaxValue > 0)
		{
			if (PSData.Progress == 0)
				ProgressStatus_ComputeTime(0);
			else
				ProgressStatus_ComputeTime((((unsigned long long)(100 - PSData.Progress))*PSData.Ellapsed) / ((unsigned long long)PSData.Progress));
		}
		else
			ProgressStatus_ComputeTime(PSData.Ellapsed);
		showStatus = true;
		PSData.LastEllapsed = PSData.Ellapsed;
	}
	
	if (PSData.Showed == false)
	{
		if (PSData.Ellapsed >= 1000)
		{
			ProgressStatus_Paint_Panel();
			ProgressStatus_Paint_Status();
		}
	}
	else {
		if (showStatus)
			ProgressStatus_Paint_Status();
	}
	if (PSData.App->terminal->IsEventAvailable())
	{
        AppCUI::Internal::SystemEvents::Event evnt;
		bool requestQuit = false;		
		// read up to 100 events
		for (int tr = 0; tr < 100; tr++)
		{
			if (PSData.App->terminal->IsEventAvailable() == false)
				break;
            PSData.App->terminal->GetSystemEvent(evnt);
            if ((evnt.eventType == AppCUI::Internal::SystemEvents::KEY_PRESSED) && (evnt.keyCode == AppCUI::Input::Key::Escape))
            {
				requestQuit = true;
				break;
			}
		}
		if (requestQuit)
		{
			if (AppCUI::Dialogs::MessageBox::ShowOkCancel("Terminate","Terminate current task ?") == AppCUI::Dialogs::DialogResult::RESULT_OK)
				return true;
			// repaint
			ProgressStatus_Paint_Panel();
		}
	}
	return false;
}
