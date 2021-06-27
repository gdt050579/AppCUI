cmake_minimum_required(VERSION 3.13)

# Project setup
project(appcui VERSION 1.0)

if (CMAKE_BUILD_TYPE MATCHES DEBUG)
    set_property(DEBUG_BUILD)
endif()

if (MSVC)
    add_definitions(-DBUILD_FOR_WINDOWS)
    add_compile_options(-W3)
else()
    if (DEBUG_BUILD)
        add_compile_options(-g)
        add_compile_options(-W)
    endif()
endif()

if (NOT DEBUG_BUILD)
    add_compile_options(-O3)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(AppCUI/include)

# libAppCUI
add_library(AppCUI SHARED)
add_subdirectory(AppCUI/src)
target_compile_definitions(AppCUI PRIVATE -DBUILD_AS_DYNAMIC_LIB)

# Preserve source file paths in debug mode for shared lib
if (DEBUG_BUILD)
    set_target_properties(AppCUI PROPERTIES DEBUG_POSTFIX "d")
endif()

if (UNIX)
    find_package(Curses REQUIRED)
    target_include_directories(AppCUI PRIVATE ${CURSES_INCLUDE_DIRS})
    target_link_libraries(AppCUI ${CURSES_LIBRARIES})
endif()

# sln configuration for MSVC
# headers have to be inserted manually as target_sources 
# for VS to see them
file(GLOB_RECURSE APP_CUI_HEADERS AppCUI/include/*.h)
target_sources(AppCUI PRIVATE ${APP_CUI_HEADERS})
get_target_property(APP_CUI_SOURCES AppCUI SOURCES)
source_group(TREE ${CMAKE_SOURCE_DIR}/AppCUI FILES ${APP_CUI_SOURCES} ${APP_CUI_HEADERS})


# tester
add_executable(tester Tester/Tester.cpp)
target_link_libraries(tester AppCUI)
