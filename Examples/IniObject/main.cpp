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
    return 0;
}
