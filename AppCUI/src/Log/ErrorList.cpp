#include "AppCUI.hpp"
#include "Internal.hpp"
#include <iostream>
#include <stdarg.h>
#include <stdio.h>

using namespace AppCUI::Log;

#define LOCAL_STACK_TEXT_SIZE 0x1000

struct InternalErrorList
{
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    char fixBuffer[LOCAL_STACK_TEXT_SIZE];
    char* heapBuffer;
    unsigned int heapBufferSize;
    InternalErrorList() : heapBuffer(nullptr), heapBufferSize(0)
    {
    }
    ~InternalErrorList()
    {
        if (heapBuffer)
            delete[] heapBuffer;
        heapBuffer     = nullptr;
        heapBufferSize = 0;
    }
    char* GetBuffer(unsigned int len)
    {
        if (len < (LOCAL_STACK_TEXT_SIZE - 2))
            return fixBuffer;
        if (len > 0xFFFFFF)
            return nullptr;
        if (heapBuffer == nullptr)
        {
            len            = (len | 0xFF) + 1;
            heapBuffer     = new char[len];
            heapBufferSize = len;
        }
        if (len > (heapBufferSize - 2))
        {
            delete[] heapBuffer;
            len            = (len | 0xFF) + 1;
            heapBuffer     = new char[len];
            heapBufferSize = len;
        }
        return heapBuffer;
    }
};

ErrorList::ErrorList()
{
    data = nullptr;
}
ErrorList::~ErrorList()
{
    if (data)
    {
        auto el = reinterpret_cast<InternalErrorList*>(data);
        delete el;
        data = nullptr;
    }
}
void ErrorList::Clear()
{
    if (!data)
        return;
    auto el = reinterpret_cast<InternalErrorList*>(data);
    el->errors.clear();
    el->warnings.clear();
}
void ErrorList::AddError(const char* format, ...)
{
    va_list args;
    int len, len2;

    if (!data)
        data = new InternalErrorList();
    auto el = reinterpret_cast<InternalErrorList*>(data);

    if (!format)
        return;
    va_start(args, format);
    len = vsnprintf(nullptr, 0, format, args);
    va_end(args);
    if (len < 0)
        return;

    
    char* p = el->GetBuffer(len);
    if (!p)
        return;

    va_start(args, format);
    len2 = vsnprintf(p, len, format, args);
    va_end(args);

    if (len2 < 0)
        return;

    p[len2] = 0;
    el->errors.emplace_back(std::string_view(p, len2));
}
void ErrorList::AddWarning(const char* format, ...)
{
    va_list args;
    int len, len2;

    if (!data)
        data = new InternalErrorList();
    auto el = reinterpret_cast<InternalErrorList*>(data);

    if (!format)
        return;
    va_start(args, format);
    len = vsnprintf(nullptr, 0, format, args);
    va_end(args);
    if (len < 0)
        return;

    char* p = el->GetBuffer(len);
    if (!p)
        return;

    va_start(args, format);
    len2 = vsnprintf(p, len, format, args);
    va_end(args);

    if (len2 < 0)
        return;

    p[len2] = 0;
    el->warnings.emplace_back(std::string_view(p, len2));
}
bool ErrorList::Empty() const
{
    if (!data)
        return true;
    auto el = reinterpret_cast<InternalErrorList*>(data);
    return el->errors.empty() && el->warnings.empty();
}
unsigned int ErrorList::GetErrorsCount() const
{
    if (!data)
        return 0;
    auto el = reinterpret_cast<InternalErrorList*>(data);
    return (unsigned int)el->errors.size();
}
unsigned int ErrorList::GetWarningsCount() const
{
    if (!data)
        return 0;
    auto el = reinterpret_cast<InternalErrorList*>(data);
    return (unsigned int) el->warnings.size();
}
std::string_view ErrorList::GetError(unsigned int index) const
{
    if (!data)
        return std::string_view();
    auto el = reinterpret_cast<InternalErrorList*>(data);
    if (index >= el->errors.size())
        return std::string_view();
    return (std::string_view) (el->errors[index]);
}
std::string_view ErrorList::GetWarning(unsigned int index) const
{
    if (!data)
        return std::string_view();
    auto el = reinterpret_cast<InternalErrorList*>(data);
    if (index >= el->warnings.size())
        return std::string_view();
    return (std::string_view) (el->warnings[index]);
}

#undef LOCAL_STACK_TEXT_SIZE