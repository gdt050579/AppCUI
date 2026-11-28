// Theme registry, built-in themes, theme persistence and the ThemePicker dialog. Runs on the in-memory test frontend
// (no terminal needed). Exit code 0 = all passed.
//
// The executable writes (and removes) `<executable>.ini` and `Themes/UnitTestTheme.theme` next to itself.

#include "AppCUI.hpp"

#include <cctype>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iterator>
#include <string>
#include <vector>

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;

static int failures = 0;
#define CHECK_TRUE(cond)                                                                                                \
    do                                                                                                                   \
    {                                                                                                                    \
        if (!(cond))                                                                                                     \
        {                                                                                                                \
            std::printf("FAILED %s:%d: %s\n", __FILE__, __LINE__, #cond);                                                \
            failures++;                                                                                                  \
        }                                                                                                                \
    } while (0)

constexpr ThemeType ALL_BUILT_IN_THEMES[] = { ThemeType::Default,  ThemeType::Dark,   ThemeType::Light,
                                              ThemeType::HighContrast, ThemeType::Midnight, ThemeType::Ocean,
                                              ThemeType::Forest,   ThemeType::RetroGreen };
constexpr ThemeType NEW_THEMES[]          = { ThemeType::HighContrast, ThemeType::Midnight, ThemeType::Ocean,
                                              ThemeType::Forest, ThemeType::RetroGreen };

static bool operator==(ColorPair a, ColorPair b)
{
    return a.Foreground == b.Foreground && a.Background == b.Background;
}
static bool operator!=(ColorPair a, ColorPair b)
{
    return !(a == b);
}

// shuts the application down (the test frontend closes the application once the script is over)
static void Shutdown()
{
    Application::RunTestScript("");
}
static bool InitTests()
{
    return Application::InitForTests(100, 30, InitializationFlags::None, true);
}
static std::filesystem::path ThemesFolder()
{
    return GetAppSettingsFile().parent_path() / "Themes";
}

// ------------------------------------------------------------------------------------------------------- registry ----
static void RegistryNames()
{
    // name round trip (canonical and display names)
    for (auto type : ALL_BUILT_IN_THEMES)
    {
        CHECK_TRUE(!GetThemeName(type).empty());
        CHECK_TRUE(GetThemeType(GetThemeName(type)) == type);
        CHECK_TRUE(GetThemeType(GetThemeDisplayName(type)) == type);
        CHECK_TRUE(IsBuiltInTheme(GetThemeName(type)));
    }
    CHECK_TRUE(GetThemeName(ThemeType::HighContrast) == "HighContrast");
    CHECK_TRUE(GetThemeDisplayName(ThemeType::RetroGreen) == "Retro Green");
    CHECK_TRUE(GetThemeName(static_cast<ThemeType>(1000)).empty());

    // case-insensitive, separators ignored
    CHECK_TRUE(GetThemeType("dark") == ThemeType::Dark);
    CHECK_TRUE(GetThemeType("LIGHT") == ThemeType::Light);
    CHECK_TRUE(GetThemeType("high contrast") == ThemeType::HighContrast);
    CHECK_TRUE(GetThemeType("High-Contrast") == ThemeType::HighContrast);
    CHECK_TRUE(GetThemeType("retro_green") == ThemeType::RetroGreen);
    CHECK_TRUE(GetThemeType("  Ocean ") == ThemeType::Ocean);

    // unknown names are not built-in (they name a .theme file / a settings section)
    CHECK_TRUE(!GetThemeType("UnitTestTheme").has_value());
    CHECK_TRUE(!IsBuiltInTheme("Custom"));
    CHECK_TRUE(!IsBuiltInTheme(""));
    CHECK_TRUE(!IsBuiltInTheme("Darkish"));
    CHECK_TRUE(!IsBuiltInTheme("Dar"));

    // without an application only the built-in themes are listed, in ThemeType order
    auto themes = EnumerateThemes();
    CHECK_TRUE(themes.size() == std::size(ALL_BUILT_IN_THEMES));
    for (size_t index = 0; index < themes.size() && index < std::size(ALL_BUILT_IN_THEMES); index++)
    {
        CHECK_TRUE(themes[index].Type == ALL_BUILT_IN_THEMES[index]);
        CHECK_TRUE(themes[index].Source == ThemeSource::BuiltIn);
        CHECK_TRUE(themes[index].Name == GetThemeName(ALL_BUILT_IN_THEMES[index]));
    }
}

// ------------------------------------------------------------------------------------------------ built-in themes ----
// The color fields of Config are laid out from SearchBar to Window (bytes only: no padding); filling them with a value
// that is not a color shows the fields a theme does not set.
constexpr uint8 SENTINEL = 0xEE;
static uint8* ColorsBegin(Config* cfg)
{
    return reinterpret_cast<uint8*>(&cfg->SearchBar);
}
static uint8* ColorsEnd(Config* cfg)
{
    return reinterpret_cast<uint8*>(&cfg->Window) + sizeof(cfg->Window);
}
static_assert(alignof(ColorPair) == 1 && alignof(ObjectColorState) == 1, "the color fields must be bytes");

static void EveryThemeSetsEveryField()
{
    CHECK_TRUE(InitTests());
    auto cfg = GetAppConfig();
    CHECK_TRUE(cfg != nullptr);
    if (!cfg)
        return;
    for (auto type : ALL_BUILT_IN_THEMES)
    {
        std::memset(ColorsBegin(cfg), SENTINEL, ColorsEnd(cfg) - ColorsBegin(cfg));
        SetTheme(type);
        size_t unset = 0;
        for (auto p = ColorsBegin(cfg); p < ColorsEnd(cfg); p++)
            if (*p == SENTINEL)
                unset++;
        if (unset)
            std::printf("theme %s leaves %zu color bytes unset\n", GetThemeName(type).data(), unset);
        CHECK_TRUE(unset == 0);
        CHECK_TRUE(cfg->Theme == type);
        CHECK_TRUE(GetCurrentThemeName() == GetThemeName(type));
    }
    Shutdown();
}

// Silver and White look almost the same in the Campbell palette (#CCCCCC / #F2F2F2)
static Color Shade(Color c)
{
    return c == Color::White ? Color::Silver : c;
}
static bool ClearlyDifferent(ColorPair a, ColorPair b)
{
    return Shade(a.Foreground) != Shade(b.Foreground) || Shade(a.Background) != Shade(b.Background);
}

// readability of the new themes: a pair never draws text in its own background color, and the roles that are shown
// next to each other do not share a color pair
static void NewThemesAreReadable()
{
    CHECK_TRUE(InitTests());
    auto cfg = GetAppConfig();
    if (!cfg)
        return;
    for (auto type : NEW_THEMES)
    {
        SetTheme(type);
        const char* name = GetThemeName(type).data();
        auto pairs       = reinterpret_cast<const ColorPair*>(ColorsBegin(cfg));
        auto pairsEnd    = reinterpret_cast<const ColorPair*>(&cfg->Window);
        for (auto p = pairs; p < pairsEnd; p++)
        {
            const bool visible = p->Foreground != p->Background || p->Background == Color::Transparent;
            if (!visible)
                std::printf("theme %s: pair #%d has the same foreground and background\n", name, (int) (p - pairs));
            CHECK_TRUE(visible);
            // text over the window background must not disappear in it
            if (p->Background == Color::Transparent && p != &cfg->Button.ShadowColor)
                CHECK_TRUE(p->Foreground != cfg->Window.Background.Normal);
        }
        CHECK_TRUE(cfg->Border.Focused != cfg->Border.Normal);
        CHECK_TRUE(cfg->Border.Normal != cfg->Border.Inactive);
        CHECK_TRUE(cfg->Button.Text.Focused != cfg->Button.Text.Normal);
        CHECK_TRUE(cfg->Button.Text.Normal != cfg->Button.Text.Inactive);
        CHECK_TRUE(cfg->Button.Text.Hovered != cfg->Button.Text.Normal);
        CHECK_TRUE(cfg->Button.HotKey.Normal != cfg->Button.Text.Normal);
        CHECK_TRUE(cfg->Button.HotKey.Focused != cfg->Button.Text.Focused);
        CHECK_TRUE(cfg->Text.Normal != cfg->Text.Inactive);
        CHECK_TRUE(cfg->Text.HotKey != cfg->Text.Normal);
        CHECK_TRUE(cfg->Cursor.Normal != cfg->Selection.Text);
        CHECK_TRUE(cfg->Cursor.Normal != cfg->Cursor.Inactive);
        CHECK_TRUE(cfg->Cursor.OverSelection != cfg->Selection.Text);
        CHECK_TRUE(cfg->Cursor.OverSelection != cfg->Cursor.Normal);
        CHECK_TRUE(cfg->Selection.Editor != cfg->Selection.SearchMarker);
        CHECK_TRUE(cfg->Menu.Text.Normal != cfg->Menu.Text.Hovered);
        CHECK_TRUE(cfg->Menu.Text.Normal != cfg->Menu.Text.PressedOrSelected);
        CHECK_TRUE(cfg->Menu.Text.Hovered != cfg->Menu.Text.PressedOrSelected);
        CHECK_TRUE(cfg->Menu.Text.Normal != cfg->Menu.Text.Inactive);
        CHECK_TRUE(cfg->Menu.HotKey.Normal != cfg->Menu.Text.Normal);
        CHECK_TRUE(cfg->Tab.Text.Normal != cfg->Tab.Text.PressedOrSelected);
        CHECK_TRUE(cfg->ProgressStatus.Empty != cfg->ProgressStatus.Full);
        CHECK_TRUE(cfg->ScrollBar.Bar.Normal != cfg->ScrollBar.Position.Normal);
        CHECK_TRUE(cfg->Text.Warning != cfg->Text.Normal);
        CHECK_TRUE(ClearlyDifferent(cfg->Button.Text.Normal, cfg->Button.Text.PressedOrSelected));
        CHECK_TRUE(ClearlyDifferent(cfg->Button.Text.Focused, cfg->Button.Text.PressedOrSelected));
        CHECK_TRUE(ClearlyDifferent(cfg->Button.Text.Normal, cfg->Button.Text.Focused));
        CHECK_TRUE(ClearlyDifferent(cfg->Selection.Text, cfg->Cursor.Normal));
        CHECK_TRUE(ClearlyDifferent(cfg->Selection.Text, cfg->Cursor.OverSelection));
        CHECK_TRUE(ClearlyDifferent(cfg->Cursor.Normal, cfg->Cursor.OverSelection));
        CHECK_TRUE(ClearlyDifferent(cfg->Selection.Editor, cfg->Selection.SearchMarker));
        CHECK_TRUE(ClearlyDifferent(cfg->Selection.Editor, cfg->Selection.SimilarText));
    }
    // high contrast: selection and highlight have a background, focused and unfocused states differ
    SetTheme(ThemeType::HighContrast);
    CHECK_TRUE(cfg->Text.Highlighted.Background != Color::Transparent);
    CHECK_TRUE(cfg->Selection.Text.Background != Color::Transparent);
    CHECK_TRUE(cfg->Cursor.Normal.Background != cfg->Cursor.Inactive.Background);
    CHECK_TRUE(cfg->Text.Focused != cfg->Text.Normal);
    Shutdown();
}

// ----------------------------------------------------------------------------- unknown names, files and settings ----
struct ThemeChangeRecorder : public Dialogs::OnThemeChangedInterface
{
    std::vector<std::string> names; // GetCurrentThemeName() at every notification
    void OnThemeChanged(const Config&) override
    {
        names.emplace_back(GetCurrentThemeName());
    }
};

static bool EqualsIgnoringCase(std::string_view a, std::string_view b)
{
    if (a.size() != b.size())
        return false;
    for (size_t i = 0; i < a.size(); i++)
        if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i])))
            return false;
    return true;
}
static bool WriteThemeFile(std::string_view name, std::string_view content)
{
    std::error_code ec;
    std::filesystem::create_directories(ThemesFolder(), ec);
    return OS::File::WriteContent(ThemesFolder() / (std::string(name) + ".theme"), content);
}

static void UnknownNamesAndFallbacks()
{
    // a .theme file in the themes folder: applied over the Default theme
    CHECK_TRUE(InitTests());
    CHECK_TRUE(WriteThemeFile("UnitTestTheme", "[Text]\nNormal = Red,Transparent\nEmphasized3 = Olive,Transparent\n"));
    ThemeChangeRecorder recorder;
    CHECK_TRUE(Dialogs::ThemeEditor::RegisterOnThemeChangeCallback(&recorder));
    auto cfg = GetAppConfig();

    SetTheme(ThemeType::Dark);
    CHECK_TRUE(SetTheme("UnitTestTheme"));
    CHECK_TRUE(GetCurrentThemeName() == "UnitTestTheme");
    CHECK_TRUE((cfg->Text.Normal == ColorPair{ Color::Red, Color::Transparent }));
    CHECK_TRUE((cfg->Text.Emphasized3 == ColorPair{ Color::Olive, Color::Transparent }));
    CHECK_TRUE(cfg->Window.Background.Normal == Color::DarkBlue); // from the Default base, not from Dark
    CHECK_TRUE(cfg->Theme == ThemeType::Default);

    // listed after the built-in themes
    auto themes = EnumerateThemes();
    bool listed = false;
    for (size_t index = 0; index < themes.size(); index++)
        if (themes[index].Name == "UnitTestTheme")
            listed = index >= std::size(ALL_BUILT_IN_THEMES) && themes[index].Source == ThemeSource::File;
    CHECK_TRUE(listed);

    // an unknown name without a file: nothing changes
    CHECK_TRUE(!SetTheme("NoSuchUnitTestTheme"));
    CHECK_TRUE(GetCurrentThemeName() == "UnitTestTheme");
    CHECK_TRUE((cfg->Text.Normal == ColorPair{ Color::Red, Color::Transparent }));

    // a theme stored in the settings ([AppCUI.Theme.<Name>], flattened .theme keys) wins over a file of the same name
    // (and is listed as such)
    auto settings = GetAppSettings();
    auto section  = settings->CreateSection("AppCUI.Theme.UnitTestTheme", true);
    section.UpdateValue("Text.Normal", "Pink,Transparent", false);
    section.UpdateValue("Window.Normal", "Teal", false);
    CHECK_TRUE(SetTheme("unittesttheme"));
    CHECK_TRUE((cfg->Text.Normal == ColorPair{ Color::Pink, Color::Transparent }));
    CHECK_TRUE(cfg->Window.Background.Normal == Color::Teal);
    for (const auto& theme : EnumerateThemes())
        if (theme.Name == "UnitTestTheme")
            CHECK_TRUE(theme.Source == ThemeSource::Settings);
    settings->DeleteSection("AppCUI.Theme.UnitTestTheme");

    // every switch notified the listeners once (the failed one did not)
    CHECK_TRUE(recorder.names.size() == 3);
    if (recorder.names.size() == 3)
    {
        CHECK_TRUE(recorder.names[0] == "Dark");
        CHECK_TRUE(recorder.names[1] == "UnitTestTheme");
    }
    Dialogs::ThemeEditor::RemoveOnThemeChangeCallback(&recorder);
    Shutdown();

    // at start-up (InitializationData / [AppCUI] Theme=): built-in names are resolved by the registry ...
    InitializationData data;
    data.Frontend  = FrontendType::Tests;
    data.Width     = 80;
    data.Height    = 25;
    data.ThemeName = "retro green";
    CHECK_TRUE(Application::Init(data));
    CHECK_TRUE(GetCurrentThemeName() == "RetroGreen");
    CHECK_TRUE(GetAppConfig()->Theme == ThemeType::RetroGreen);
    Shutdown();
    // ... a file is loaded over the base theme ...
    InitializationData fileData;
    fileData.Frontend  = FrontendType::Tests;
    fileData.Width     = 80;
    fileData.Height    = 25;
    fileData.ThemeName = "UnitTestTheme";
    CHECK_TRUE(Application::Init(fileData));
    CHECK_TRUE(GetCurrentThemeName() == "UnitTestTheme");
    CHECK_TRUE((GetAppConfig()->Text.Normal == ColorPair{ Color::Red, Color::Transparent }));
    Shutdown();
    // ... and an unknown name keeps the base theme
    InitializationData unknownData;
    unknownData.Frontend  = FrontendType::Tests;
    unknownData.Width     = 80;
    unknownData.Height    = 25;
    unknownData.Theme     = ThemeType::Ocean;
    unknownData.ThemeName = "NoSuchUnitTestTheme";
    CHECK_TRUE(Application::Init(unknownData));
    CHECK_TRUE(GetCurrentThemeName() == "Ocean");
    CHECK_TRUE(GetAppConfig()->Window.Background.Normal == Color::DarkBlue);
    Shutdown();

    std::error_code ec;
    std::filesystem::remove(ThemesFolder() / "UnitTestTheme.theme", ec);
}

// <Name>.theme next to a built-in theme customizes it (the behaviour from before the registry)
static void BuiltInThemeFileOverlay()
{
    CHECK_TRUE(WriteThemeFile("Forest", "[Text]\nNormal = Pink,Transparent\n"));
    CHECK_TRUE(InitTests());
    auto cfg = GetAppConfig();
    CHECK_TRUE(SetTheme("forest"));
    CHECK_TRUE(GetCurrentThemeName() == "Forest");
    CHECK_TRUE(cfg->Theme == ThemeType::Forest);
    CHECK_TRUE((cfg->Text.Normal == ColorPair{ Color::Pink, Color::Transparent })); // from Forest.theme
    CHECK_TRUE((cfg->Border.Normal == ColorPair{ Color::Olive, Color::Transparent })); // from the built-in theme
    size_t listed = 0;
    for (const auto& theme : EnumerateThemes())
        if (EqualsIgnoringCase(theme.Name, "Forest"))
            listed++;
    CHECK_TRUE(listed == 1); // only as the built-in theme
    Shutdown();

    // at start-up too ([AppCUI] Theme=Forest)
    InitializationData data;
    data.Frontend  = FrontendType::Tests;
    data.Width     = 80;
    data.Height    = 25;
    data.ThemeName = "Forest";
    CHECK_TRUE(Application::Init(data));
    CHECK_TRUE((GetAppConfig()->Text.Normal == ColorPair{ Color::Pink, Color::Transparent }));
    Shutdown();

    std::error_code ec;
    std::filesystem::remove(ThemesFolder() / "Forest.theme", ec);
}

// ------------------------------------------------------------------------------------------- settings reset ----
static void ResetRemovesThemeSections()
{
    Utils::IniObject ini;
    CHECK_TRUE(ini.Create());
    ini.CreateSection("AppCUI", false).UpdateValue("Theme", "Custom", false);
    ini.CreateSection("AppCUI.Theme.Custom", false).UpdateValue("Text.Normal", "Red,Transparent", false);
    ini.CreateSection("Generic.Something", false).UpdateValue("Key", "1", false);
    UpdateAppCUISettings(ini, false); // adds missing keys only
    CHECK_TRUE(ini.HasSection("AppCUI.Theme.Custom"));
    CHECK_TRUE(ini.GetValue("AppCUI/Theme").ToStringView() == "Custom");
    UpdateAppCUISettings(ini, true); // reset: the kept themes go away too, other sections stay
    CHECK_TRUE(!ini.HasSection("AppCUI.Theme.Custom"));
    CHECK_TRUE(ini.HasSection("Generic.Something"));
    CHECK_TRUE(ini.GetValue("AppCUI/Theme").ToStringView() == "Default");
}

// ------------------------------------------------------------------------------------------------- custom colors ----
static void CustomColorsFollowThemeSwitches()
{
    CHECK_TRUE(InitTests());
    Config::CustomColorNameStorage colors;
    colors["Marker"] = CustomColor(ColorPair{ Color::Red, Color::Black });
    CHECK_TRUE(Dialogs::ThemeEditor::RegisterCustomColors("UnitTestColors", colors, nullptr));
    auto cfg     = GetAppConfig();
    auto& marker = cfg->CustomColors["UnitTestColors"].data["Marker"];
    marker       = CustomColor(ColorPair{ Color::Green, Color::Green }); // e.g. loaded from a theme / edited
    SetTheme(ThemeType::Forest);
    auto pair = GetAppConfig()->CustomColors["UnitTestColors"].data["Marker"].TryGetColorPair();
    CHECK_TRUE(pair != nullptr);
    if (pair)
        CHECK_TRUE((*pair == ColorPair{ Color::Red, Color::Black })); // back to the registered value

    // a theme that mentions only some colors of a category overlays them: the other names stay readable
    Config::CustomColorNameStorage more;
    more["First"]  = CustomColor(ColorPair{ Color::White, Color::Black });
    more["Second"] = CustomColor(ColorPair{ Color::Yellow, Color::Black });
    CHECK_TRUE(Dialogs::ThemeEditor::RegisterCustomColors("UnitTestOverlay", more, nullptr));
    CHECK_TRUE(WriteThemeFile(
          "UnitTestOverlay",
          "[CustomColors.UnitTestOverlay]\nFieldNames = [First]\nFieldTypes = [ColorPair]\nFirst = Red,Black\n"));
    CHECK_TRUE(SetTheme("UnitTestOverlay"));
    auto& overlay = GetAppConfig()->CustomColors["UnitTestOverlay"].data;
    CHECK_TRUE(overlay.size() == 2);
    auto first  = overlay.count("First") ? overlay["First"].TryGetColorPair() : nullptr;
    auto second = overlay.count("Second") ? overlay["Second"].TryGetColorPair() : nullptr;
    CHECK_TRUE(first && (*first == ColorPair{ Color::Red, Color::Black }));
    CHECK_TRUE(second && (*second == ColorPair{ Color::Yellow, Color::Black }));
    std::error_code ec;
    std::filesystem::remove(ThemesFolder() / "UnitTestOverlay.theme", ec);
    Shutdown();
}

// ------------------------------------------------------------------------------------------- SaveThemeToSettings ----
static void SaveThemeToSettingsWritesTheIni()
{
    const auto iniFile = GetAppSettingsFile();
    std::error_code ec;
    std::filesystem::remove(iniFile, ec);

    CHECK_TRUE(InitTests());
    CHECK_TRUE(!UsesSettingsFile()); // InitializationFlags::LoadSettingsFile was not given
    CHECK_TRUE(SaveThemeToSettings("high contrast"));
    CHECK_TRUE(GetAppSettings()->GetValue("AppCUI/Theme").ToStringView() == "HighContrast"); // canonical name
    {
        Utils::IniObject ini;
        CHECK_TRUE(ini.CreateFromFile(iniFile));
        CHECK_TRUE(ini.GetValue("AppCUI/Theme").ToStringView() == "HighContrast");
    }
    CHECK_TRUE(SaveThemeToSettings("MyOwnTheme")); // not built-in: written as it is
    {
        Utils::IniObject ini;
        CHECK_TRUE(ini.CreateFromFile(iniFile));
        CHECK_TRUE(ini.GetValue("AppCUI/Theme").ToStringView() == "MyOwnTheme");
    }
    CHECK_TRUE(!SaveThemeToSettings(""));

    // only the theme key is written: unsaved in-memory changes of other modules stay out of the file (but stay in
    // memory), and the other sections of the file are kept
    GetAppSettings()->CreateSection("UnitTestOther", false).UpdateValue("Unsaved", "yes", false);
    {
        Utils::IniObject ini;
        CHECK_TRUE(ini.CreateFromFile(iniFile));
        ini.CreateSection("UnitTestOnDisk", false).UpdateValue("Kept", "yes", false);
        CHECK_TRUE(ini.Save(iniFile));
    }
    CHECK_TRUE(SaveThemeToSettings("midnight"));
    CHECK_TRUE(GetAppSettings()->GetValue("AppCUI/Theme").ToStringView() == "Midnight");
    CHECK_TRUE(GetAppSettings()->GetValue("UnitTestOther/Unsaved").ToStringView() == "yes");
    {
        Utils::IniObject ini;
        CHECK_TRUE(ini.CreateFromFile(iniFile));
        CHECK_TRUE(ini.GetValue("AppCUI/Theme").ToStringView() == "Midnight");
        CHECK_TRUE(!ini.HasSection("UnitTestOther"));
        CHECK_TRUE(ini.GetValue("UnitTestOnDisk/Kept").ToStringView() == "yes");
    }
    Shutdown();

    // the next start (with the settings file) uses the saved theme
    CHECK_TRUE(Application::InitForTests(80, 25, InitializationFlags::LoadSettingsFile, true));
    CHECK_TRUE(UsesSettingsFile());
    CHECK_TRUE(GetCurrentThemeName() == "Midnight");
    CHECK_TRUE(GetAppConfig()->Theme == ThemeType::Midnight);
    Shutdown();

    std::filesystem::remove(iniFile, ec);
}

// ----------------------------------------------------------------------------------- ThemePicker (test script) ----
static bool pickerResult = false;
static std::vector<std::string> pickerNotifications;
static std::string themeAfterPicker;

class PickerHost : public Window, public Dialogs::OnThemeChangedInterface
{
  public:
    PickerHost() : Window("Host", "d:c,w:40,h:8", WindowFlags::None)
    {
        Factory::Button::Create(this, "&Themes...", "l:1,t:1,w:20", 1);
    }
    void OnThemeChanged(const Config&) override
    {
        pickerNotifications.emplace_back(GetCurrentThemeName());
    }
    bool OnEvent(Reference<Control> control, Event eventType, int ID) override
    {
        if (eventType == Event::ButtonClicked && ID == 1)
        {
            pickerResult     = Dialogs::ThemePicker::Show();
            themeAfterPicker = GetCurrentThemeName();
            return true;
        }
        return Window::OnEvent(control, eventType, ID);
    }
};

static void RunPickerScript(
      std::string_view script, InitializationFlags flags = InitializationFlags::None, void (*setup)() = nullptr)
{
    pickerResult = false;
    pickerNotifications.clear();
    themeAfterPicker.clear();
    CHECK_TRUE(Application::InitForTests(100, 30, flags, true));
    if (setup)
        setup();
    auto host = std::make_unique<PickerHost>();
    auto* listener = host.get();
    Application::AddWindow(std::move(host));
    CHECK_TRUE(Dialogs::ThemeEditor::RegisterOnThemeChangeCallback(listener));
    CHECK_TRUE(Application::RunTestScript(script));
}

static void ThemePickerPreviewAndCancel()
{
    // open the picker (Enter on the button), preview Dark then Light, cancel with Esc -> Default is restored
    RunPickerScript(R"SCRIPT(
        Key.Press(Enter)
        Key.Press(Down)
        Key.Press(Down)
        Key.Press(Escape)
    )SCRIPT");
    CHECK_TRUE(!pickerResult);
    CHECK_TRUE(themeAfterPicker == "Default");
    CHECK_TRUE(pickerNotifications.size() == 3);
    if (pickerNotifications.size() == 3)
    {
        CHECK_TRUE(pickerNotifications[0] == "Dark");
        CHECK_TRUE(pickerNotifications[1] == "Light");
        CHECK_TRUE(pickerNotifications[2] == "Default"); // the restore notifies the listeners too
    }

    // preview down to High Contrast and keep it with Enter (no settings file -> no question, session only)
    RunPickerScript(R"SCRIPT(
        Key.Press(Enter)
        Key.PressMultipleTimes(Down,3)
        Key.Press(Enter)
    )SCRIPT");
    CHECK_TRUE(pickerResult);
    CHECK_TRUE(themeAfterPicker == "HighContrast");
    CHECK_TRUE(pickerNotifications.size() == 3);
    if (!pickerNotifications.empty())
        CHECK_TRUE(pickerNotifications.back() == "HighContrast");

    // the active theme is not in the list (its file is gone): Enter on the highlighted first item applies Default
    RunPickerScript(
          R"SCRIPT(
        Key.Press(Enter)
        Key.Press(Enter)
    )SCRIPT",
          InitializationFlags::None,
          []() {
              CHECK_TRUE(WriteThemeFile("UnitTestGone", "[Text]\nNormal = Red,Transparent\n"));
              CHECK_TRUE(SetTheme("UnitTestGone"));
              std::error_code ec;
              std::filesystem::remove(ThemesFolder() / "UnitTestGone.theme", ec);
          });
    CHECK_TRUE(pickerResult);
    CHECK_TRUE(themeAfterPicker == "Default");

    // with a settings file, Apply asks whether to keep the theme; Yes writes [AppCUI] Theme=
    const auto iniFile = GetAppSettingsFile();
    std::error_code ec;
    std::filesystem::remove(iniFile, ec);
    RunPickerScript(
          R"SCRIPT(
        Key.Press(Enter)
        Key.Press(Down)
        Key.Press(Enter)
        Key.Press(Enter)
    )SCRIPT",
          InitializationFlags::LoadSettingsFile);
    CHECK_TRUE(pickerResult);
    CHECK_TRUE(themeAfterPicker == "Dark");
    {
        Utils::IniObject ini;
        CHECK_TRUE(ini.CreateFromFile(iniFile));
        CHECK_TRUE(ini.GetValue("AppCUI/Theme").ToStringView() == "Dark");
    }
    // ... and No keeps it for this session only
    std::filesystem::remove(iniFile, ec);
    RunPickerScript(
          R"SCRIPT(
        Key.Press(Enter)
        Key.Press(Down)
        Key.Press(Enter)
        Key.Press(Tab)
        Key.Press(Enter)
    )SCRIPT",
          InitializationFlags::LoadSettingsFile);
    CHECK_TRUE(pickerResult);
    CHECK_TRUE(themeAfterPicker == "Dark");
    CHECK_TRUE(!std::filesystem::exists(iniFile, ec));
    std::filesystem::remove(iniFile, ec);
}

int main()
{
    RegistryNames();
    EveryThemeSetsEveryField();
    NewThemesAreReadable();
    UnknownNamesAndFallbacks();
    BuiltInThemeFileOverlay();
    CustomColorsFollowThemeSwitches();
    ResetRemovesThemeSections();
    SaveThemeToSettingsWritesTheIni();
    ThemePickerPreviewAndCancel();
    if (failures == 0)
        std::printf("All theme tests passed\n");
    return failures == 0 ? 0 : 1;
}
