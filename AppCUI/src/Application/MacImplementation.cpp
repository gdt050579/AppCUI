#include "Internal.h"
#include <string.h>

using namespace AppCUI::Internal;
bool ConsoleRenderer::Init() { return true; }
void ConsoleRenderer::FlushToScreen() {}
bool InputReader::Init() { return true; }
void InputReader::GetSystemEvent(AppCUI::Internal::SystemEvents::Event & evnt) {}
