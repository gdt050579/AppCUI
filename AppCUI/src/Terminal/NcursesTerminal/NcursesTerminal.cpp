#include "NcursesTerminal.hpp"
#include "ncurses.h"
#include <poll.h>

#if __APPLE__
    #include <mach-o/dyld.h>
#elif __linux__
    #include <linux/limits.h>
#endif

#if __linux__
static std::string get_self_path()
{
  char self[PATH_MAX] = { 0 };
  int nchar = readlink("/proc/self/exe", self, sizeof self);
  if (nchar < 0 || nchar >= PATH_MAX)   
  {
    return self;
  }

  return self;
}
#elif HAVE_WINDOWS_H
static std::wstring get_self_path()
{
  wchar_t self[MAX_PATH] = { 0 };
  SetLastError(0);
  DWORD nchar = GetModuleFileNameW(NULL, self, MAX_PATH);

  if (nchar == 0 || (nchar == MAX_PATH && ((GetLastError() == ERROR_INSUFFICIENT_BUFFER) || (self[MAX_PATH - 1] != 0))))
  {
    return self;
  }

  return self;
}
#elif __APPLE__
static std::string get_self_path()
{
  char self[PATH_MAX] = { 0 };
  uint32_t size = sizeof self;

  if (_NSGetExecutablePath(self, &size) != 0)
  {
    return self;
  }

  return self;
}
#elif HAVE_GETEXECNAME
static std::string get_self_path()
{
  std::string execname = getexecname();
  if (car(execname) == chr('/'))
  {
    return execname;
  }

  return scat3(getcwd_wrap(), chr('/'), execname);
}
#else
static val get_self_path(void)
{
  char self[PATH_MAX];
  if (argv[0] && realpath(argv[0], self))
  {
    return self;
  }

  return lit(HARD_INSTALLATION_PATH);
}
#endif

namespace AppCUI::Internal
{
bool NcursesTerminal::OnInit(const Application::InitializationData&)
{
    bool setTerminInfo = false;
    if(const char* terminfo = std::getenv("TERMINFO"))
    {
        std::cout << "Your TERMINFO is: " << terminfo << '\n';

        struct stat myStat;
        if ((stat(terminfo, &myStat) == 0) && (((myStat.st_mode) & S_IFMT) == S_IFDIR)) 
        {
            std::cout << "TERMINFO folder exists!\n";
        }
        else
        {
            std::cout << "TERMINFO folder does not exists!\n";
            setTerminInfo = true;
        }
    }

    if (setTerminInfo)
    {
        auto path = get_self_path();
        path.substr(0, path.find_last_of('/'));
        path += "/terminfo";
        std::cout << "Your terminfo path is: " << path << '\n';
        CHECK(setenv("TERMINFO", path.c_str(), true), false, "");
    }
    else
    {
        std::cout << "Your terminfo path is all set!\n";
    }

    if (!InitScreen())
        return false;
    if (!InitInput())
        return false;
    return true;
}

void NcursesTerminal::OnUnInit()
{
    UnInitInput();
    UnInitScreen();
}
}