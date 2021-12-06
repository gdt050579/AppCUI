#include "AppCUI.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <istream>
#include <iterator>

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;
namespace fs = std::filesystem;

constexpr int COMMAND_ID_BTN_OPEN = 1;

class HexViewUserControl : public UserControl
{
  public:
    HexViewUserControl(std::string_view layout) : UserControl(layout)
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

        const uint64 height     = GetHeight();
        const uint64 width      = GetWidth();
        const uint64 bufferSize = height * width;
        const uint64 maxProcess = std::min(bufferSize, fileSize - filePosition);

        for (uint64 i = 0; i < maxProcess; i++)
        {
            const int y = static_cast<int>(i / width);
            const int x = static_cast<int>(i % width);
            renderer.WriteCharacter(x, y, fileData[filePosition + i], ColorPair{ Color::White, Color::Transparent });
        }
    }

    bool OnKeyEvent(AppCUI::Input::Key keyCode, char16_t /*AsciiCode*/) override
    {
        const uint64 bufferSize = GetWidth() * GetHeight();

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
    uint64 filePosition;
    uint64 fileSize;
};

class SimpleHexView : public AppCUI::Controls::Window
{
  private:
    Reference<HexViewUserControl> hexView;

  public:
    SimpleHexView() : Window("SimpleHexView", "x:0,y:0,w:100%,h:100%", WindowFlags::None)
    {
        Factory::Button::Create(this, "Open File ...", "l:2,t:0,r:2,h:1", COMMAND_ID_BTN_OPEN, ButtonFlags::Flat);
        hexView = this->CreateChildControl<HexViewUserControl>("x:0,y:2,w:100%,h:100%");
    }

    bool OnEvent(Reference<Control> /*sender*/, Event eventType, int controlID) override
    {
        if (eventType == Event::WindowClose)
        {
            Application::Close();
            return true;
        }

        if (eventType == Event::ButtonClicked && controlID == COMMAND_ID_BTN_OPEN)
        {
            auto path = Dialogs::FileDialog::ShowOpenFileWindow("", "", ".");
            if (path.has_value())
            {
                hexView->Open(path.value());
                hexView->SetFocus();
            }
        }
        return false;
    }
};

int main()
{
    if (!Application::Init(InitializationFlags::CommandBar | InitializationFlags::LoadSettingsFile))
        return -1;
    auto window = std::make_unique<SimpleHexView>();
    Application::AddWindow(std::move(window));
    Application::Run();
    return 0;
}
