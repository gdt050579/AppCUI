#include "Internal.hpp"
#include <chrono>

namespace AppCUI::Graphics
{
using namespace Internal;
using namespace std::chrono;

constexpr uint32 PROGRESS_STATUS_PANEL_WIDTH         = 60;
constexpr uint32 PROGRESS_STATUS_PANEL_HEIGHT        = 8;
constexpr uint64 MIN_SECONDS_BEFORE_SHOWING_PROGRESS = 2;
constexpr uint32 MAX_PROGRESS_TIME_TEXT              = 35;

// remove MessageBox definition that comes with Windows.h header
#ifdef MessageBox
#    undef MessageBox
#endif

struct ProgressStatusData
{
    CharacterBuffer Title;
    CharacterBuffer Text;

    uint64 MaxValue;
    bool Showed;
    time_point<steady_clock> StartTime;
    uint64 Ellapsed, LastEllapsed;
    Clip WindowClip;
    ApplicationImpl* App;
    char progressString[5];
    Utils::FixSizeString<MAX_PROGRESS_TIME_TEXT> timeStr;
    uint32 Progress;
};

static ProgressStatusData PSData = {};
static bool progress_inited      = false;

void ProgressStatus_Paint_Panel()
{
    auto canvas = &PSData.App->terminal->ScreenCanvas;

    canvas->DarkenScreen();
    ;
    canvas->SetAbsoluteClip(PSData.WindowClip);
    canvas->SetTranslate(PSData.WindowClip.ScreenPosition.X, PSData.WindowClip.ScreenPosition.Y);
    canvas->HideCursor();

    canvas->Clear(' ', { Color::Black, PSData.App->config.Window.Background.Normal });
    canvas->DrawRectSize(
          0,
          0,
          PROGRESS_STATUS_PANEL_WIDTH,
          PROGRESS_STATUS_PANEL_HEIGHT,
          PSData.App->config.Border.Focused,
          LineType::Single);
    WriteTextParams params(
          WriteTextFlags::SingleLine | WriteTextFlags::OverwriteColors | WriteTextFlags::ClipToWidth |
                WriteTextFlags::LeftMargin | WriteTextFlags::RightMargin,
          TextAlignament::Center);
    params.X     = 5;
    params.Y     = 0;
    params.Color = PSData.App->config.Text.Highlighted;
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

    canvas->FillHorizontalLine(2, 3, PROGRESS_STATUS_PANEL_WIDTH - 3, ' ', PSData.App->config.ProgressStatus.Empty);
    canvas->WriteSingleLineText(2, 2, PSData.Text, PSData.App->config.Text.Normal);

    if (PSData.MaxValue > 0)
    {
        canvas->WriteSingleLineText(
              PROGRESS_STATUS_PANEL_WIDTH - 6,
              2,
              string_view(PSData.progressString, 4),
              PSData.App->config.Text.Highlighted);
        canvas->FillHorizontalLine(
              2,
              3,
              ((PSData.Progress * (PROGRESS_STATUS_PANEL_WIDTH - 4)) / 100) + 1,
              ' ',
              PSData.App->config.ProgressStatus.Full);
        canvas->WriteSingleLineText(2, 4, "ETA:", PSData.App->config.Text.Normal);
    }
    else
    {
        canvas->WriteSingleLineText(2, 4, "Ellapsed:", PSData.App->config.Text.Normal);
    }
    canvas->WriteSingleLineText(
          (PROGRESS_STATUS_PANEL_WIDTH - 2) - (uint32) PSData.timeStr.Len(),
          4,
          PSData.timeStr,
          PSData.App->config.Text.Highlighted);

    PSData.App->terminal->Update();
}
void ProgressStatus_ComputeTime(uint64 time)
{
    if (time == 0)
    {
        PSData.timeStr.Clear();
        return;
    }
    uint32 days  = (uint32) (time / (24 * 60 * 60));
    time         = time % (24 * 60 * 60);
    uint32 hours = (uint32) (time / (60 * 60));
    time         = time % (60 * 60);
    uint32 min   = (uint32) (time / 60);
    uint32 sec   = (uint32) (time % 60);
    if (days >= 10)
    {
        PSData.timeStr = "More than 10 days";
        return;
    }
    char temp[128];
    char* p = temp;
    if (days > 0)
    {
        *p++ = '0' + days;
        *p++ = 'd';
        *p++ = ' ';
    }
    *p++           = '0' + hours / 10;
    *p++           = '0' + hours % 10;
    *p++           = ':';
    *p++           = '0' + min / 10;
    *p++           = '0' + min % 10;
    *p++           = ':';
    *p++           = '0' + sec / 10;
    *p++           = '0' + sec % 10;
    PSData.timeStr = { (const char*) temp, (size_t) (p - temp) };
}
void ProgressStatus::Init(const ConstString& Title, uint64 maxValue)
{
    Size appSize = { 0, 0 };
    Application::GetApplicationSize(appSize);
    PSData.MaxValue = maxValue;
    PSData.Showed   = false;
    PSData.WindowClip.Reset();
    PSData.WindowClip.Set(
          ((int32) appSize.Width - (int32) PROGRESS_STATUS_PANEL_WIDTH) / 2,
          ((int32) appSize.Height - (int32) PROGRESS_STATUS_PANEL_HEIGHT) / 2,
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
    PSData.timeStr.Clear();
    if (PSData.Title.Set(Title) == false)
    {
        LOG_WARNING("Fail to set title for progress status object !");
        PSData.Title.Clear();
    }
    PSData.App->RepaintStatus = REPAINT_STATUS_ALL; // once the progress is over, all screen will be re-drawn
    PSData.StartTime          = std::chrono::steady_clock::now();
    progress_inited           = true;
}
bool __ProgressStatus_Update(uint64 value, const ConstString* content)
{
    CHECK(progress_inited,
          true,
          "Progress status was not initialized ! Have you called ProgressStatus::Init(...) before calling "
          "ProgressStatus::Update(...) ?");
    uint32 newProgress;
    bool showStatus = false;
    if (PSData.MaxValue > 0)
    {
        if (value < PSData.MaxValue)
            newProgress = (uint32) ((value * (uint64) 100) / PSData.MaxValue);
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
                          (((uint64) (100 - PSData.Progress)) * PSData.Ellapsed) / ((uint64) PSData.Progress));
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

bool ProgressStatus::Update(uint64 value, const ConstString& content)
{
    return __ProgressStatus_Update(value, &content);
}
bool ProgressStatus::Update(uint64 value)
{
    return __ProgressStatus_Update(value, nullptr);
}
} // namespace AppCUI::Graphics