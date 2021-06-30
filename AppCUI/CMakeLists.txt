include_directories(include)
if (MSVC)
include_directories(include/Windows)
else()
include_directories(include/Unix)
endif()


add_library(AppCUI SHARED)
add_subdirectory(src)
target_compile_definitions(AppCUI PRIVATE -DBUILD_AS_DYNAMIC_LIB)

# Preserve source file paths in debug mode for shared lib
if (DEBUG_BUILD)
    set_target_properties(AppCUI PROPERTIES DEBUG_POSTFIX "d")
endif()

if (UNIX)
    set(CURSES_NEED_WIDE "TRUE")
    set(CURSES_NEED_NCURSES "TRUE")
    find_package(Curses)
    if (CURSES_FOUND)
    target_include_directories(AppCUI PRIVATE ${CURSES_INCLUDE_DIRS})
    target_link_libraries(AppCUI ${CURSES_LIBRARIES})
    else()
        add_compile_options(NO_CURSES)
    endif()
endif()

# sln configuration for MSVC
# headers have to be inserted manually as target_sources 
# for VS to see them
file(GLOB_RECURSE APP_CUI_HEADERS include/*.h)
target_sources(AppCUI PRIVATE ${APP_CUI_HEADERS})
get_target_property(APP_CUI_SOURCES AppCUI SOURCES)
source_group(TREE ${CMAKE_SOURCE_DIR}/AppCUI FILES ${APP_CUI_SOURCES} ${APP_CUI_HEADERS})
