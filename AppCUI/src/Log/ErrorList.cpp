#include "AppCUI.hpp"
#include "Internal.hpp"
#include <iostream>
#include <stdarg.h>
#include <stdio.h>

using namespace AppCUI::Log;

struct InternalErrorList
{
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};
bool Internal_FormatV(std::vector<std::string>& list, const char* format, va_list args)
{
    char Text[0x2000];
    int len = vsnprintf(Text, sizeof(Text) - 2, format, args);
    if ((len < 0) || (len >= (int) (sizeof(Text) - 1)))
        return false;
    Text[len] = 0;
    list.emplace_back(std::string_view(Text, len));
    return true;
}
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
bool ErrorList::AddError(const char* format, ...)
{
    if (!data)
        data = new InternalErrorList();
    auto el = reinterpret_cast<InternalErrorList*>(data);

    va_list args;
    va_start(args, format);
    bool res = Internal_FormatV(el->errors, format, args);
    va_end(args);

    return res;
}
bool ErrorList::AddWarning(const char* format, ...)
{
    if (!data)
        data = new InternalErrorList();
    auto el = reinterpret_cast<InternalErrorList*>(data);

    va_list args;
    va_start(args, format);
    bool res = Internal_FormatV(el->warnings, format, args);
    va_end(args);

    return res;
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
    return (unsigned int) el->errors.size();
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
void ErrorList::PopulateListView(AppCUI::Utils::Reference<AppCUI::Controls::ListView> listView) const
{
    if (!listView.IsValid())
        return;
    listView->DeleteAllItems();
    if (!data)
        return;
    auto el = reinterpret_cast<InternalErrorList*>(data);
    if (el->errors.size() > 0)
    {
        auto handle = listView->AddItem("Errors");
        listView->SetItemType(handle, AppCUI::Controls::ListViewItemType::Highlighted);
        for (auto& text : el->errors)
        {
            handle = listView->AddItem(text);
            listView->SetItemType(handle, AppCUI::Controls::ListViewItemType::ErrorInformation);
            listView->SetItemXOffset(handle, 2);
        }
    }
    if (el->warnings.size() > 0)
    {
        auto handle = listView->AddItem("Warnings");
        listView->SetItemType(handle, AppCUI::Controls::ListViewItemType::Highlighted);
        for (auto& text : el->warnings)
        {
            handle = listView->AddItem(text);
            listView->SetItemType(handle, AppCUI::Controls::ListViewItemType::WarningInformation);
            listView->SetItemXOffset(handle, 2);
        }
    }
}
