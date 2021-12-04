#include "Internal.hpp"
#include <chrono>

namespace AppCUI
{
using namespace Graphics;
using namespace Internal;
using namespace std::chrono;

#define MAX_PROGRESS_STATUS_TITLE           36
#define MAX_PROGRESS_STATUS_TEXT            52
#define PROGRESS_STATUS_PANEL_WIDTH         60
#define MAX_PROGRESS_TIME_TEXT              50
#define PROGRESS_STATUS_PANEL_HEIGHT        8
#define MIN_SECONDS_BEFORE_SHOWING_PROGRESS 2

// remove MessageBox definition that comes with Windows.h header
#ifdef MessageBox
#    undef MessageBox
#endif

struct ProgressStatusData
{
    CharacterBuffer Title;
    CharacterBuffer Text;

    unsigned long long MaxValue;
    bool Showed;
    time_point<steady_clock> StartTime;
    unsigned long long Ellapsed, LastEllapsed;
    Clip WindowClip;
    ApplicationImpl* App;
    char progressString[5];
    char timeString[MAX_PROGRESS_TIME_TEXT];
    int timeStringSize;
    unsigned int Progress;
};

static ProgressStatusData PSData = {};
bool progress_inited             = false;

void ProgressStatus_Paint_Panel()
{
    auto canvas = &PSData.App->terminal->ScreenCanvas;

    canvas->DarkenScreen();
    canvas->SetAbsoluteClip(PSData.WindowClip);
    canvas->SetTranslate(PSData.WindowClip.ScreenPosition.X, PSData.WindowClip.ScreenPosition.Y);
    canvas->HideCursor();

    canvas->Clear(' ', PSData.App->config.ProgressStatus.Border);
    canvas->DrawRectSize(
          0,
          0,
          PROGRESS_STATUS_PANEL_WIDTH,
          PROGRESS_STATUS_PANEL_HEIGHT,
          PSData.App->config.ProgressStatus.Border,
          false);
    WriteTextParams params(
          WriteTextFlags::SingleLine | WriteTextFlags::OverwriteColors | WriteTextFlags::ClipToWidth |
                WriteTextFlags::LeftMargin | WriteTextFlags::RightMargin,
          TextAlignament::Center);
    params.X     = 5;
    params.Y     = 0;
    params.Color = PSData.App->config.ProgressStatus.Title;
    params.Width = PROGRESS_STATUS_PANEL_WIDTH - 10;
    canvas->WriteText(PSData.Title, params);

    PSData.Showed = true;
    PSData.App->terminal->Update();
}

void ProgressStatus_Paint_Status()
{
    auto canvas = &PSData.App->terminal->ScreenCanvas;
    canvas->SetAbsoluteClip(PSData.WindowClip);
    canvas->SetTranslate(PSData.WindowClip.ScreenPosition.X, PSData.WindowClip.ScreenPosition.Y);

    canvas->FillHorizontalLine(
          2, 3, PROGRESS_STATUS_PANEL_WIDTH - 3, ' ', PSData.App->config.ProgressStatus.EmptyProgressBar);
    canvas->WriteSingleLineText(2, 2, PSData.Text, PSData.App->config.ProgressStatus.Text);

    if (PSData.MaxValue > 0)
    {
        canvas->WriteSingleLineText(
              PROGRESS_STATUS_PANEL_WIDTH - 6,
              2,
              std::string_view(PSData.progressString, 4),
              PSData.App->config.ProgressStatus.Percentage);
        canvas->FillHorizontalLine(
              2,
              3,
              ((PSData.Progress * (PROGRESS_STATUS_PANEL_WIDTH - 4)) / 100) + 1,
              ' ',
              PSData.App->config.ProgressStatus.FullProgressBar);
        canvas->WriteSingleLineText(2, 4, "ETA:", PSData.App->config.ProgressStatus.Text);
    }
    else
    {
        canvas->WriteSingleLineText(2, 4, "Ellapsed:", PSData.App->config.ProgressStatus.Text);
    }
    canvas->WriteSingleLineText(
          (PROGRESS_STATUS_PANEL_WIDTH - 2) - PSData.timeStringSize,
          4,
          std::string_view(PSData.timeString, PSData.timeStringSize),
          PSData.App->config.ProgressStatus.Time);

    PSData.App->terminal->Update();
}
void ProgressStatus_ComputeTime(unsigned long long time)
{
    if (time == 0)
    {
        PSData.timeString[0]  = 0;
        PSData.timeStringSize = 0;
        return;
    }
    unsigned int days  = (unsigned int) (time / (24 * 60 * 60));
    time               = time % (24 * 60 * 60);
    unsigned int hours = (unsigned int) (time / (60 * 60));
    time               = time % (60 * 60);
    unsigned int min   = (unsigned int) (time / 60);
    unsigned int sec   = (unsigned int) (time % 60);
    if (days >= 10)
    {
        Utils::String::Set(PSData.timeString, "More than 10 days", MAX_PROGRESS_TIME_TEXT);
        PSData.timeStringSize = 17;
        return;
    }
    int index = 0;
    if (days > 0)
    {
        PSData.timeString[0] = '0' + days;
        PSData.timeString[1] = 'd';
        PSData.timeString[2] = ' ';
        index                = 3;
    }
    PSData.timeString[index++] = '0' + hours / 10;
    PSData.timeString[index++] = '0' + hours % 10;
    PSData.timeString[index++] = ':';
    PSData.timeString[index++] = '0' + min / 10;
    PSData.timeString[index++] = '0' + min % 10;
    PSData.timeString[index++] = ':';
    PSData.timeString[index++] = '0' + sec / 10;
    PSData.timeString[index++] = '0' + sec % 10;
    PSData.timeString[index]   = 0;
    PSData.timeStringSize      = index;
}
void ProgressStatus::Init(const Utils::ConstString& Title, unsigned long long maxValue)
{
    Size appSize = { 0, 0 };
    Application::GetApplicationSize(appSize);
    PSData.MaxValue = maxValue;
    PSData.Showed   = false;
    PSData.WindowClip.Reset();
    PSData.WindowClip.Set(
          (appSize.Width - PROGRESS_STATUS_PANEL_WIDTH) / 2,
          (appSize.Height - PROGRESS_STATUS_PANEL_HEIGHT) / 2,
          PROGRESS_STATUS_PANEL_WIDTH,
          PROGRESS_STATUS_PANEL_HEIGHT);
    PSData.App               = Application::GetApplication();
    PSData.progressString[0] = ' ';
    PSData.progressString[1] = ' ';
    PSData.progressString[2] = '0';
    PSData.progressString[3] = '%';
    PSData.progressString[4] = 0;
    PSData.Progress          = 0;
    PSData.Ellapsed          = 0;
    PSData.LastEllapsed      = 0;
    PSData.timeString[0]     = 0;
    if (PSData.Title.Set(Title) == false)
    {
        LOG_WARNING("Fail to set title for progress status object !");
        PSData.Title.Clear();
    }
    PSData.App->RepaintStatus = REPAINT_STATUS_ALL; // once the progress is over, all screen will be re-drawn
    PSData.StartTime          = std::chrono::steady_clock::now();
    progress_inited           = true;
}
bool __ProgressStatus_Update(unsigned long long value, const Utils::ConstString* content)
{
    CHECK(progress_inited,
          true,
          "Progress status was not initialized ! Have you called ProgressStatus::Init(...) before calling "
          "ProgressStatus::Update(...) ?");
    unsigned int newProgress;
    bool showStatus = false;
    if (PSData.MaxValue > 0)
    {
        if (value < PSData.MaxValue)
            newProgress = (unsigned int) ((value * (unsigned long long) 100) / PSData.MaxValue);
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
                if (newProgress > 9)
                {
                    PSData.progressString[1] = (newProgress / 10) + '0';
                    newProgress %= 10;
                }
                else
                {
                    PSData.progressString[1] = ' ';
                }
                PSData.progressString[2] = newProgress + '0';
            }
            showStatus = true;
        }
    }
    if (content)
    {
        if (!PSData.Text.Set(*content))
            PSData.Text.Clear();
        showStatus = true;
    }
    PSData.Ellapsed =
          std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - PSData.StartTime).count();
    if (PSData.Ellapsed >= MIN_SECONDS_BEFORE_SHOWING_PROGRESS)
    {
        if (PSData.LastEllapsed != PSData.Ellapsed)
        {
            // only compute time if it's different from the previous display (meaning once per second)
            if (PSData.MaxValue > 0)
            {
                if (PSData.Progress == 0)
                    ProgressStatus_ComputeTime(0);
                else
                    ProgressStatus_ComputeTime(
                          (((unsigned long long) (100 - PSData.Progress)) * PSData.Ellapsed) /
                          ((unsigned long long) PSData.Progress));
            }
            else
                ProgressStatus_ComputeTime(PSData.Ellapsed);
            showStatus          = true;
            PSData.LastEllapsed = PSData.Ellapsed;
        }
        if (PSData.Showed == false)
        {
            ProgressStatus_Paint_Panel();
            ProgressStatus_Paint_Status();
        }
        else
        {
            // update screen if (message or progress have changed or at least one second has passed from the previous
            // screen update)
            if (showStatus)
                ProgressStatus_Paint_Status();
        }
    }

    if (PSData.App->terminal->IsEventAvailable())
    {
        Internal::SystemEvent evnt;
        bool requestQuit = false;
        // read up to 100 events
        for (int tr = 0; tr < 100; tr++)
        {
            if (PSData.App->terminal->IsEventAvailable() == false)
                break;
            PSData.App->terminal->GetSystemEvent(evnt);
            if ((evnt.eventType == Internal::SystemEventType::KeyPressed) && (evnt.keyCode == Input::Key::Escape))
            {
                requestQuit = true;
                break;
            }
        }
        if (requestQuit)
        {
            if (Dialogs::MessageBox::ShowOkCancel("Terminate", "Terminate current task ?") == Dialogs::Result::Ok)
            {
                progress_inited = false; // loop has ended
                return true;
            }
            // repaint
            ProgressStatus_Paint_Panel();
        }
    }
    return false;
}

bool ProgressStatus::Update(unsigned long long value, const Utils::ConstString& content)
{
    return __ProgressStatus_Update(value, &content);
}
bool ProgressStatus::Update(unsigned long long value)
{
    return __ProgressStatus_Update(value, nullptr);
}
} // namespace AppCUI