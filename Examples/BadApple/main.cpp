#include "AppCUI.hpp"
#include <chrono>
#include <thread>
#include <time.h>

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Dialogs;
using namespace AppCUI::Graphics;

using namespace std::string_literals;

constexpr unsigned int VIDEO_WIDTH    = 100;
constexpr unsigned int VIDEO_HEIGHT   = 30;
constexpr unsigned int BUTTON_ID_PLAY = 1;
constexpr unsigned int BUTTON_ID_STOP = 2;

class VideoScreen : public AppCUI::Controls::UserControl
{
  public:
    VideoScreen(std::string_view layout) : UserControl(layout), painitng(false), currentFrame(0)
    {
    }

    void Paint(Graphics::Renderer& renderer) override
    {
        if (currentFrame < frames.size())
        {
            renderer.Clear(' ', { Color::Transparent, Color::Transparent });
            unsigned int y = 0;
            for (const auto& line : frames[currentFrame])
            {
                renderer.WriteSingleLineText(0, y, line, { Color::White, Color::Transparent });
                ++y;
            }

            if (painitng)
            {
                currentFrame++;
            }
        }
    }

    void SetFrames(const std::vector<std::vector<std::string>>& _frames)
    {
        frames = _frames;
    }

    void SetPaining(const bool _painting)
    {
        painitng = _painting;
    }

  private:
    bool painitng;
    size_t currentFrame;
    std::vector<std::vector<std::string>> frames;
};

class BadApple : public AppCUI::Controls::Window
{
  public:
    BadApple() : Window("ASCII Player", "d:c,w:100,h:32", WindowFlags::None)
    {
        Factory::Button::Create(this, "Play", "x:0,y:0,w:50%,h:2", BUTTON_ID_PLAY, ButtonFlags::None);
        Factory::Button::Create(this, "Stop", "x:50%,y:0,w:50%,h:2", BUTTON_ID_STOP, ButtonFlags::None);
        auto videoScreen = std::make_unique<VideoScreen>("x:0,y:2,h:30,w:100");

        myVideo = videoScreen.get();
        AddChildControl(std::move(videoScreen));

        const auto maybeFile = AppCUI::Dialogs::FileDialog::ShowOpenFileWindow("", "Text Files:txt", "");
        if (!maybeFile.has_value())
        {
            MessageBox::ShowError("Error", "Frames file not loaded");
            return;
        }
        if (!txtFile.OpenRead(maybeFile.value()))
        {
            MessageBox::ShowError("Error", "Can't open file for reading");
        }
        ReadAllFrames();
    }

    void ReadAllFrames()
    {
        unsigned int bytesRead            = 0;
        char videoBuffer[VIDEO_WIDTH + 1] = { 0 };
        do
        {
            std::vector<std::string> frame;
            for (unsigned int i = 0; i < VIDEO_HEIGHT; i++)
            {
                txtFile.ReadBuffer(videoBuffer, VIDEO_WIDTH, bytesRead);
                videoBuffer[VIDEO_WIDTH] = 0;
                frame.push_back(videoBuffer);
                if (bytesRead == 0)
                    break;
            }
            frames.push_back(frame);
        } while (bytesRead != 0);
        myVideo->SetFrames(frames);
    }

    bool OnEvent(AppCUI::Controls::Control*, Event eventType, int controlID) override
    {
        if (eventType == Event::WindowClose)
        {
            Application::Close();
            return true;
        }
        if (eventType == Event::ButtonClicked)
        {
            if (controlID == BUTTON_ID_PLAY)
            {
                myVideo->SetPaining(true);
            }
            else if (controlID == BUTTON_ID_STOP)
            {
                myVideo->SetPaining(false);
            }
            return true;
        }
        return false;
    }

    Reference<VideoScreen> myVideo;
    OS::File txtFile;
    std::filesystem::path filePath;
    std::vector<std::vector<std::string>> frames;
};

int main()
{
    if (!Application::Init(InitializationFlags::Maximized | InitializationFlags::AutoRedraw))
        return 1;
    Application::AddWindow(std::make_unique<BadApple>());
    Application::Run();
    return 0;
}
