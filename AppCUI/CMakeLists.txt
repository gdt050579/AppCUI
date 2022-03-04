include_directories(include)
include_directories(src)

# Add libAppCUI
add_library(AppCUI SHARED)

#versioning
file(READ "include/AppCUI.hpp" appcui_content)
string(REGEX MATCH "#define APPCUI_VERSION \"([0-9]+.[0-9]+.[0-9]+)\"" _ ${appcui_content})
set(APPCUI_VERSION ${CMAKE_MATCH_1})
message("AppCUI version: ${APPCUI_VERSION}")

set_target_properties(AppCUI PROPERTIES PREFIX "lib")
set_target_properties(AppCUI PROPERTIES VERSION ${APPCUI_VERSION} SOVERSION ${APPCUI_VERSION})

if (MSVC)
    set(WINRC_FOLDER "windows.rc")
    message("Copy to ${WINRC_FOLDER}")
    configure_file("resources/windows.rc.in" ${WINRC_FOLDER} @ONLY)  
    target_sources(AppCUI PRIVATE ${WINRC_FOLDER})
endif()

option(APPCUI_HAVE_SDL "Use SDL" ON)
option(APPCUI_HAVE_NCURSES "Use ncurses" ON)

# Add SDL
set(SDL2_LIBRARY_RELEASE "")
set(SDL2TTF_LIBRARY "")
set(FONT_LIB_NAME "")
if (APPCUI_HAVE_SDL)
    include(FindSDL2)
    include(FindSDL2TTF)
    find_package(SDL2)
    find_package(SDL2TTF)
    if (SDL2_FOUND)
        target_compile_definitions(AppCUI PRIVATE APPCUI_HAVE_SDL)
        message("Have SDL2")

        # Add font used by SDL 
        include(CMakeRC)
        set(FONT_NAME Courier.ttf)
        set(FONT_PATH resources/${FONT_NAME})
        target_compile_definitions(AppCUI PRIVATE FONT_PATH="${FONT_PATH}")
        cmrc_add_resource_library(font ${FONT_PATH})
        target_include_directories(AppCUI PRIVATE ${SDL2_INCLUDE_DIR} ${SDL2TTF_INCLUDE_DIR})
        set(FONT_LIB_NAME font)
    else()
        message("Don't have SDL2")

    endif()
endif()

if (APPCUI_HAVE_NCURSES)
    if (UNIX)
        set(CURSES_NEED_WIDE "TRUE")
        set(CURSES_NEED_NCURSES "TRUE")
        find_package(Curses)
    endif()

    if (CURSES_FOUND)
        message("Have Curses")
        target_compile_definitions(AppCUI PRIVATE APPCUI_HAVE_NCURSES)
        target_include_directories(AppCUI PRIVATE ${CURSES_INCLUDE_DIRS})
        target_compile_definitions(AppCUI PRIVATE NCURSES_WIDECHAR)
    else()
        message("Don't have Curses")
        set(CURSES_LIBRARIES "")
    endif()
endif()

if (UNIX AND NOT (SDL2_FOUND OR CURSES_FOUND))
    message(FATAL_ERROR "On unix you must have either ncurses or SDL")
endif()


if (UNIX AND NOT APPLE AND NOT (CMAKE_SYSTEM_NAME MATCHES "FreeBSD"))
target_link_libraries(AppCUI PRIVATE ${CURSES_LIBRARIES} ${SDL2_LIBRARY_RELEASE} ${SDL2TTF_LIBRARY} ${FONT_LIB_NAME} stdc++fs)
else()
target_link_libraries(AppCUI PRIVATE ${CURSES_LIBRARIES} ${SDL2_LIBRARY_RELEASE} ${SDL2TTF_LIBRARY} ${FONT_LIB_NAME})
endif()

add_subdirectory(src)
target_include_directories(AppCUI PRIVATE src/3rdPartyLibs/lodepng)
target_compile_definitions(AppCUI PRIVATE -DBUILD_AS_DYNAMIC_LIB)

# lodepng defines
target_compile_definitions(AppCUI PRIVATE -DLODEPNG_NO_COMPILE_ENCODER)
target_compile_definitions(AppCUI PRIVATE -DLODEPNG_NO_COMPILE_DISK)
target_compile_definitions(AppCUI PRIVATE -DLODEPNG_NO_COMPILE_ANCILLARY_CHUNKS)

# Preserve source file paths in debug mode for shared lib
if (DEBUG_BUILD)
    set_target_properties(AppCUI PROPERTIES DEBUG_POSTFIX "d")
endif()



# sln configuration for MSVC
# headers have to be inserted manually as target_sources 
# for VS to see them
file(GLOB_RECURSE APP_CUI_HEADERS include/*.hpp)
target_sources(AppCUI PRIVATE ${APP_CUI_HEADERS})
get_target_property(APP_CUI_SOURCES AppCUI SOURCES)
source_group(TREE ${CMAKE_SOURCE_DIR}/AppCUI FILES ${APP_CUI_SOURCES} ${APP_CUI_HEADERS})
