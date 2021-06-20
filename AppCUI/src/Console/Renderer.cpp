#include "../../include/AppCUI.h"
#include "../../include/Internal.h"

using namespace AppCUI::Console;

#define CRND  ((AppCUI::Internal::ConsoleRenderer*)this->consoleRenderer)
#define ConsoleRendererCall(x) if (this->consoleRenderer) CRND->x;

Renderer::Renderer()
{
    this->consoleRenderer = nullptr;
}
void Renderer::Init(void* _consoleRenderer)
{
    if ((_consoleRenderer) && !(this->consoleRenderer))
        this->consoleRenderer = _consoleRenderer;
}
void Renderer::FillRect(int left, int top, int right, int bottom, int charCode, unsigned int color)
{
    ConsoleRendererCall(FillRect(left, top, right, bottom, charCode, color));
}
void Renderer::FillRectWidthHeight(int x, int y, int width, int height, int charCode, unsigned int color)
{
    ConsoleRendererCall(FillRect(x, y, x + width - 1, y + height - 1, charCode, color));
}
void Renderer::Clear(int charCode, unsigned int color)
{
    ConsoleRendererCall(ClearClipRectangle(charCode, color));
}
void Renderer::ClearWithSpecialChar(SpecialChars::Type charID, unsigned int color)
{
    if (this->consoleRenderer)
    {
        CRND->ClearClipRectangle(CRND->SpecialCharacters[(unsigned int)charID], color);
    }
}
void Renderer::WriteSingleLineText(int x, int y, const char * text, unsigned int color, int textSize)
{
    ConsoleRendererCall(WriteSingleLineText(x, y, text, color, textSize));
}
void Renderer::WriteMultiLineText(int x, int y, const char * text, unsigned int color, int textSize)
{
    ConsoleRendererCall(WriteMultiLineText(x, y, text, color, textSize));
}
void Renderer::FillHorizontalLine(int left, int y, int right, int charCode, unsigned int color)
{
    ConsoleRendererCall(FillHorizontalLine(left, y, right, charCode, color));
}
void Renderer::FillHorizontalLineSize(int x, int y, int size, int charCode, unsigned int color)
{
    ConsoleRendererCall(FillHorizontalLine(x, y, x+size-1, charCode, color));
}
void Renderer::FillVerticalLine(int x, int top, int bottom, int charCode, unsigned int color)
{
    ConsoleRendererCall(FillVerticalLine(x, top, bottom, charCode, color));
}
void Renderer::FillVerticalLineSize(int x, int y, int size, int charCode, unsigned int color)
{
    ConsoleRendererCall(FillVerticalLine(x, y, y+size-1, charCode, color));
}
void Renderer::DrawRect(int left, int top, int right, int bottom, unsigned int color, bool doubleLine)
{
    ConsoleRendererCall(DrawRect(left, top, right, bottom, color, doubleLine));
}
void Renderer::DrawRectWidthHeight(int x, int y, int width, int height, unsigned int color, bool doubleLine)
{
    ConsoleRendererCall(DrawRect(x, y, x+width-1, y+height-1, color, doubleLine));
}
void Renderer::WriteCharacter(int x, int y, int charCode, unsigned int color)
{
    ConsoleRendererCall(WriteCharacter(x, y, charCode, color));
}
void Renderer::WriteSpecialCharacter(int x, int y, SpecialChars::Type charID, unsigned int color)
{
    if (this->consoleRenderer)
    {
        CRND->WriteCharacter(x, y, CRND->SpecialCharacters[(unsigned int)charID], color);
    }
}
#undef CRND