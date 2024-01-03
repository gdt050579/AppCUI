#include "Internal.hpp"

#include <stdarg.h>

namespace AppCUI::Log
{
using namespace Utils;

constexpr uint32 CRITICAL_ERROR_STACK_BUFFER_SIZE = 0x10000;
static void (*fnMessageLogCallbak)(const Message& msg)   = nullptr;
static  OS::File* logFile                                = nullptr;
static const char* _severity_type_names_[5]              = {
    "[Information] ", "[  Warning  ] ", "[   Error   ] ", "[InternalErr] ", "[   Fatal   ] "
};

#define EXIT_IF_ERROR(condition)                                                                                       \
    if (!(condition))                                                                                                  \
        return false;
#define CHECK_INTERNAL_CONDITION(condition, errorDescription)                                                          \
    if (!(condition))                                                                                                  \
    {                                                                                                                  \
        String::Set(Text, errorDescription, CRITICAL_ERROR_STACK_BUFFER_SIZE);                                         \
        msg.Type = Severity::InternalError;                                                                            \
        break;                                                                                                         \
    }

bool _LogMessage_to_String_(const Message& msg, String& str, bool multiLine, bool addNewLineTerminator)
{
    EXIT_IF_ERROR(str.Set(_severity_type_names_[(uint32) msg.Type]));
    EXIT_IF_ERROR(str.Add(msg.Content));
    if (msg.Type != Severity::Information)
    {
        if (multiLine)
        {
            EXIT_IF_ERROR(str.AddFormat("\n              => File        : %s", msg.FileName));
            EXIT_IF_ERROR(str.AddFormat("\n              => Function    : %s", msg.Function));
            EXIT_IF_ERROR(str.AddFormat("\n              => Line number : %d", msg.LineNumber));
            EXIT_IF_ERROR(str.AddFormat("\n              => Condition   : %s", msg.Condition));
        }
        else
        {
            EXIT_IF_ERROR(str.AddFormat(
                  " => File:%s , Function:%s , Line:%d, Condition:%s",
                  msg.FileName,
                  msg.Function,
                  msg.LineNumber,
                  msg.Condition));
        }
    }
    if (addNewLineTerminator)
    {
        EXIT_IF_ERROR(str.Add("\n"));
    }
    return true;
}

void Report(
      Severity severity,
      const char* fileName,
      const char* function,
      const char* condition,
      uint32 line,
      const char* format,
      ...)
{
    va_list args;
    int len, len2;
    char Text[CRITICAL_ERROR_STACK_BUFFER_SIZE];
    Message msg;

    if (fnMessageLogCallbak == nullptr)
        return;
    // fill up Message structure
    msg.Type       = severity;
    msg.Condition  = condition;
    msg.FileName   = fileName;
    msg.Function   = function;
    msg.LineNumber = line;
    msg.Content    = Text;

    // fill up the content
    while (true)
    {
        CHECK_INTERNAL_CONDITION(format != nullptr, "Invalid 'format' parameter (expecting a non-null one) !");
        va_start(args, format);
        len = vsnprintf(nullptr, 0, format, args);
        va_end(args);
        CHECK_INTERNAL_CONDITION(len >= 0, "'vsnprinf' has returned an invalid value !");
        CHECK_INTERNAL_CONDITION(
              len < (int)(CRITICAL_ERROR_STACK_BUFFER_SIZE - 2),
              "Formatting buffer size is too large (max accepted is 0x10000 bytes)");
        va_start(args, format);
        len2 = vsnprintf(Text, CRITICAL_ERROR_STACK_BUFFER_SIZE - 2, format, args);
        va_end(args);
        CHECK_INTERNAL_CONDITION(len2 >= 0, "'vsnprinf' has returned an invalid value when writing the buffer !");
        Text[len2] = 0;
        break;
    }
    fnMessageLogCallbak(msg);
}

void SetLogCallback(void (*callback)(const Message&))
{
    fnMessageLogCallbak = callback;
}

void _write_to_file_callback_(const Message& msg)
{
    LocalString<2048> tmpString;
    if ((_LogMessage_to_String_(msg, tmpString, true, true)) && (logFile))
    {
        logFile->Write(tmpString.GetText(), tmpString.Len());
    }
}
bool ToFile(const std::filesystem::path& fileName)
{
    if (logFile == nullptr)
    {
        if ((logFile = new OS::File()) == nullptr)
            return false; // fail to allocate memory for File object
    }
    if (logFile->Create(fileName, true) == false)
    {
        delete logFile;
        logFile = nullptr;
        return false;
    }
    // all good
    fnMessageLogCallbak = _write_to_file_callback_;
    return true;
}

#ifdef OutputDebugString
void _write_to_OutDebugString_(const Message& msg)
{
    LocalString<2048> tmpString;
    if (_LogMessage_to_String_(msg, tmpString, false, true))
    {
        OutputDebugStringA(tmpString.GetText());
    }
}
#endif
bool ToOutputDebugString()
{
#ifdef OutputDebugString
    fnMessageLogCallbak = _write_to_OutDebugString_;
    return true;
#endif
    return false; // not on Windows
}

void _write_to_stderr_callback_(const Message& msg)
{
    LocalString<2048> tmpString;
    if (_LogMessage_to_String_(msg, tmpString, true, true))
    {
        std::cerr << tmpString.GetText();
    }
}
bool ToStdErr()
{
    fnMessageLogCallbak = _write_to_stderr_callback_;
    return true;
}

void _write_to_stdout_callback_(const Message& msg)
{
    LocalString<2048> tmpString;
    if (_LogMessage_to_String_(msg, tmpString, true, true))
    {
        std::cout << tmpString.GetText();
    }
}
bool ToStdOut()
{
    fnMessageLogCallbak = _write_to_stdout_callback_;
    return true;
}

// only available for internal usage
void Unit()
{
    if (logFile != nullptr)
    {
        logFile->Close();
        delete logFile;
        logFile = nullptr;
    }
}
} // namespace AppCUI::Log
#undef EXIT_IF_ERROR
#undef CHECK_INTERNAL_CONDITION