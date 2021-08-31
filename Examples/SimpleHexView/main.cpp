#include "AppCUI.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <istream>

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;
namespace fs = std::filesystem;

constexpr int COMMAND_ID_BTN_OPEN = 1;

class HexViewUserControl : public UserControl
{
  public:
    HexViewUserControl()
    {
        resetAll();
    }

    bool Open(const fs::path& filePath)
    {
        resetAll();
        return readAll(filePath);
    }

    void Paint(Graphics::Renderer& renderer) override
    {
        renderer.Clear(' ', ColorPair{ Color::White, Color::Black });

        const std::uint64_t height     = GetHeight();
        const std::uint64_t width      = GetWidth();
        const std::uint64_t bufferSize = height * width;
        const std::uint64_t maxProcess = std::min(bufferSize, fileSize - filePosition);

        for (size_t i = 0; i < maxProcess; i++)
        {
            const int y = i / width;
            const int x = i % width;
            renderer.WriteCharacter(x, y, fileData[filePosition + i], ColorPair{ Color::White, Color::Transparent });
        }
    }

    bool OnKeyEvent(AppCUI::Input::Key keyCode, char16_t /*AsciiCode*/) override
    {
        const std::uint64_t bufferSize = GetWidth() * GetHeight();

        switch (keyCode)
        {
        case Input::Key::PageDown:
            if (filePosition + bufferSize < fileSize)
            {
                filePosition += bufferSize;
            }
            return true;
        case Input::Key::PageUp:
            if (filePosition < bufferSize)
            {
                filePosition = 0;
            }
            else
            {
                filePosition -= bufferSize;
            }
            return true;
        default:
            break;
        }
        return false;
    }

  private:
    void resetAll()
    {
        fileData.clear();
        fileSize     = 0;
        filePosition = 0;
    }

    bool readAll(const fs::path& filePath)
    {
        CHECK(fs::exists(filePath), false, "File %s doesn't exist", filePath.string().c_str());
        fileSize = fs::file_size(filePath);

        std::ifstream fileStream(filePath);
        fileData.reserve(fileSize);
        std::copy(
              std::istream_iterator<unsigned char>(fileStream),
              std::istream_iterator<unsigned char>(),
              std::back_inserter(fileData));

        return true;
    }

  private:
    std::vector<unsigned char> fileData;
    uint64_t filePosition;
    uint64_t fileSize;
};

class SimpleHexView : public AppCUI::Controls::Window
{
  private:
    Button btnOpenFile;
    HexViewUserControl hexView;

  public:
    SimpleHexView()
    {
        this->Create("SimpleHexView", "w:100%,h:100%");
        btnOpenFile.Create(this, "Open File ...", "a:c,t:0,w:100%,h:1", COMMAND_ID_BTN_OPEN);
        hexView.Create(this, "t:1,w:100%,h:100%");
    }

    bool OnEvent(Control* /*sender*/, Event eventType, int controlID) override
    {
        if (eventType == Event::EVENT_WINDOW_CLOSE)
        {
            Application::Close();
            return true;
        }

        if (eventType == Event::EVENT_BUTTON_CLICKED && controlID == COMMAND_ID_BTN_OPEN)
        {
            auto path = Dialogs::FileDialog::ShowOpenFileWindow("", "", ".");
            if (path.has_value())
            {
                hexView.Open(path.value());
                hexView.SetFocus();
            }
        }
        return false;
    }
};

int main()
{
    if (!Application::Init(InitializationFlags::CommandBar))
        return 1;
    auto window = new SimpleHexView();
    Application::AddWindow(window);
    Application::Run();
    delete window;
    return 0;
}
