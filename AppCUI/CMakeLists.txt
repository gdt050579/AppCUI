project(AppCUI VERSION 1.0)

include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)

if( supported )
	message(STATUS "${PROJECT_NAME} => IPO / LTO enabled")
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
	message(STATUS "${PROJECT_NAME} => IPO / LTO not supported: <${error}>")
endif()

get_filename_component(INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/include" ABSOLUTE)
get_filename_component(SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src" ABSOLUTE)
get_filename_component(LIBS_DIR "${CMAKE_CURRENT_LIST_DIR}/libs" ABSOLUTE)
get_filename_component(MAIN_HEADER "${INCLUDE_DIR}/AppCUI.hpp" ABSOLUTE)

message("${PROJECT_NAME} INCLUDE_DIR: ${INCLUDE_DIR}")
message("${PROJECT_NAME} SRC_DIR: ${SRC_DIR}")
message("${PROJECT_NAME} LIBS_DIR: ${LIBS_DIR}")
message("${PROJECT_NAME} MAIN_HEADER: ${MAIN_HEADER}")

include_directories("${INCLUDE_DIR}")
include_directories("${SRC_DIR}")

# ---------------------------------------------------------------------------------------------------------------------
# Target layout
#
#   AppCUI::Core                 (AppCUICore, OBJECT)  - everything except the UI backends; no SDL/FreeType/ncurses
#   AppCUI::Frontend::SDL        (AppCUIFrontendSDL, OBJECT)            - SDL2 + SDL2_ttf + FreeType + embedded font
#   AppCUI::Frontend::Ncurses    (AppCUIFrontendNcurses, OBJECT)        - Unix / macOS only, ncursesw
#   AppCUI::Frontend::WindowsConsole (AppCUIFrontendWindowsConsole, OBJECT) - Windows (MSVC) only
#   AppCUI                       (SHARED) - libAppCUI = Core + every enabled frontend
#
# The components are OBJECT libraries: their object files are linked straight into libAppCUI, exactly as if all the
# sources still belonged to it (same flags, same LTO, no extra runtime cost). It also guarantees that the static
# registration object of every frontend (see src/Terminal/TerminalFactory.hpp) is kept by the linker - an archive
# (STATIC library) member that nothing references would be silently dropped.
# Turning every APPCUI_FRONTEND_* option OFF produces a core-only libAppCUI (only FrontendType::Tests and
# FrontendType::Custom are available).
# ---------------------------------------------------------------------------------------------------------------------
if (MSVC)
    set(APPCUI_FRONTEND_WINDOWS_CONSOLE_DEFAULT ON)
else()
    set(APPCUI_FRONTEND_WINDOWS_CONSOLE_DEFAULT OFF)
endif()
if (UNIX)
    set(APPCUI_FRONTEND_NCURSES_DEFAULT ON)
else()
    set(APPCUI_FRONTEND_NCURSES_DEFAULT OFF)
endif()
option(APPCUI_FRONTEND_SDL "Link the SDL2 frontend (AppCUI::Frontend::SDL) into libAppCUI" ON)
option(APPCUI_FRONTEND_NCURSES "Link the ncurses frontend (AppCUI::Frontend::Ncurses) into libAppCUI" ${APPCUI_FRONTEND_NCURSES_DEFAULT})
option(APPCUI_FRONTEND_WINDOWS_CONSOLE "Link the Windows console frontend (AppCUI::Frontend::WindowsConsole) into libAppCUI" ${APPCUI_FRONTEND_WINDOWS_CONSOLE_DEFAULT})

if (APPCUI_FRONTEND_NCURSES AND NOT UNIX)
    message(FATAL_ERROR "${PROJECT_NAME} => APPCUI_FRONTEND_NCURSES is only available on Unix / macOS")
endif()
if (APPCUI_FRONTEND_WINDOWS_CONSOLE AND NOT MSVC)
    message(FATAL_ERROR "${PROJECT_NAME} => APPCUI_FRONTEND_WINDOWS_CONSOLE is only available on Windows (MSVC)")
endif()

# every object file ends up in the shared libAppCUI -> same settings as the sources of a SHARED target
function(appcui_configure_component target)
    set_target_properties(${target} PROPERTIES POSITION_INDEPENDENT_CODE ON FOLDER "AppCUI")
    target_compile_definitions(${target} PRIVATE -DBUILD_AS_DYNAMIC_LIB)
endfunction()

# ---------------------------------------------------------------------------------------------------------------------
# AppCUI::Core
# ---------------------------------------------------------------------------------------------------------------------
add_library(AppCUICore OBJECT)
add_library(AppCUI::Core ALIAS AppCUICore)
appcui_configure_component(AppCUICore)
target_include_directories(AppCUICore PUBLIC "${INCLUDE_DIR}")

find_package(JPEG REQUIRED)
find_package(libjpeg-turbo CONFIG REQUIRED)
find_package(PNG REQUIRED)

if (UNIX AND NOT APPLE)
    target_link_libraries(AppCUICore PUBLIC ${CMAKE_DL_LIBS})
endif()

message("PNG_LIBRARIES => ${PNG_LIBRARIES}")
target_include_directories(AppCUICore PRIVATE ${PNG_INCLUDE_DIR})
target_link_libraries(AppCUICore PUBLIC ${PNG_LIBRARIES})

message("JPEG_LIBRARIES => ${JPEG_LIBRARIES}")
target_include_directories(AppCUICore PRIVATE ${JPEG_INCLUDE_DIR})
target_link_libraries(AppCUICore PUBLIC ${JPEG_LIBRARIES})
target_link_libraries(AppCUICore PUBLIC $<IF:$<TARGET_EXISTS:libjpeg-turbo::turbojpeg>,libjpeg-turbo::turbojpeg,libjpeg-turbo::turbojpeg-static>)

if (APPLE)
    # (kept as before the frontend split: the whole library is compiled with it on macOS)
    add_definitions(-D_XOPEN_SOURCE_EXTENDED)
endif()

if (UNIX AND NOT APPLE AND NOT (CMAKE_SYSTEM_NAME MATCHES "FreeBSD"))
    target_link_libraries(AppCUICore PUBLIC stdc++fs)
endif()

# ---------------------------------------------------------------------------------------------------------------------
# AppCUI::Frontend::SDL
# ---------------------------------------------------------------------------------------------------------------------
if (APPCUI_FRONTEND_SDL)
    add_library(AppCUIFrontendSDL OBJECT)
    add_library(AppCUI::Frontend::SDL ALIAS AppCUIFrontendSDL)
    appcui_configure_component(AppCUIFrontendSDL)
    target_link_libraries(AppCUIFrontendSDL PRIVATE AppCUICore)

    include(FindSDL2)

    find_package(unofficial-brotli CONFIG REQUIRED)
    find_package(BZip2 REQUIRED)
    find_package(Freetype REQUIRED)
    find_package(SDL2 CONFIG REQUIRED)
    find_package(SDL2_ttf CONFIG REQUIRED)

    if (APPLE)
        find_package(Iconv REQUIRED)
        message("Iconv_LIBRARY => ${Iconv_LIBRARY}")
        target_link_libraries(AppCUIFrontendSDL PUBLIC "${Iconv_LIBRARY}")

        target_link_libraries(AppCUIFrontendSDL PUBLIC "-framework Cocoa")
        target_link_libraries(AppCUIFrontendSDL PUBLIC "-framework IOKit")
        target_link_libraries(AppCUIFrontendSDL PUBLIC "-framework OpenGL")
        target_link_libraries(AppCUIFrontendSDL PUBLIC "-framework Metal")
        target_link_libraries(AppCUIFrontendSDL PUBLIC "-framework CoreAudio")
        target_link_libraries(AppCUIFrontendSDL PUBLIC "-framework AudioToolBox")
        target_link_libraries(AppCUIFrontendSDL PUBLIC "-framework Carbon")
        target_link_libraries(AppCUIFrontendSDL PUBLIC "-framework CoreVideo")
        target_link_libraries(AppCUIFrontendSDL PUBLIC "-framework CoreHaptics")
        target_link_libraries(AppCUIFrontendSDL PUBLIC "-framework GameController")
        target_link_libraries(AppCUIFrontendSDL PUBLIC "-framework ForceFeedback")
        target_link_libraries(AppCUIFrontendSDL PUBLIC "-framework QuartzCore")
    elseif (UNIX)
        include(FindX11)
        if (X11_FOUND)
            message("X11_LIBRARIES => ${X11_LIBRARIES}")
            target_link_libraries(AppCUIFrontendSDL PUBLIC ${X11_LIBRARIES})
        else()
            message(" *****************************************************************************")
            message(" *************** X11 DEVELOPMENT LIBRARIES NOT FOUND *************************")
            message(" *** Make sure libx11-dev is installed if you want SDL support.            ***")
            message(" *** On Ubuntu run: sudo apt-get install libx11-dev                        ***")
            message(" *** (or configure with -DAPPCUI_FRONTEND_SDL=OFF)                         ***")
            message(" *****************************************************************************")
            message( FATAL_ERROR "${PROJECT_NAME} => X11 DEVELOPMENT LIBRARIES NOT FOUND!" )
        endif()
    endif()

    message("BZIP2_LIBRARIES => ${BZIP2_LIBRARIES}")
    target_link_libraries(AppCUIFrontendSDL PUBLIC ${BZIP2_LIBRARIES})

    message("BROTLICOMMON_LIBRARIES => ${BROTLICOMMON_LIBRARIES}")
    message("BROTLIDEC_LIBRARIES => ${BROTLIDEC_LIBRARIES}")
    target_link_libraries(AppCUIFrontendSDL PUBLIC ${BROTLIDEC__LIBRARIES} ${BROTLICOMMON_LIBRARIES})

    message("FREETYPE_LIBRARIES => ${FREETYPE_LIBRARIES}")
    target_link_libraries(AppCUIFrontendSDL PUBLIC ${FREETYPE_LIBRARIES})

    # Add font used by SDL
    include(CMakeRC)

    set(FONT_NAME JuliaMono-Regular.ttf)
    set(FONT_PATH resources/${FONT_NAME})
    target_compile_definitions(AppCUIFrontendSDL PRIVATE FONT_PATH="${FONT_PATH}")
    cmrc_add_resource_library(font ${FONT_PATH})
    set(FONT_LIB_NAME font)
    message("FONT_NAME => ${FONT_NAME}")
    message("FONT_PATH => ${FONT_PATH}")
    message("FONT_LIB_NAME => ${FONT_LIB_NAME}")

    target_include_directories(AppCUIFrontendSDL PRIVATE ${SDL2_INCLUDE_DIR})
    target_link_libraries(AppCUIFrontendSDL PUBLIC ${SDL2_LIBRARIES})
    # Use the imported target so each configuration links its matching library (SDL2_ttfd in Debug, SDL2_ttf in Release)
    target_link_libraries(AppCUIFrontendSDL PUBLIC $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>)
    target_link_libraries(AppCUIFrontendSDL PUBLIC ${FONT_LIB_NAME})

    if (APPLE AND NOT APPCUI_ENABLE_OUTPUT_LOCATION)
        if (CMAKE_GENERATOR STREQUAL "Xcode")
            set_target_properties(${FONT_LIB_NAME} PROPERTIES
                ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../bin/${CMAKE_BUILD_TYPE}"
                LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../bin/${CMAKE_BUILD_TYPE}"
                RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../bin/${CMAKE_BUILD_TYPE}"
                ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../bin/${CMAKE_BUILD_TYPE}"
                LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../bin/${CMAKE_BUILD_TYPE}"
                RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../bin/${CMAKE_BUILD_TYPE}")
        endif()
    endif()
endif()

# ---------------------------------------------------------------------------------------------------------------------
# AppCUI::Frontend::Ncurses
# ---------------------------------------------------------------------------------------------------------------------
if (APPCUI_FRONTEND_NCURSES)
    add_library(AppCUIFrontendNcurses OBJECT)
    add_library(AppCUI::Frontend::Ncurses ALIAS AppCUIFrontendNcurses)
    appcui_configure_component(AppCUIFrontendNcurses)
    target_link_libraries(AppCUIFrontendNcurses PRIVATE AppCUICore)

    set(CURSES_NEED_WIDE TRUE)
    set(CURSES_NEED_NCURSES TRUE)
    find_package(Curses REQUIRED)

    message("CURSES FOUND!")
    message("CURSES_INCLUDE_DIRS => ${CURSES_INCLUDE_DIRS}")
    message("CURSES_LIBRARIES => ${CURSES_LIBRARIES}")
    target_compile_definitions(AppCUIFrontendNcurses PRIVATE NCURSES_WIDECHAR)
    target_include_directories(AppCUIFrontendNcurses PRIVATE ${CURSES_INCLUDE_DIRS})
    target_link_libraries(AppCUIFrontendNcurses PUBLIC ${CURSES_LIBRARIES})
endif()

# ---------------------------------------------------------------------------------------------------------------------
# AppCUI::Frontend::WindowsConsole
# ---------------------------------------------------------------------------------------------------------------------
if (APPCUI_FRONTEND_WINDOWS_CONSOLE)
    add_library(AppCUIFrontendWindowsConsole OBJECT)
    add_library(AppCUI::Frontend::WindowsConsole ALIAS AppCUIFrontendWindowsConsole)
    appcui_configure_component(AppCUIFrontendWindowsConsole)
    target_link_libraries(AppCUIFrontendWindowsConsole PRIVATE AppCUICore)
endif()

add_subdirectory("${SRC_DIR}")

# ---------------------------------------------------------------------------------------------------------------------
# libAppCUI (public target used by the examples, the tests and every consumer)
# ---------------------------------------------------------------------------------------------------------------------
add_library(${PROJECT_NAME} SHARED)

# versioning
file(READ "${MAIN_HEADER}" appcui_content)
string(REGEX MATCH "#define APPCUI_VERSION \"([0-9]+.[0-9]+.[0-9]+)\"" _ ${appcui_content})
set(APPCUI_VERSION ${CMAKE_MATCH_1})
message("${PROJECT_NAME} version: ${APPCUI_VERSION}")
string(REPLACE "." "," VERSION_LIST ${APPCUI_VERSION})
set(APPCUI_VERSION_LIST ${VERSION_LIST},0)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${APPCUI_VERSION} SOVERSION ${APPCUI_VERSION})

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "lib")
target_compile_definitions(${PROJECT_NAME} PRIVATE -DBUILD_AS_DYNAMIC_LIB)

if (MSVC)
    set(WINRC_FOLDER "windows.rc")
    message("Copy to ${WINRC_FOLDER}")
    configure_file("resources/windows.rc.in" ${WINRC_FOLDER} @ONLY)
    target_sources(${PROJECT_NAME} PRIVATE ${WINRC_FOLDER})
endif()
target_sources(${PROJECT_NAME} PRIVATE "${SRC_DIR}/AppCUI.natvis")

target_link_libraries(${PROJECT_NAME} PRIVATE AppCUICore)
set(APPCUI_LINKED_FRONTENDS "")
foreach(frontend SDL Ncurses WindowsConsole)
    if (TARGET AppCUIFrontend${frontend})
        target_link_libraries(${PROJECT_NAME} PRIVATE AppCUIFrontend${frontend})
        list(APPEND APPCUI_LINKED_FRONTENDS ${frontend})
    endif()
endforeach()
message(STATUS "${PROJECT_NAME} => frontends linked into lib${PROJECT_NAME}: ${APPCUI_LINKED_FRONTENDS} (+ Tests, Custom)")

# Preserve source file paths in debug mode for shared lib
if (DEBUG_BUILD)
    set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")
endif()

# sln configuration for MSVC
# headers have to be inserted manually as target_sources
# for VS to see them
if (MSVC)
    file(GLOB_RECURSE APP_CUI_HEADERS include/*.hpp)
    target_sources(${PROJECT_NAME} PRIVATE ${APP_CUI_HEADERS})
    set(APP_CUI_SOURCES "")
    foreach(component ${PROJECT_NAME} AppCUICore AppCUIFrontendSDL AppCUIFrontendWindowsConsole)
        if (TARGET ${component})
            get_target_property(COMPONENT_SOURCES ${component} SOURCES)
            list(APPEND APP_CUI_SOURCES ${COMPONENT_SOURCES})
        endif()
    endforeach()
    source_group(TREE ${CMAKE_SOURCE_DIR}/${PROJECT_NAME} FILES ${APP_CUI_SOURCES} ${APP_CUI_HEADERS})
endif()
