cmake_minimum_required(VERSION 3.13)

# Project setup
project(appcui VERSION 1.0)

if (MSVC)
add_definitions(-DBUILD_FOR_WINDOWS)
else()
set(CMAKE_CXX_FLAGS_DEBUG "-g")
endif()

set(CMAKE_CXX_FLAGS "-W3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(AppCUI/include)


# libAppCUI
file(GLOB HEADER_FILES "AppCUI/include/*.h")
file(GLOB APPLICATION_FILES "AppCUI/src/Application/*.cpp")
file(GLOB CONSOLE_FILES "AppCUI/src/Console/*.cpp")
file(GLOB CONTROLS_FILES "AppCUI/src/Controls/*.cpp")
file(GLOB UTILS_FILES "AppCUI/src/Utils/*.cpp")
source_group("include" FILES ${HEADER_FILES})
source_group("src/Application" FILES ${APPLICATION_FILES})
source_group("src/Console" FILES ${CONSOLE_FILES})
source_group("src/Controls" FILES ${CONTROLS_FILES})
source_group("src/Utils" FILES ${UTILS_FILES})
add_library(AppCUI SHARED ${HEADER_FILES} ${APPLICATION_FILES} ${CONSOLE_FILES} ${CONTROLS_FILES} ${UTILS_FILES})
add_subdirectory(AppCUI/src)
target_compile_definitions(AppCUI PRIVATE -DBUILD_AS_DYNAMIC_LIB)

# tester
# get_target_property(APP_CUI_SOURCES AppCUI SOURCES)
add_executable(tester Tester/Tester.cpp)
target_link_libraries(tester AppCUI)