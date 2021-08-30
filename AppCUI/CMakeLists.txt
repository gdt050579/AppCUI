include_directories(include)
if (MSVC)
    include_directories(include/Windows)
elseif (UNIX)
    include_directories(include/Unix)
endif()

# Add libAppCUI
add_library(AppCUI SHARED)

# Add SDL
include(FindSDL2)
include(FindSDL2TTF)
find_package(SDL2)
find_package(SDL2TTF)
if (SDL2_FOUND)
    target_compile_definitions(AppCUI PRIVATE HAVE_SDL)
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
    set(SDL2_LIBRARY_RELEASE "")
    set(SDL2TTF_LIBRARY "")
    set(FONT_LIB_NAME "")
endif()

if (UNIX)
    set(CURSES_NEED_WIDE "TRUE")
    set(CURSES_NEED_NCURSES "TRUE")
    find_package(Curses)
endif()

if (CURSES_FOUND)
    message("Have Curses")
    target_compile_definitions(AppCUI PRIVATE HAVE_CURSES)
    target_include_directories(AppCUI PRIVATE ${CURSES_INCLUDE_DIRS})
    target_compile_definitions(AppCUI PRIVATE NCURSES_WIDECHAR)
else()
    message("Don't have Curses")
    set(CURSES_LIBRARIES "")
endif()


if (UNIX AND NOT APPLE)
target_link_libraries(AppCUI PRIVATE ${CURSES_LIBRARIES} ${SDL2_LIBRARY_RELEASE} ${SDL2TTF_LIBRARY} ${FONT_LIB_NAME} stdc++fs)
else()
target_link_libraries(AppCUI PRIVATE ${CURSES_LIBRARIES} ${SDL2_LIBRARY_RELEASE} ${SDL2TTF_LIBRARY} ${FONT_LIB_NAME})
endif()

add_subdirectory(src)
target_compile_definitions(AppCUI PRIVATE -DBUILD_AS_DYNAMIC_LIB)

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
