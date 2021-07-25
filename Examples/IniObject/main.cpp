#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Utils;

const char* ini_string = R"INI(
#fist section
[Values]
Boolean.value = true
Number = 100
Float = -15.3

[Strings]
Path = C:\Program Files\My Objects\test.txt
Path2 = C:\Program Files    ; path to the program files
SimpleString = "This is a simple string"
MultiLineString = """
    This is a multi line string
    Example
"""

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
    LOG_INFO("String value for 'Path' is [%s]", ini.GetSection("strinGs").GetValue("path"));
    LOG_INFO("String value for 'SimpleString' is [%s]", ini.GetSection("strinGs").GetValue("SimpleString"));
    LOG_INFO("String value for 'Boolean.value' is [%s]", ini.GetSection("vaLUeS").GetValue("boolean.value"));

    bool res;
    if (ini.GetSection("values").CopyBoolValue("Boolean.value", res))
    {
        LOG_INFO("The value of 'Boolean.value' is %d", res);
    }
    return 0;
}
