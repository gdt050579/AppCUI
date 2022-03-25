#include "Internal.hpp"
#include "dlfcn.h"

using namespace AppCUI::OS;

Library::Library()
{
    this->libraryHandle = nullptr;
}

bool Library::Load(const std::filesystem::path& path)
{
    CHECK(this->libraryHandle == nullptr, false, "Library already opened !");
    this->libraryHandle = dlopen(path.native().c_str(), RTLD_LAZY);
    CHECK(this->libraryHandle,
          false,
          "Fail to load library: %s [dlerror=%s]",
          path.generic_string().c_str(),
          dlerror());
    return true;
}

void* Library::GetFunction(const char* functionName) const
{
    CHECK(this->libraryHandle, nullptr, "Library was not loaded --> have you call Load(...) first ?");
    CHECK(functionName, nullptr, "Expecting a valid (non-null) function name !");
    CHECK(*functionName, nullptr, "Expecting a valid (non-empty) function name !");
    // all good
    void* fnPtr             = dlsym(libraryHandle, functionName);
    const char* dlsym_error = dlerror();
    CHECK((dlsym_error == nullptr) && (fnPtr),
          nullptr,
          "Unable to find address of function: %s [dlerror=%s]",
          functionName,	
          dlsym_error);
    return fnPtr;
}
