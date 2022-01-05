#include "AppCUI.hpp"
#include <stdio.h>
#include <cinttypes>

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

int main(void)
{
    const char fileName[] = "test.txt";
    AppCUI::OS::File app  = AppCUI::OS::File();
    app.Create(fileName, true);
    app.OpenWrite(fileName);
    uint32 bytesWritten = 0;
    app.Write("GDT1", 4, bytesWritten);
    printf("Size: %" PRIu64 "\n", app.GetSize());
    printf("Get poz: %" PRIu64 "\n", app.GetCurrentPos());
    app.SetCurrentPos(app.GetSize() / 2);
    printf("New poz: %" PRIu64 "\n", app.GetCurrentPos());
    app.SetSize(app.GetSize() / 2);
    printf("New size: %" PRIu64 "\n", app.GetSize());
    printf("Get poz: %" PRIu64 "\n", app.GetCurrentPos());
    app.Close();
    return 0;
}
