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
              this, "d:c,w:100%,h:100%", 10, 18, AppCUI::Controls::GridFlags::None);

        auto generator        = std::bind(std::uniform_int_distribution<>(0, 1), std::default_random_engine());
        const auto dimensions = grid->GetGridDimensions();
        for (auto i = 0U; i < dimensions.first; i++)
        {
            for (auto j = 0U; j < dimensions.second; j++)
            {
                const auto cellIndex = dimensions.first * j + i;
                auto cellType        = AppCUI::Controls::Grid::CellType::String;

                if (generator())
                {
                    cellType = AppCUI::Controls::Grid::CellType::Boolean;
                }

                switch (cellType)
                {
                case AppCUI::Controls::Grid::CellType::Boolean:
                    grid->UpdateCell(cellIndex, { cellType, static_cast<bool>(generator()) });
                    break;
                case AppCUI::Controls::Grid::CellType::String:
                {
                    AppCUI::Utils::LocalString<32> value;
                    value.Format("%u | %u -> %u", i, j, cellIndex);
                    grid->UpdateCell(cellIndex, { cellType, value });
                }
                break;
                default:
                    break;
                }
            }
        }

        grid->UpdateHeaderValues({ "test01", "test02", "test03", "test04" });
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
