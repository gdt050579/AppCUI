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
    this->second       = 0;
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
        this->year   = t.tm_year + 1900;
        this->month  = t.tm_mon;
        this->day    = t.tm_mday;
        this->hour   = t.tm_hour;
        this->minute = t.tm_min;
        this->second = t.tm_sec;
    }
    catch (...)
    {
        Reset();
        RETURNERROR(false, "Exception triggered when reading time !");
    }

    return true;
}

inline uint64 FileTimeToPOSIX(uint32 low, uint32 high, uint32* nano = nullptr)
{
    const uint64 now    = ((uint64) low + ((uint64) (high) << 32ULL)) - 116444736000000000ULL;
    const uint64 result = now / 10000ULL;

    if (nano != nullptr)
    {
        *nano = (uint32) (((now % 10000) * 100) & 0xFFFFFFFF);
    }

    return result;
};

bool DateTime::CreateFromFileTime(const uint32 entry[2])
{
    const auto timestamp = FileTimeToPOSIX(entry[0], entry[1]);
    const auto time      = (time_t) (timestamp / 1000000U);
    struct tm t;
    try
    {
#if BUILD_FOR_WINDOWS
        localtime_s(&t, &time);
#elif BUILD_FOR_OSX
        localtime_r(&time, &t);
#elif BUILD_FOR_UNIX
        localtime_r(&time, &t);
#endif
        this->year   = t.tm_year + 1900;
        this->month  = t.tm_mon;
        this->day    = t.tm_mday;
        this->hour   = t.tm_hour;
        this->minute = t.tm_min;
        this->second = t.tm_sec;
    }
    catch (...)
    {
        Reset();
        RETURNERROR(false, "Exception triggered when reading time !");
    }

    return true;
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