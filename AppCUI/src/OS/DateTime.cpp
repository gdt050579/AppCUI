#include "AppCUI.hpp"
#include <chrono>
#if defined(BUILD_FOR_OSX) || defined(BUILD_FOR_UNIX)
#    include <sys/stat.h>
#endif
namespace AppCUI::OS
{
DateTime::DateTime()
{
    Reset();
}
void DateTime::Reset()
{
    this->year         = 0;
    this->month        = 0;
    this->day          = 0;
    this->hour         = 0;
    this->minute       = 0;
    this->second      = 0;
    this->strFormat[0] = 0;
}
// Currently not all compilers support clock_cast (including gcc)
// AppleClang supports std::chrono::file_clock::to_time_t, but gcc or VS doesn't
// Have this frankenstein's monster while the compilers update
//
// Normally, we should be able to convert using clock_cast or a to_time_t kind of function
// to say we have full support
bool DateTime::CreateFrom(const std::filesystem::directory_entry& entry)
{
    time_t result;
    struct tm t;
    try
    {
#if BUILD_FOR_WINDOWS
        auto lastTime = entry.last_write_time();
        result = std::chrono::system_clock::to_time_t(std::chrono::clock_cast<std::chrono::system_clock>(lastTime));
        localtime_s(&t, &result);
#elif BUILD_FOR_OSX
        struct stat attr;
        stat(entry.path().string().c_str(), &attr);
        result = attr.st_mtimespec.tv_sec;
        localtime_r(&result, &t);
#elif BUILD_FOR_UNIX
        struct stat attr;
        stat(entry.path().string().c_str(), &attr);
        result = attr.st_mtime;
        localtime_r(&result, &t);
#endif
        this->year    = t.tm_year+1900;
        this->month   = t.tm_mon;
        this->day     = t.tm_mday;
        this->hour    = t.tm_hour;
        this->minute  = t.tm_min;
        this->second = t.tm_sec;
        return true;
    }
    catch (...)
    {
        Reset();
        RETURNERROR(false, "Exception triggered when reading time !");
    }
}
std::string_view DateTime::GetStringRepresentation()
{
    if (this->year == 0)
    {
        return "----/--/-- --:--:--";
    }
    AppCUI::Utils::String temp;
    temp.Create(this->strFormat, sizeof(this->strFormat), true);
    return temp.Format("%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);
}
} // namespace AppCUI::OS