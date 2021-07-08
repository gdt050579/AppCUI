#include "AppCUI.h"
#include <stdarg.h>
#include <stdio.h>

using namespace AppCUI;
using namespace AppCUI::Utils;

#define CRITICAL_ERROR_STACK_BUFFER_SIZE	0x10000
#define EXIT_IF_ERROR(condition) if (!(condition)) return false;
#define CHECK_INTERNAL_CONDITION(condition,errorDescription) \
    if (!(condition)) { \
    String::Set(Text, errorDescription, CRITICAL_ERROR_STACK_BUFFER_SIZE); \
    msg.Type = Severity::InternalError; \
    break; \
}

void (*fnMessageLogCallbak)     (const Log::Message& msg)   = nullptr;
const char * _severity_type_names_[4] = {
    "[Information] ",
    "[  Warning  ] ",
    "[   Eror    ] ",
    "[InternalErr] "
};

bool _LogMessage_to_String_(const Log::Message& msg, String &str, bool multiLine, bool addNewLineTerminator)
{
    EXIT_IF_ERROR(str.Set(_severity_type_names_[(unsigned int)msg.Type]));
    EXIT_IF_ERROR(str.Add(msg.Content));
    if (msg.Type != Log::Severity::Information)
    {
        if (multiLine) 
        {
            EXIT_IF_ERROR(str.AddFormat("\n              => File        : %s", msg.FileName));
            EXIT_IF_ERROR(str.AddFormat("\n              => Function    : %s", msg.Function));
            EXIT_IF_ERROR(str.AddFormat("\n              => Line number : %d", msg.LineNumber));
            EXIT_IF_ERROR(str.AddFormat("\n              => Condition   : %s", msg.Condition));
        } else {
            EXIT_IF_ERROR(str.AddFormat(" => File:%s , Function:%s , Line:%d, Condition:%s", msg.FileName, msg.Function,msg.LineNumber,msg.Condition ));
        }
    }
    if (addNewLineTerminator)
    {
        EXIT_IF_ERROR(str.Add("\n"));
    }
    return true;
}

void Log::Report(Log::Severity severity, const char* fileName,const char *function, const char *condition, int line, const char *format, ...)
{
	va_list 		args;
	int     		len,len2;
	char			Text[CRITICAL_ERROR_STACK_BUFFER_SIZE];
    Log::Message    msg;

	if (fnMessageLogCallbak == nullptr)
		return;
    // fill up Message structure
    msg.Type = severity;
    msg.Condition = condition;
    msg.FileName = fileName;
    msg.Function = function;
    msg.LineNumber = line;
    msg.Content = Text;

    // fill up the content
    while (true)
    {
        CHECK_INTERNAL_CONDITION(format != nullptr, "Invalid 'format' parameter (expecting a non-null one) !");
        va_start(args, format);
        len = vsnprintf(nullptr, 0, format, args);
        va_end(args);
        CHECK_INTERNAL_CONDITION(len>=0, "'vsnprinf' has returned an invalid value !");
        CHECK_INTERNAL_CONDITION(len < CRITICAL_ERROR_STACK_BUFFER_SIZE - 2, "Formatting buffer size is too large (max accepted is 0x10000 bytes)");
        va_start(args, format);
        len2 = vsnprintf(Text, CRITICAL_ERROR_STACK_BUFFER_SIZE - 2, format, args);
        va_end(args);
        CHECK_INTERNAL_CONDITION(len2 >= 0, "'vsnprinf' has returned an invalid value when writing the buffer !");
        Text[len2] = 0;
        break;
    }
    fnMessageLogCallbak(msg);
}



void Log::SetLogCallback(void(*callback)(const Message &))
{
    fnMessageLogCallbak = callback;
}

#undef EXIT_IF_ERROR
#undef CHECK_INTERNAL_CONDITION