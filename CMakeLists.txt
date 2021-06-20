cmake_minimum_required(VERSION 3.13)

# Project setup
project(appcui VERSION 1.0)

if (CMAKE_WIN)
add_definitions(-DBUILD_FOR_WINDOWS)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(AppCUI/include)

# libAppCUI
add_library(AppCUI)
add_subdirectory(AppCUI/src)


# tester
# get_target_property(APP_CUI_SOURCES AppCUI SOURCES)
# add_executable(tester ${APP_CUI_SOURCES} Tester/Tester.cpp)