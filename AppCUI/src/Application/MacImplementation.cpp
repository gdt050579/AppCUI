#include "Internal.h"
#include <string.h>

using namespace AppCUI::Internal;
bool ConsoleRenderer::Init() { return true; }
void ConsoleRenderer::Uninit() { }
void ConsoleRenderer::FlushToScreen() {}
bool ConsoleRenderer::UpdateCursor() {}
bool InputReader::Init() { return true; }
void InputReader::Uninit() { }
void InputReader::GetSystemEvent(AppCUI::Internal::SystemEvents::Event & evnt) {}
