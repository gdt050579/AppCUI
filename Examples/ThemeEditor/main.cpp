#include "AppCUI.hpp"
#include <cassert>
using namespace AppCUI;
using namespace AppCUI::Controls;
using namespace AppCUI::Dialogs;

struct WindowWithColors : public Dialogs::OnThemeChangedInterface, public Dialogs::OnThemePreviewWindowDrawInterface
{
    Application::Config::CustomColorNameStorage colors1, colors2;

    WindowWithColors()
    {
        colors1 = {
            { "TitleColor",
              Graphics::CustomColor(Graphics::ColorPair{ Graphics::Color::Red, Graphics::Color::Transparent }) },
            { "TextColor",
              Graphics::CustomColor(Graphics::ColorPair{ Graphics::Color::Green, Graphics::Color::Transparent }) }
        };

        constexpr Graphics::ColorPair focused  = { Graphics::Color::Yellow, Graphics::Color::Transparent };
        constexpr Graphics::ColorPair normal  = { Graphics::Color::Aqua, Graphics::Color::Transparent };
        constexpr Graphics::ColorPair hovered  = { Graphics::Color::Green, Graphics::Color::Transparent };
        constexpr Graphics::ColorPair inactive = { Graphics::Color::Gray, Graphics::Color::Transparent };
        constexpr Graphics::ColorPair pressed  = { Graphics::Color::Red, Graphics::Color::Transparent };

        auto colorObj = Graphics::ObjectColorState();
        colorObj.Set(focused, normal, inactive, hovered, pressed);

        colors2 = { { "Obj", Graphics::CustomColor(colorObj) } };

        if (!Dialogs::ThemeEditor::RegisterCustomColors("ACustomSimpleColor", colors1, this))
        {
            LOG_ERROR("Failed to register custom colors1!");
            return;
        }

        if (!Dialogs::ThemeEditor::RegisterCustomColors("ACustomObjColor", colors2, this))
        {
            LOG_ERROR("Failed to register custom colors!");
            return;
        }
    }
    ~WindowWithColors() override
    {
        Dialogs::ThemeEditor::RemovePreviewDrawListener(this);
        Dialogs::ThemeEditor::RemoveOnThemeChangeCallback(this);
    }

    void OnThemeChanged(const Application::Config& config) override
    {
        auto& simpleColorCategory = config.CustomColors.at("ACustomSimpleColor");
        colors1                   = simpleColorCategory.data;

        auto& customObjectColor = config.CustomColors.at("ACustomObjColor");
        colors2                 = customObjectColor.data;
    }

    void OnPreviewWindowDraw(
          std::string_view categoryName,
          Graphics::Renderer& r,
          int startingX,
          int startingY,
          Graphics::Size sz,
          const Application::Config::CustomColorNameStorage& colors) override
    {
        if (categoryName == "ACustomSimpleColor")
        {
            auto titleColor = colors.at("TitleColor").TryGetColorPair();
            auto textColor  = colors.at("TextColor").TryGetColorPair();
            assert(titleColor && textColor);
            if (!(titleColor && textColor))
                return; // fail-safe

            r.WriteSingleLineText(startingX, startingY, "This is a title", *titleColor);
            r.WriteSingleLineText(startingX, startingY + 2, "This is a text with custom colors.", *textColor);
        }
        if (categoryName == "ACustomObjColor")
        {
            auto objColorState = colors.at("Obj").TryGetColorState();
            assert(objColorState);
            if (!objColorState)
                return; // fail-safe
            LocalString<128> buffer;
            int y = startingY;
            for (uint32 i = 0; i < Graphics::OBJECT_COLOR_STATE_COUNT; i++)
            {
                buffer.SetFormat("This is a text with %s color.", Graphics::OBJECT_COLOR_STATE_NAMES[i].data());
                ControlState state = static_cast<ControlState>(i);
                r.WriteSingleLineText(
                      startingX,
                      y,
                      buffer.GetText(), objColorState->GetColor(state));
                y += 2;
            }
        }
    }
};

int main()
{
    Log::ToFile("IniInitialization.log");
    if (!Application::Init())
        return 1;

    WindowWithColors colorsWindow = {};

    Application::GetDesktop()->Handlers()->OnStart = [](Reference<Control>) { Dialogs::ThemeEditor::Show(); };
    Application::Run();
    return 0;
}
