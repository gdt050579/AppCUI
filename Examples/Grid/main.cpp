#include "AppCUI.hpp"
#include <random>

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
              this, "d:c,w:100%,h:100%", 10, 14, AppCUI::Controls::GridFlags::None);

        auto generator        = std::bind(std::uniform_int_distribution<>(0, 1), std::default_random_engine());
        const auto dimensions = grid->GetGridDimensions();
        for (auto i = 0U; i < dimensions.Width; i++)
        {
            for (auto j = 0U; j < dimensions.Height; j++)
            {
                auto cellType = AppCUI::Controls::Grid::CellType::String;

                if (generator())
                {
                    cellType = AppCUI::Controls::Grid::CellType::Boolean;
                }

                switch (cellType)
                {
                case AppCUI::Controls::Grid::CellType::Boolean:
                    grid->UpdateCell(
                          i, j, cellType, static_cast<bool>(generator()), AppCUI::Graphics::TextAlignament::Center);
                    break;
                case AppCUI::Controls::Grid::CellType::String:
                {
                    AppCUI::Utils::LocalString<32> value;
                    grid->UpdateCell(
                          i,
                          j,
                          cellType,
                          value.Format("%u | %u -> %u", i, j, dimensions.Width * j + i),
                          AppCUI::Graphics::TextAlignament::Center);
                }
                break;
                default:
                    break;
                }
            }
        }

        grid->UpdateCell(
              0,
              AppCUI::Controls::Grid::CellType::String,
              "testing_super_long_cell_content \nwith spaces and \nnewlines",
              AppCUI::Graphics::TextAlignament::Center);

        grid->UpdateHeaderValues(
              { "test01", "test02", "test03", "test04", "test05_super_long_header_value" },
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
