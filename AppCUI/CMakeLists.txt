include_directories(include)
include_directories(src)

project(AppCUI VERSION 1.0)

# Add libAppCUI
add_library(${PROJECT_NAME} SHARED)
set(APPCUI_HAS_NCURSES 1)

#versioning
file(READ "include/AppCUI.hpp" appcui_content)
string(REGEX MATCH "#define APPCUI_VERSION \"([0-9]+.[0-9]+.[0-9]+)\"" _ ${appcui_content})
set(APPCUI_VERSION ${CMAKE_MATCH_1})
message("${PROJECT_NAME} version: ${APPCUI_VERSION}")

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "lib")
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${APPCUI_VERSION} SOVERSION ${APPCUI_VERSION})

if (MSVC)
    set(WINRC_FOLDER "windows.rc")
    message("Copy to ${WINRC_FOLDER}")
    configure_file("resources/windows.rc.in" ${WINRC_FOLDER} @ONLY)  
    target_sources(${PROJECT_NAME} PRIVATE ${WINRC_FOLDER})
endif()

option(APPCUI_HAS_SDL "Use SDL" ON)
option(APPCUI_HAS_NCURSES "Use ncurses" 1)

# Add SDL
if (APPCUI_HAS_SDL)
    include(FindSDL2)
    include(FindSDL2TTF)
    find_package(SDL2)
    find_package(SDL2TTF)

    if (SDL2_FOUND)
        target_compile_definitions(${PROJECT_NAME} PRIVATE APPCUI_HAS_SDL)
        message("Have SDL2")

        # Add font used by SDL 
        include(CMakeRC)
        set(FONT_NAME Courier.ttf)
        set(FONT_PATH resources/${FONT_NAME})
        target_compile_definitions(${PROJECT_NAME} PRIVATE FONT_PATH="${FONT_PATH}")
        cmrc_add_resource_library(font ${FONT_PATH})
        set(FONT_LIB_NAME font)
        target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIR} ${SDL2TTF_INCLUDE_DIR})
        target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARY_RELEASE} ${SDL2TTF_LIBRARY})
        target_link_libraries(${PROJECT_NAME} PRIVATE ${FONT_LIB_NAME})
    else()
        message("Don't have SDL2")
        set(APPCUI_HAS_SDL OFF)
    endif()
endif()

if (NOT APPCUI_HAS_SDL)
    if (UNIX)

        set(REPO_LOCATION "https://github.com/mirror/ncurses.git")

        get_filename_component(BIN_LOCATION "${CMAKE_BINARY_DIR}/../bin" ABSOLUTE)
        set(INSTALL_LOCATION "${BIN_LOCATION}/ncursesw")
        message("INSTALL_LOCATION => ${INSTALL_LOCATION}")

        if (NOT EXISTS INSTALL_LOCATION) # some sort of caching => we may want to put this under a flag that can be passed to cmake
            if (NOT EXISTS BIN_LOCATION)
                message("Creating BIN_LOCATION => ${BIN_LOCATION}")
                file(MAKE_DIRECTORY ${BIN_LOCATION})
            else()
                message("BIN_LOCATION exists => ${BIN_LOCATION}")
            endif()

            target_compile_definitions(${PROJECT_NAME} PRIVATE APPCUI_HAS_NCURSES)

            set(CURSES_NEED_WIDE "TRUE")
            set(CURSES_NEED_NCURSES "TRUE")

            include(ExternalProject)
    
            ExternalProject_Add(ncursesw
                INSTALL_DIR ${INSTALL_LOCATION}
                GIT_REPOSITORY ${REPO_LOCATION}
                UPDATE_COMMAND git pull ${REPO_LOCATION}
                CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix <INSTALL_DIR> --with-normal --without-debug --without-manpages --enable-widec --without-progs --without-tack --without-tests
                BUILD_COMMAND make -C <BINARY_DIR>
                INSTALL_COMMAND make -C <BINARY_DIR> install
            )
    
            target_include_directories(${PROJECT_NAME} PRIVATE ${INSTALL_LOCATION}/ncursesw/include)
            target_compile_definitions(${PROJECT_NAME} PRIVATE NCURSES_WIDECHAR)
            target_link_libraries(${PROJECT_NAME} PRIVATE INTERFACE -L${INSTALL_LOCATION}/lib -lncursesw -fPIC ${CMAKE_DL_LIBS})
        endif()
    endif()
endif()

if (UNIX AND NOT APPLE AND NOT (CMAKE_SYSTEM_NAME MATCHES "FreeBSD"))
    target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
endif()

add_subdirectory(src)

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/3rdPartyLibs/lodepng")
target_compile_definitions(${PROJECT_NAME} PRIVATE -DBUILD_AS_DYNAMIC_LIB)

# lodepng defines
target_compile_definitions(${PROJECT_NAME} PRIVATE -DLODEPNG_NO_COMPILE_ENCODER)
target_compile_definitions(${PROJECT_NAME} PRIVATE -DLODEPNG_NO_COMPILE_DISK)
target_compile_definitions(${PROJECT_NAME} PRIVATE -DLODEPNG_NO_COMPILE_ANCILLARY_CHUNKS)

# Preserve source file paths in debug mode for shared lib
if (DEBUG_BUILD)
    set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")
endif()

# sln configuration for MSVC
# headers have to be inserted manually as target_sources 
# for VS to see them
file(GLOB_RECURSE APP_CUI_HEADERS include/*.hpp)
target_sources(${PROJECT_NAME} PRIVATE ${APP_CUI_HEADERS})
get_target_property(APP_CUI_SOURCES ${PROJECT_NAME} SOURCES)
source_group(TREE ${CMAKE_SOURCE_DIR}/${PROJECT_NAME} FILES ${APP_CUI_SOURCES} ${APP_CUI_HEADERS})
