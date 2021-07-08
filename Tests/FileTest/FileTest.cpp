#include "AppCUI.h"
#include <stdio.h>

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;


int main(void) {
    const char fileName[] = "test.txt";
    AppCUI::OS::File app = AppCUI::OS::File();
    app.Create(fileName, true);
    app.OpenWrite(fileName);
    unsigned int bytesWritten = 0;
    app.WriteBuffer("GDT1", 4, bytesWritten);
    printf("Size: %llu\n", app.GetSize());
    printf("Get poz: %llu\n", app.GetCurrentPos());
    app.SetCurrentPos(app.GetSize()/2);
    printf("New poz: %llu\n", app.GetCurrentPos());
    app.SetSize(app.GetSize()/2);
    printf("New size: %llu\n", app.GetSize());
    printf("Get poz: %llu\n", app.GetCurrentPos());
    app.Close();
    return 0;
}
