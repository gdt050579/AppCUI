#include "Internal.hpp"

using namespace AppCUI;
using namespace AppCUI::Utils;

enum class AppCUIPropertyIDs : uint32
{
    FRONTEND_PROPERTY_ID         = 1,
    SIZE_PROPERTY_ID             = 2,
    CHARACTER_SIZE_PROPERTY_ID   = 3,
    FIXED_PROPERTY_ID            = 4,
    THEME_NAME_PROPERTY_ID       = 5,
    THEME_FOLDER_PROPERTY_ID     = 6,
    CHARACTER_SET_PROPERTY_ID    = 7,
};

namespace AppCUI::Internal
{
bool ApplicationImpl::GetPropertyValue(uint32 propertyID, Utils::PropertyValue& value)
{
    switch (static_cast<AppCUIPropertyIDs>(propertyID))
    {
    case AppCUIPropertyIDs::FRONTEND_PROPERTY_ID:
        value = static_cast<uint32>(this->frontend);
        return true;
    //case AppCUIPropertyIDs::SIZE_PROPERTY_ID:
    //    //TODO:
    //    return true;
    //case AppCUIPropertyIDs::CHARACTER_SIZE_PROPERTY_ID:
    //    // TODO:
    //    return true;
    //case AppCUIPropertyIDs::FIXED_PROPERTY_ID:
    //    // TODO:
    //    return true;
    case AppCUIPropertyIDs::THEME_NAME_PROPERTY_ID:
        value = static_cast<uint32>(this->config.Theme);
        return true;
    case AppCUIPropertyIDs::CHARACTER_SET_PROPERTY_ID:
        value = static_cast<uint32>(SpecialCharsSet);
        return true;
    }
    return false;
}
bool ApplicationImpl::SetPropertyValue(uint32 propertyID, const Utils::PropertyValue& value, Utils::String& error)
{
    switch (static_cast<AppCUIPropertyIDs>(propertyID))
    {
    case AppCUIPropertyIDs::FRONTEND_PROPERTY_ID:
    {
        const uint32 frontEndValue = std::get<uint32>(value);
        if (frontEndValue > (uint32) Application::FrontendType::Tests)
        {
            error.Format("Invalid frontend type value (%d) !", frontEndValue);
            return false;
        }
        this->frontend = static_cast<Application::FrontendType>(frontEndValue);
        return true;   
    }
    // case AppCUIPropertyIDs::SIZE_PROPERTY_ID:
    //     //TODO:
    //     return true;
    // case AppCUIPropertyIDs::CHARACTER_SIZE_PROPERTY_ID:
    //     // TODO:
    //     return true;
    // case AppCUIPropertyIDs::FIXED_PROPERTY_ID:
    //     // TODO:
    //     return true;
    //case AppCUIPropertyIDs::THEME_FOLDER_PROPERTY_ID:
    //{
    //    this->config.ThemesFolder = std::get<string_view>(value);
    //    return true;
    //}
    case AppCUIPropertyIDs::CHARACTER_SET_PROPERTY_ID:
    {
        const uint32 charSetValue = std::get<uint32>(value);
        if (charSetValue > (uint32) Application::SpecialCharacterSetType::Ascii)
        {
            error.Format("Invalid SpecialCharacterSetType type value (%d) !", charSetValue);
            return false;
        }
        this->SpecialCharsSet = static_cast<Application::SpecialCharacterSetType>(charSetValue);
        return true;
    }
    }
    return false;
}

void ApplicationImpl::SetCustomPropertyValue(uint32 propertyID)
{
    
}

bool ApplicationImpl::IsPropertyValueReadOnly(uint32 propertyID)
{
    switch (static_cast<AppCUIPropertyIDs>(propertyID))
    {
    case AppCUIPropertyIDs::FRONTEND_PROPERTY_ID:
    case AppCUIPropertyIDs::THEME_NAME_PROPERTY_ID:
    case AppCUIPropertyIDs::CHARACTER_SET_PROPERTY_ID:
        return true;
    }
    return false;
}
#define BT(t) static_cast<uint32>(t)
const vector<Utils::Property> ApplicationImpl::GetPropertiesList()
{
    return {
        { BT(AppCUIPropertyIDs::FRONTEND_PROPERTY_ID),
          "Config",
          "Frontend",
          PropertyType::List,
          false,
          "Default=0,SDL=1,Terminal=2,WindowsConsole=3,Tests=4" },
        { BT(AppCUIPropertyIDs::THEME_NAME_PROPERTY_ID),
          "Config",
          "Theme",
          PropertyType::List,
          false,
          "Default=0,Dark=1,Light=2" },
        //{ BT(AppCUIPropertyIDs::THEME_FOLDER_PROPERTY_ID), "Config", "ThemeFolder", PropertyType::UTF8, true },
        { BT(AppCUIPropertyIDs::CHARACTER_SET_PROPERTY_ID),
          "Config",
          "CharacterSet",
          PropertyType::List,
          false,
          "Auto=0,Unicode=1,LinuxTerminal=2,Ascii=3" },
    };
}
}