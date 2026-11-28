// Theme registry: the built-in themes (names <-> ThemeType), the themes found in the themes folder and in the settings
// file, switching between them, and keeping a theme after a restart.

#include <algorithm>
#include <cctype>

#include "Internal.hpp"

#ifdef MessageBox
#    undef MessageBox // <windows.h> (through Internal.hpp) maps it to MessageBoxA / MessageBoxW
#endif

namespace AppCUI
{
using namespace Application;
using namespace Utils;

namespace
{
    struct BuiltInTheme
    {
        ThemeType Type;
        string_view Name;
        string_view DisplayName;
    };
    // ThemeType order: GetThemeName indexes this table with the value of a ThemeType
    constexpr BuiltInTheme BUILT_IN_THEMES[] = {
        { ThemeType::Default, "Default", "Default" },
        { ThemeType::Dark, "Dark", "Dark" },
        { ThemeType::Light, "Light", "Light" },
        { ThemeType::HighContrast, "HighContrast", "High Contrast" },
        { ThemeType::Midnight, "Midnight", "Midnight" },
        { ThemeType::Ocean, "Ocean", "Ocean" },
        { ThemeType::Forest, "Forest", "Forest" },
        { ThemeType::RetroGreen, "RetroGreen", "Retro Green" },
    };
    constexpr uint32 BUILT_IN_THEMES_COUNT = sizeof(BUILT_IN_THEMES) / sizeof(BUILT_IN_THEMES[0]);
    static_assert(BUILT_IN_THEMES_COUNT == static_cast<uint32>(ThemeType::RetroGreen) + 1, "Update BUILT_IN_THEMES");

    inline char LowerCase(char ch)
    {
        return static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    bool EqualsNoCase(string_view a, string_view b)
    {
        if (a.size() != b.size())
            return false;
        for (size_t i = 0; i < a.size(); i++)
            if (LowerCase(a[i]) != LowerCase(b[i]))
                return false;
        return true;
    }
    bool StartsWithNoCase(string_view text, string_view prefix)
    {
        return (text.size() >= prefix.size()) && EqualsNoCase(text.substr(0, prefix.size()), prefix);
    }
    bool LessNoCase(const std::string& a, const std::string& b)
    {
        return std::lexicographical_compare(
              a.begin(), a.end(), b.begin(), b.end(), [](char x, char y) { return LowerCase(x) < LowerCase(y); });
    }
    // "High Contrast", "high-contrast", "HIGH_CONTRAST" -> equal to the canonical name "HighContrast"
    bool EqualsIgnoringSeparators(string_view name, string_view canonical)
    {
        size_t pos = 0;
        for (char ch : name)
        {
            if ((ch == ' ') || (ch == '-') || (ch == '_') || (ch == '\t'))
                continue;
            if ((pos >= canonical.size()) || (LowerCase(ch) != LowerCase(canonical[pos])))
                return false;
            pos++;
        }
        return pos == canonical.size();
    }
    // a value written in the .theme (ini) syntax: quoted when it contains spaces, comment characters or quotes
    void AppendIniValue(std::string& output, string_view value)
    {
        if (value.empty())
        {
            output += "\"\"";
            return;
        }
        const bool needsQuotes = value.find_first_of(" \t#;\"'") != string_view::npos;
        if (!needsQuotes)
        {
            output += value;
            return;
        }
        string_view quote = "\"";
        if (value.find('"') != string_view::npos)
            quote = (value.find('\'') == string_view::npos) ? string_view("'") : string_view("\"\"\"");
        output += quote;
        output += value;
        output += quote;
    }
    void AppendIniArray(std::string& output, const IniValue& value)
    {
        output += '[';
        for (uint32 index = 0; index < value.GetArrayCount(); index++)
        {
            if (index > 0)
                output += ',';
            AppendIniValue(output, value[index].ToStringView());
        }
        output += ']';
    }
    std::string SettingsSectionName(string_view themeName)
    {
        std::string result(Internal::Themes::SETTINGS_SECTION_PREFIX);
        result += themeName;
        return result;
    }
    std::filesystem::path ThemesFolderOf(const Application::Config& config)
    {
        if (!config.ThemesFolder.empty())
            return config.ThemesFolder;
        auto appPath = OS::GetCurrentApplicationPath();
        return appPath.empty() ? appPath : appPath.parent_path();
    }
    std::filesystem::path ThemeFilePath(const Application::Config& config, string_view name)
    {
        auto folder = ThemesFolderOf(config);
        if (folder.empty())
            return folder;
        std::string fileName(name);
        if (!(fileName.size() > 6 && EqualsNoCase(string_view(fileName).substr(fileName.size() - 6), ".theme")))
            fileName += ".theme";
        return folder / std::filesystem::path(std::u8string(fileName.begin(), fileName.end()));
    }
    bool IsBlank(string_view name)
    {
        return name.find_first_not_of(" \t") == string_view::npos;
    }
#ifdef __EMSCRIPTEN__
    // browser frontend: files written to the themes folder are lost on reload, the settings file is not (the host page
    // keeps it) -> edited colors go into an [AppCUI.Theme.Custom] section of the settings
    constexpr bool CUSTOM_THEME_IN_SETTINGS = true;
#else
    constexpr bool CUSTOM_THEME_IN_SETTINGS = false;
#endif
    // Writes [AppCUI] Theme=<value> (and stores / removes the [AppCUI.Theme.Custom] section) both in the settings file
    // and in the in-memory settings. Only these keys change on disk: the file is read again instead of saving the
    // in-memory settings, which may hold unsaved changes of other modules.
    bool WriteThemeSettings(
          Internal::ApplicationImpl& app, string_view value, Application::Config* storeCustom, bool removeCustom)
    {
        const auto path = Application::GetAppSettingsFile();
        CHECK(!path.empty(), false, "The path of the settings file is unknown");
        Utils::IniObject file;
        std::error_code ec;
        if (std::filesystem::exists(path, ec))
        {
            CHECK(file.CreateFromFile(path),
                  false,
                  "Settings file %s can not be parsed (it is not overwritten)",
                  path.string().c_str());
        }
        else
        {
            CHECK(file.Create(), false, "Fail to create the settings");
        }
        const auto customSection = SettingsSectionName(Internal::Themes::CUSTOM_THEME_NAME);
        for (Utils::IniObject* ini : { &file, &app.settings })
        {
            auto section = (*ini)["AppCUI"];
            CHECK(section.Exists(), false, "Fail to create the [AppCUI] section of the settings");
            section.UpdateValue("Theme", value, false);
            if (storeCustom)
            {
                CHECK(Internal::Themes::SaveToSettingsSection(*storeCustom, *ini, Internal::Themes::CUSTOM_THEME_NAME),
                      false,
                      "Fail to store the theme in the settings");
            }
            else if (removeCustom && Internal::Themes::HasSettingsSection(*ini, Internal::Themes::CUSTOM_THEME_NAME))
            {
                ini->DeleteSection(customSection);
            }
        }
        CHECK(file.Save(path), false, "Fail to save the settings file %s", path.string().c_str());
        Internal::OnSettingsFileSaved(path);
        return true;
    }
    // saves the edited colors as the "Custom" theme; false if the user declined to overwrite or the save failed
    bool SaveCustomTheme(Application::Config& colors, Internal::ApplicationImpl& app)
    {
        constexpr string_view name = Internal::Themes::CUSTOM_THEME_NAME;
#ifdef __EMSCRIPTEN__
        // the colors are stored by WriteThemeSettings; only ask before replacing a saved Custom theme
        (void) colors;
        if (Internal::Themes::HasSettingsSection(app.settings, name))
        {
            if (Dialogs::MessageBox::ShowYesNo("Theme", "A 'Custom' theme is already saved. Overwrite it ?") !=
                Dialogs::Result::Yes)
                return false;
        }
        return true;
#else
        (void) app;
        auto folder = ThemesFolderOf(colors);
        std::error_code ec;
        if (folder.empty() || (!std::filesystem::is_directory(folder, ec) &&
                               !std::filesystem::create_directories(folder, ec)))
        {
            Dialogs::MessageBox::ShowError("Theme", "Fail to create the themes folder !");
            return false;
        }
        auto file = ThemeFilePath(colors, name);
        if (std::filesystem::exists(file, ec))
        {
            if (Dialogs::MessageBox::ShowYesNo("Theme", "Custom.theme already exists in the themes folder. Overwrite it ?") !=
                Dialogs::Result::Yes)
                return false;
        }
        if (!Internal::Config::Save(colors, file))
        {
            Dialogs::MessageBox::ShowError("Theme", "Fail to save Custom.theme !");
            return false;
        }
        return true;
#endif
    }
} // namespace

// ======================================================================================= registry (public) ============
std::string_view Application::GetThemeName(ThemeType themeType)
{
    const auto index = static_cast<uint32>(themeType);
    return index < BUILT_IN_THEMES_COUNT ? BUILT_IN_THEMES[index].Name : string_view();
}
std::string_view Application::GetThemeDisplayName(ThemeType themeType)
{
    const auto index = static_cast<uint32>(themeType);
    return index < BUILT_IN_THEMES_COUNT ? BUILT_IN_THEMES[index].DisplayName : string_view();
}
std::optional<ThemeType> Application::GetThemeType(std::string_view themeName)
{
    for (const auto& theme : BUILT_IN_THEMES)
        if (EqualsIgnoringSeparators(themeName, theme.Name))
            return theme.Type;
    return std::nullopt;
}
bool Application::IsBuiltInTheme(std::string_view themeName)
{
    return GetThemeType(themeName).has_value();
}
std::vector<ThemeInfo> Application::EnumerateThemes()
{
    std::vector<ThemeInfo> result;
    result.reserve(BUILT_IN_THEMES_COUNT + 8);
    for (const auto& theme : BUILT_IN_THEMES)
        result.push_back(
              { std::string(theme.Name), std::string(theme.DisplayName), ThemeSource::BuiltIn, theme.Type });

    auto app = Application::GetApplication();
    if (!app)
        return result;
    auto isListed = [&result](string_view name) {
        if (IsBlank(name) || IsBuiltInTheme(name))
            return true; // a built-in name always means the built-in theme
        for (const auto& t : result)
            if (EqualsNoCase(t.Name, name))
                return true;
        return false;
    };

    // [AppCUI.Theme.<Name>] sections of the settings file first: Apply prefers them over a file of the same name
    std::vector<std::string> names;
    for (auto& section : app->settings.GetSections())
    {
        const auto sectionName = section.GetName();
        if (StartsWithNoCase(sectionName, Internal::Themes::SETTINGS_SECTION_PREFIX))
            names.emplace_back(sectionName.substr(Internal::Themes::SETTINGS_SECTION_PREFIX.size()));
    }
    std::sort(names.begin(), names.end(), LessNoCase);
    for (auto& name : names)
        if (!isListed(name))
            result.push_back({ name, name, ThemeSource::Settings, ThemeType::Default });

    // <Name>.theme files of the themes folder (sorted by name; a built-in name customizes the built-in theme)
    names.clear();
    std::error_code ec;
    const auto folder = ThemesFolderOf(app->config);
    if (!folder.empty() && std::filesystem::is_directory(folder, ec))
    {
        for (std::filesystem::directory_iterator it(folder, ec), end; !ec && it != end; it.increment(ec))
        {
            std::error_code fileError;
            if (!it->is_regular_file(fileError))
                continue;
            const auto& path = it->path();
            const auto ext   = path.extension().u8string();
            if (!EqualsNoCase(string_view(reinterpret_cast<const char*>(ext.data()), ext.size()), ".theme"))
                continue;
            const auto stem = path.stem().u8string();
            names.emplace_back(reinterpret_cast<const char*>(stem.data()), stem.size());
        }
    }
    std::sort(names.begin(), names.end(), LessNoCase);
    for (auto& name : names)
        if (!isListed(name))
            result.push_back({ name, name, ThemeSource::File, ThemeType::Default });
    return result;
}

// ======================================================================================= switching themes ============
void Internal::Themes::ResetCustomColors(Application::Config& config, ApplicationImpl* app)
{
    if (!app)
        return;
    for (const auto& [categoryName, defaults] : app->customColorDefaults)
    {
        auto& category = config.CustomColors[categoryName]; // keeps the preview interface of the category
        for (const auto& [colorName, color] : defaults)
            category.data[colorName] = color;
    }
}
bool Internal::Themes::Apply(
      Application::Config& config,
      string_view name,
      Application::ThemeType base,
      ApplicationImpl* app,
      std::string& resolvedName)
{
    const auto builtIn = GetThemeType(name);
    if (builtIn.has_value())
    {
        Internal::Config::SetTheme(config, builtIn.value());
        config.Theme = builtIn.value();
        ResetCustomColors(config, app);
        resolvedName = GetThemeName(builtIn.value());
        // as before the registry: a <Name>.theme file next to a built-in theme customizes it (e.g. colors saved by an
        // older theme editor as Dark.theme)
        const auto file = ThemeFilePath(config, resolvedName);
        std::error_code ec;
        if (!file.empty() && std::filesystem::is_regular_file(file, ec))
        {
            Application::Config customized = config;
            if (Internal::Config::Load(customized, file))
            {
                config = std::move(customized);
            }
            else
            {
                LOG_WARNING("Fail to load %s (the built-in theme is used)", file.string().c_str());
            }
        }
        return true;
    }
    if (IsBlank(name))
        return false;

    // a theme from the settings or from a .theme file: applied over the base theme (a theme may be incomplete)
    Application::Config theme = config;
    Internal::Config::SetTheme(theme, base);
    theme.Theme = base;
    ResetCustomColors(theme, app);
    bool loaded = false;
    if (app && HasSettingsSection(app->settings, name))
        loaded = LoadFromSettingsSection(theme, app->settings, name);
    if (!loaded)
    {
        const auto file = ThemeFilePath(theme, name);
        std::error_code ec;
        if (!file.empty() && std::filesystem::is_regular_file(file, ec))
            loaded = Internal::Config::Load(theme, file);
        if (!loaded)
        {
            LOG_WARNING("Theme '%.*s' was not found (%s)", (int) name.size(), name.data(), file.string().c_str());
        }
    }
    if (!loaded)
        return false;
    config       = std::move(theme);
    resolvedName = name;
    return true;
}
void Internal::Themes::Activate(const Application::Config& newConfig, string_view name)
{
    auto app = Application::GetApplication();
    if (!app)
        return;
    app->config           = newConfig;
    app->currentThemeName = name;
    app->RepaintStatus |= REPAINT_STATUS_ALL;
    app->TriggerThemeChange();
}
bool Internal::Themes::SameColors(Application::Config& a, Application::Config& b)
{
    std::string textA, textB;
    if (!Internal::Config::SaveToString(a, textA) || !Internal::Config::SaveToString(b, textB))
        return false;
    return textA == textB;
}

// ======================================================================================= settings sections ===========
bool Internal::Themes::HasSettingsSection(Utils::IniObject& settings, string_view name)
{
    if (IsBlank(name))
        return false;
    // GetSections is safe on settings that were never created (no file loaded)
    for (auto& section : settings.GetSections())
    {
        const auto sectionName = section.GetName();
        if (StartsWithNoCase(sectionName, SETTINGS_SECTION_PREFIX) &&
            EqualsNoCase(sectionName.substr(SETTINGS_SECTION_PREFIX.size()), name))
            return true;
    }
    return false;
}
// [AppCUI.Theme.<name>] holds the keys of a .theme file as "<Section>.<Key> = <value>" (the custom color sections
// keep their name: "CustomColors.<Category>.<Key>"; a category name must not contain a '.')
bool Internal::Themes::SaveToSettingsSection(Application::Config& config, Utils::IniObject& settings, string_view name)
{
    CHECK(!IsBlank(name), false, "Expecting a theme name");
    std::string text;
    CHECK(Internal::Config::SaveToString(config, text), false, "Fail to serialize the theme");
    Utils::IniObject theme;
    CHECK(theme.CreateFromString(text), false, "Fail to parse the serialized theme");

    auto destination = settings.CreateSection(SettingsSectionName(name), true);
    CHECK(destination.Exists(), false, "Fail to create the settings section of theme '%.*s'", (int) name.size(), name.data());
    std::string key, value;
    for (auto& section : theme.GetSections())
    {
        for (auto& entry : section.GetValues())
        {
            key = section.GetName();
            key += '.';
            key += entry.GetName();
            if (entry.IsArray())
            {
                value.clear();
                AppendIniArray(value, entry);
                destination.UpdateValue(key, string_view(value), false);
            }
            else
            {
                destination.UpdateValue(key, entry.ToStringView(), false);
            }
        }
    }
    return true;
}
bool Internal::Themes::LoadFromSettingsSection(Application::Config& config, Utils::IniObject& settings, string_view name)
{
    constexpr string_view CUSTOM_COLORS_PREFIX = "CustomColors.";
    if (!HasSettingsSection(settings, name))
        return false;
    auto section = settings.GetSection(SettingsSectionName(name));
    CHECK(section.Exists(), false, "");

    // rebuild the .theme text: one [Section] per key prefix
    std::map<std::string, std::string> themeSections;
    for (auto& entry : section.GetValues())
    {
        const auto key = entry.GetName();
        size_t dot     = key.find('.');
        if (StartsWithNoCase(key, CUSTOM_COLORS_PREFIX))
            dot = key.find('.', CUSTOM_COLORS_PREFIX.size());
        if ((dot == string_view::npos) || (dot == 0) || (dot + 1 >= key.size()))
            continue;
        auto& lines = themeSections[std::string(key.substr(0, dot))];
        lines += key.substr(dot + 1);
        lines += " = ";
        if (entry.IsArray())
        {
            AppendIniArray(lines, entry);
        }
        else
        {
            const auto raw = entry.ToStringView();
            if (!raw.empty() && raw[0] == '[')
                lines += raw; // an array stored as text (before the settings were saved and parsed again)
            else
                AppendIniValue(lines, raw);
        }
        lines += '\n';
    }
    if (themeSections.empty())
        return false;
    std::string text;
    for (const auto& [sectionName, lines] : themeSections)
    {
        text += '[';
        text += sectionName;
        text += "]\n";
        text += lines;
    }
    return Internal::Config::LoadFromString(config, text);
}

// ======================================================================================= keep after restart ==========
void Internal::Themes::AskToKeepAfterRestart(Application::Config& applied, string_view name, bool edited)
{
    auto app = Application::GetApplication();
    if (!app || !app->usesSettingsFile)
        return; // no settings file -> the theme lasts for this session only
    if (Dialogs::MessageBox::ShowYesNo("Theme", "Keep this theme after a restart? (saved to the .ini)") !=
        Dialogs::Result::Yes)
        return;
    bool saved = false;
    if (edited || IsBlank(name))
    {
        if (!SaveCustomTheme(applied, *app))
            return;
        // in the browser the colors go into [AppCUI.Theme.Custom]; elsewhere they are in Custom.theme and a leftover
        // [AppCUI.Theme.Custom] section (e.g. settings copied from the browser) would win over the file
        saved = WriteThemeSettings(
              *app, CUSTOM_THEME_NAME, CUSTOM_THEME_IN_SETTINGS ? &applied : nullptr, !CUSTOM_THEME_IN_SETTINGS);
        if (saved)
            app->currentThemeName = CUSTOM_THEME_NAME;
    }
    else
    {
        saved = Application::SaveThemeToSettings(name);
    }
    if (!saved)
        Dialogs::MessageBox::ShowError("Theme", "Fail to save the theme in the settings file !");
}

// ======================================================================================= public API ==================
void Application::SetTheme(ThemeType themeType)
{
    auto app = Application::GetApplication();
    if (!app)
        return;
    Application::Config config = app->config;
    std::string name;
    if (GetThemeName(themeType).empty())
        themeType = ThemeType::Default;
    Internal::Themes::Apply(config, GetThemeName(themeType), ThemeType::Default, app, name);
    Internal::Themes::Activate(config, name);
}
bool Application::SetTheme(std::string_view themeName)
{
    auto app = Application::GetApplication();
    CHECK(app, false, "Application has not been initialized !");
    Application::Config config = app->config;
    std::string name;
    if (!Internal::Themes::Apply(config, themeName, ThemeType::Default, app, name))
        return false;
    Internal::Themes::Activate(config, name);
    return true;
}
std::string Application::GetCurrentThemeName()
{
    auto app = Application::GetApplication();
    return app ? app->currentThemeName : std::string();
}
bool Application::SaveThemeToSettings(std::string_view themeName)
{
    auto app = Application::GetApplication();
    CHECK(app, false, "Application has not been initialized !");
    CHECK(!IsBlank(themeName), false, "Expecting a theme name");
    const auto builtIn = GetThemeType(themeName);
    const string_view value = builtIn.has_value() ? GetThemeName(builtIn.value()) : themeName;
    return WriteThemeSettings(*app, value, nullptr, false);
}
bool Application::UsesSettingsFile()
{
    auto app = Application::GetApplication();
    return app && app->usesSettingsFile;
}
} // namespace AppCUI
