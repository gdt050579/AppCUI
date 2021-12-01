#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;

const char* python_code = R"PYTHON(
import os;
d = {}
#this code will count how many files that starts with a specific letter exists
for fname in os.listdir("."):
    fname = fname.lower()
    if not fname[0] in d:
        d[fname[0]] = [fname]
    else:
        d[fname[0]] += [fname]

#show statistics
for letter in d:
    print("Letter: "+letter+" => "+str(len(d[letter]))
    index = 1
    for fname in d[letter]:
        print("%3d => %s"%(index,fname))
        index += 1

#other statistics
total = 0
for letter in d:
    total += len(d[letter])
print("Total files: "+str(total))
)PYTHON";

#define PYTHON_CHAR_TYPE_LETTER   1
#define PYTHON_CHAR_TYPE_NUMBER   2
#define PYTHON_CHAR_TYPE_STRING   3
#define PYTHON_CHAR_TYPE_COMMENT  4
#define PYTHON_CHAR_TYPE_OPERATOR 5
#define PYTHON_CHAR_TYPE_OTHER    6

const char* PythonKeywords[] = { "if",    "else", "while",  "str",  "len",    "for",    "in",
                                 "print", "def",  "return", "from", "import", "global", "local" };

int GetCharacterType(Graphics::Character* c)
{
    if (((c->Code >= 'A') && (c->Code <= 'Z')) || ((c->Code >= 'a') && (c->Code <= 'z')))
        return PYTHON_CHAR_TYPE_LETTER;
    if ((c->Code >= '0') && (c->Code <= '9'))
        return PYTHON_CHAR_TYPE_NUMBER;
    if (c->Code == '"')
        return PYTHON_CHAR_TYPE_STRING;
    if (c->Code == '#')
        return PYTHON_CHAR_TYPE_COMMENT;
    if ((c->Code == '=') || (c->Code == '<') || (c->Code == '>') || (c->Code == '!') || (c->Code == '+') ||
        (c->Code == '%') || (c->Code == '[') || (c->Code == ']') || (c->Code == '{') || (c->Code == '}') ||
        (c->Code == '(') || (c->Code == ')') || (c->Code == '-') || (c->Code == '/'))
        return PYTHON_CHAR_TYPE_OPERATOR;
    return PYTHON_CHAR_TYPE_OTHER;
}
bool Equal(Graphics::Character* start, unsigned int size, const char* text)
{
    unsigned int tr;
    for (tr = 0; (tr < size) && (*text); tr++, text++, start++)
    {
        if ((*text) != start->Code)
            return false;
    }
    return ((tr == size) && ((*text) == 0));
}
bool IsKeyword(Graphics::Character* start, unsigned int size)
{
    for (unsigned int tr = 0; tr < sizeof(PythonKeywords) / sizeof(const char*); tr++)
    {
        if (Equal(start, size, PythonKeywords[tr]))
            return true;
    }
    return false;
}
void PythonHighligh(Reference<Control>, Graphics::Character* chars, unsigned int charsCount)
{
    Graphics::Character* end   = chars + charsCount;
    Graphics::Character* start = nullptr;
    ColorPair col;
    while (chars < end)
    {
        int type = GetCharacterType(chars);
        switch (type)
        {
        case PYTHON_CHAR_TYPE_LETTER:
            start = chars;
            while ((chars < end) && (GetCharacterType(chars) == PYTHON_CHAR_TYPE_LETTER))
                chars++;
            if (IsKeyword(start, (unsigned int) (chars - start)))
                col = ColorPair{ Color::Yellow, Color::Transparent };
            else
                col = ColorPair{ Color::White, Color::Transparent };
            while (start < chars)
            {
                start->Color = col;
                start++;
            }
            break;
        case PYTHON_CHAR_TYPE_NUMBER:
            do
            {
                chars->Color = ColorPair{ Color::Aqua, Color::Transparent };
                chars++;
            } while ((chars < end) && (GetCharacterType(chars) == PYTHON_CHAR_TYPE_NUMBER));
            break;
        case PYTHON_CHAR_TYPE_OTHER:
            do
            {
                chars->Color = ColorPair{ Color::Gray, Color::Transparent };
                chars++;
            } while ((chars < end) && (GetCharacterType(chars) == PYTHON_CHAR_TYPE_OTHER));
            break;
        case PYTHON_CHAR_TYPE_OPERATOR:
            do
            {
                chars->Color = ColorPair{ Color::Silver, Color::Transparent };
                chars++;
            } while ((chars < end) && (GetCharacterType(chars) == PYTHON_CHAR_TYPE_OPERATOR));
            break;
        case PYTHON_CHAR_TYPE_COMMENT:
            do
            {
                chars->Color = ColorPair{ Color::Olive, Color::Transparent };
                chars++;
            } while ((chars < end) && (chars->Code != 13) && (chars->Code != 10));
            break;
        case PYTHON_CHAR_TYPE_STRING:
            do
            {
                chars->Color = ColorPair{ Color::Red, Color::Transparent };
                chars++;
            } while ((chars < end) && (GetCharacterType(chars) != PYTHON_CHAR_TYPE_STRING));
            if (chars < end)
            {
                chars->Color = ColorPair{ Color::Red, Color::Transparent };
                chars++;
            }
            break;
        default:
            chars->Color = ColorPair{ Color::Gray, Color::Red };
            chars++;
            break;
        }
    }
}

int main()
{
    if (!Application::Init())
        return 1;
    auto wnd = Factory::Window::Create("Python Editor", "d:c,w:40,h:20", WindowFlags::Sizeable);
    auto ta  = Factory::TextArea::Create(
          wnd,
          python_code,
          "d:c",
          TextAreaFlags::ShowLineNumbers | TextAreaFlags::ScrollBars | TextAreaFlags::SyntaxHighlighting |
                TextAreaFlags::ProcessTabKey);
    ta->Handlers()->OnTextColor = PythonHighligh;
    Application::AddWindow(std::move(wnd));
    Application::Run();
    return 0;
}
