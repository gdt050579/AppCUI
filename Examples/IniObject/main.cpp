#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Utils;
using namespace AppCUI::Input;

const char* ini_string = R"INI(
#first section
[Values]
Boolean.value = true
Number = +100
NegativeNumber = -578 ; a negative number
HexNumber = 0x1234
Binary = 0b1111 ; 15 decimal
Octal = 0777
Float = -15.3
ScreenSize = 120 x 30

[Strings]
Path = C:\Program Files\My Objects\test.txt
Path2 = C:\Program Files    ; path to the program files
SimpleString = "This is a simple string"
MultiLineString = """
    This is a multi line string
    Example
"""
Shortcut = Ctrl+Alt+F5
hexBuffer = hex:"00 20 FF 30"

[Arrays]
primeNumbers = [1,2,3,5,7,11,13,17,19]

)INI";

int main()
{
    Log::ToStdOut();
    IniObject ini;
    CHECK(ini.CreateFromString(ini_string), 1, "Fail to load ini string");
    LOG_INFO("Ini object created ==> Total sections: %d", ini.GetSectionsCount());
    LOG_INFO("Section 'BlaBlabla' status: %d", ini.GetSection("BlaBlaBla").Exists());
    LOG_INFO("Section 'Values' status: %d", ini.GetSection("Values").Exists());
    LOG_INFO("Section 'Strings' status: %d", ini.GetSection("Strings").Exists());
    LOG_INFO("Section 'Strings' hasi its name: %s", ini.GetSection("strINGS").GetName());
    LOG_INFO("The negative value of number is %d", ini.GetSection("Values").GetValue("NegativeNumber").ToInt32(-1));
    LOG_INFO("The integer value of number is %d", ini.GetSection("Values").GetValue("Number").ToInt32());
    LOG_INFO(
          "The unsigned integer value of number is 0x%08X",
          ini.GetSection("Values").GetValue("HexNumber").ToUInt32(0xFFFFFFFF));
    LOG_INFO("The binary value is %d", ini.GetSection("Values").GetValue("Binary").ToUInt32(0xFFFFFFFF));
    LOG_INFO("The octal value is %d", ini.GetSection("Values").GetValue("Octal").ToUInt32());
    LOG_INFO("The float value is %f", ini.GetSection("Values").GetValue("float").ToFloat());
    LOG_INFO("String value for 'Path' is [%s]", ini.GetSection("strinGs").GetValue("path").ToString());
    LOG_INFO("String value for 'SimpleString' is [%s]", ini.GetSection("strinGs").GetValue("SimpleString").ToString());
    LOG_INFO(
          "String value for 'Boolean.value' is [%s] (normal)",
          ini.GetSection("vaLUeS").GetValue("boolean.value").ToString());
    LOG_INFO(
          "String value for 'Boolean.value' is [%d] (from template)",
          ini.GetSection("vaLUeS").GetValue("boolean.value").AsBool().value());
    LOG_INFO("Key value for 'Shortcut' is [%d]", ini.GetSection("Strings").GetValue("Shortcut").AsKey().value());
    Key k = ini.GetSection("Strings").GetValue("Shortcut2").ToKey();
    LOG_INFO("Unexisting key: %d", k);
    Key k2 = ini.GetValue("Strings/Shortcut").ToKey();
    LOG_INFO("Shortcut = %d", k2);
    AppCUI::Graphics::Size sz = ini.GetValue("Values/ScreenSize").ToSize();
    LOG_INFO("Size is %dx%d", sz.Width, sz.Height);

    unsigned int value = Utils::Number::ToUInt32("12345678").value();
    LOG_INFO("Number is %u", value);
    int x              = Utils::Number::ToInt32("-1").value();
    LOG_INFO("Number is %d", x);

    LOG_INFO("List all sections:");
    for (auto s : ini.GetSections())
    {
        LOG_INFO("- Section: %s", s.GetName().data());
    }
    LOG_INFO("List all values for section: Values");
    for (auto v : ini.GetSection("Values").GetValues())
    {
        LOG_INFO("- %s = %s", v.GetName().data(), v.AsString().value());    
    }

    LOG_INFO("Hex buffer: %s", ini.GetSection("Strings").GetValue("hexBuffer").ToStringView().data());

    auto av = ini.GetSection("Arrays").GetValue("primeNumbers");
    LOG_INFO("Prime nubers: %d", av.GetArrayCount());
    for (unsigned int tr = 0; tr < av.GetArrayCount();tr++)
    {
        LOG_INFO(" - %d", av[tr].ToUInt32());
    }
    return 0;
}
