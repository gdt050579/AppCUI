#include "AppCUI.hpp"

class SimpleWin : public AppCUI::Controls::Window
{
  public:
    SimpleWin()
        : Window(
                "Grid Sample",
                "d:c,w:100%,h:100%",
                AppCUI::Controls::WindowFlags::NoCloseButton | AppCUI::Controls::WindowFlags::FixedPosition |
                      AppCUI::Controls::WindowFlags::Maximized)
    {
        auto grid = AppCUI::Controls::Factory::Grid::Create(
              this, "d:c,w:100%,h:100%", 20, 20, AppCUI::Controls::GridFlags::None);

        const auto dimensions = grid->GetGridDimensions();
        for (auto i = 0U; i < dimensions.Width; i++)
        {
            for (auto j = 0U; j < dimensions.Height; j++)
            {
                AppCUI::Utils::LocalString<32> value;
                grid->UpdateCell(
                      i,
                      j,
                      value.Format("%u | %u -> %u", i, j, dimensions.Width * j + i),
                      AppCUI::Graphics::TextAlignament::Center);
            }
        }

        grid->UpdateCell(
              0,
              "testing_super_long_cell_content \nwith spaces and \nnewlines",
              AppCUI::Graphics::TextAlignament::Center);

        grid->UpdateHeaderValues(
              { "test01",
                "test02",
                "test03",
                "test04",
                "test05_super_long_header_value",
                "testing_super_long_cell_content \nwith spaces and \nnewlines" },
              AppCUI::Graphics::TextAlignament::Center);
    }
};

int main()
{
    if (AppCUI::Application::Init(
              AppCUI::Application::InitializationFlags::Maximized |
              AppCUI::Application::InitializationFlags::FixedSize) == false)
    {
        return 1;
    }

    AppCUI::Application::AddWindow(std::make_unique<SimpleWin>());
    AppCUI::Application::Run();

    return 0;
}
