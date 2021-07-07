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
    printf("size: %llu\n", app.GetSize());
    app.Close();
    return 0;
}
