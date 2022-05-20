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

option(APPCUI_HAS_SDL "Use SDL" ON)
option(APPCUI_HAS_NCURSES "Use ncurses" ON)

# Add SDL
if (APPCUI_HAS_SDL)
    include(FindSDL2)
    include(FindSDL2TTF)
    find_package(SDL2)
    find_package(SDL2TTF)

    if (SDL2_FOUND)
        target_compile_definitions(AppCUI PRIVATE APPCUI_HAS_SDL)
        message("Have SDL2")

        # Add font used by SDL 
        include(CMakeRC)
        set(FONT_NAME Courier.ttf)
        set(FONT_PATH resources/${FONT_NAME})
        target_compile_definitions(AppCUI PRIVATE FONT_PATH="${FONT_PATH}")
        cmrc_add_resource_library(font ${FONT_PATH})
        set(FONT_LIB_NAME font)
        target_include_directories(AppCUI PRIVATE ${SDL2_INCLUDE_DIR} ${SDL2TTF_INCLUDE_DIR})
        target_link_libraries(AppCUI PRIVATE ${SDL2_LIBRARY_RELEASE} ${SDL2TTF_LIBRARY})
        target_link_libraries(AppCUI PRIVATE ${FONT_LIB_NAME})
    else()
        message("Don't have SDL2")
        set(APPCUI_HAS_SDL OFF)
    endif()
endif()

if (APPCUI_HAS_NCURSES)
    if (UNIX)
        set(CURSES_NEED_WIDE "TRUE")
        set(CURSES_NEED_NCURSES "TRUE")
        find_package(Curses)
    endif()

    if (CURSES_FOUND)
        message("Have Curses")
        target_compile_definitions(AppCUI PRIVATE APPCUI_HAS_NCURSES)
        target_include_directories(AppCUI PRIVATE ${CURSES_INCLUDE_DIRS})
        target_compile_definitions(AppCUI PRIVATE NCURSES_WIDECHAR)
        target_link_libraries(AppCUI PRIVATE ${CURSES_LIBRARIES})
    else()
        message("Don't have Curses")
        set(CURSES_LIBRARIES "")
        set(APPCUI_HAS_NCURSES OFF)
    endif()
endif()

if (UNIX AND NOT (SDL2_FOUND OR CURSES_FOUND))
    message(FATAL_ERROR "On unix you must have either ncurses or SDL")
endif()


if (UNIX AND NOT APPLE AND NOT (CMAKE_SYSTEM_NAME MATCHES "FreeBSD"))
    target_link_libraries(AppCUI PRIVATE stdc++fs)
endif()

add_subdirectory(src)

macro(find_dir_deep RESULT_PATHS BASE_PATH CONTAINS)
	file(GLOB results "${BASE_PATH}/*")
	foreach(f ${results})
		if(IS_DIRECTORY "${f}")
			string(FIND ${f} "${CONTAINS}" position)
			if (NOT position MATCHES "-1")
				set(${RESULT_PATHS} ${${RESULT_PATHS}} ${f})
				break()
			endif()
			find_dir_deep(${RESULT_PATHS} ${f} "${CONTAINS}")
		endif()
	endforeach()
endmacro(find_dir_deep RESULT_PATHS BASE_PATH)

set(LODE_PNG_PATH "")
find_dir_deep(LODE_PNG_PATH "${CMAKE_CURRENT_SOURCE_DIR}" "src/3rdPartyLibs/lodepng")
message(STATUS "LODE_PNG_PATH: ${LODE_PNG_PATH}")
target_include_directories(AppCUI PRIVATE "${LODE_PNG_PATH}")
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
