#include "AppCUI.h"
#include <stdarg.h>
#include <stdio.h>

using namespace AppCUI;
using namespace AppCUI::Utils;

#define CRITICAL_ERROR_STACK_BUFFER_SIZE	0x10000
#define CHECK_INTERNAL_CONDITION(condition,errorDescription) \
    if (!(condition)) { \
    String::Set(Text, errorDescription, CRITICAL_ERROR_STACK_BUFFER_SIZE); \
    msg.Type = Severity::InternalError; \
    break; \
}

void (*fnMessageLogCallbak)     (const Log::Message& msg)   = nullptr;

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
