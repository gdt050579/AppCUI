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

set(LIBRARIES # add all the libraries here from "libs" folder (LIBS_DIR) here
    "${LIBS_DIR}/lodepng"
)

message("${PROJECT_NAME} INCLUDE_DIR: ${INCLUDE_DIR}")
message("${PROJECT_NAME} SRC_DIR: ${SRC_DIR}")
message("${PROJECT_NAME} LIBS_DIR: ${LIBS_DIR}")
message("${PROJECT_NAME} MAIN_HEADER: ${MAIN_HEADER}")

include_directories("${INCLUDE_DIR}")
include_directories("${SRC_DIR}")

# Add libAppCUI
add_library(${PROJECT_NAME} SHARED)

# versioning
file(READ "${MAIN_HEADER}" appcui_content)
string(REGEX MATCH "#define APPCUI_VERSION \"([0-9]+.[0-9]+.[0-9]+)\"" _ ${appcui_content})
set(APPCUI_VERSION ${CMAKE_MATCH_1})
message("${PROJECT_NAME} version: ${APPCUI_VERSION}")
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${APPCUI_VERSION} SOVERSION ${APPCUI_VERSION})

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "lib")
target_compile_definitions(${PROJECT_NAME} PRIVATE -DBUILD_AS_DYNAMIC_LIB)

if (MSVC)
    set(WINRC_FOLDER "windows.rc")
    message("Copy to ${WINRC_FOLDER}")
    configure_file("resources/windows.rc.in" ${WINRC_FOLDER} @ONLY)  
    target_sources(${PROJECT_NAME} PRIVATE ${WINRC_FOLDER})
endif()

# Add SDL
include(FindSDL2)
include(FindSDL2TTF)

find_package(unofficial-brotli CONFIG REQUIRED)
find_package(BZip2 REQUIRED)
find_package(PNG REQUIRED) 
find_package(Freetype REQUIRED) 
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2TTF REQUIRED)

if (UNIX)
    if (APPLE)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Cocoa -framework OpenGL -framework IOKit")
    else()
        target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_DL_LIBS})

    	find_package(X11 CONFIG)
        message("X11_LIBRARIES => ${X11_LIBRARIES}")
	target_link_libraries(${PROJECT_NAME} PRIVATE ${X11_LIBRARIES})
    endif()	
endif()

message("BZIP2_LIBRARIES => ${BZIP2_LIBRARIES}")
target_link_libraries(${PROJECT_NAME} PRIVATE ${BZIP2_LIBRARIES})

message("BROTLICOMMON_LIBRARIES} => ${BROTLICOMMON_LIBRARIES}")
message("BROTLIDEC_LIBRARIES => ${BROTLIDEC_LIBRARIES}")
target_link_libraries(${PROJECT_NAME} PRIVATE ${BROTLIDEC__LIBRARIES} ${BROTLICOMMON_LIBRARIES})

message("PNG_LIBRARIES => ${PNG_LIBRARIES}")
target_link_libraries(${PROJECT_NAME} PRIVATE ${PNG_LIBRARIES})

message("FREETYPE_LIBRARIES => ${FREETYPE_LIBRARIES}")
target_link_libraries(${PROJECT_NAME} PRIVATE ${FREETYPE_LIBRARIES})

# Add font used by SDL 
include(CMakeRC)

set(FONT_NAME Courier.ttf)
set(FONT_PATH resources/${FONT_NAME})
target_compile_definitions(${PROJECT_NAME} PRIVATE FONT_PATH="${FONT_PATH}")
cmrc_add_resource_library(font ${FONT_PATH})
set(FONT_LIB_NAME font)

target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIR} ${SDL2TTF_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES} ${SDL2TTF_LIBRARY})
target_link_libraries(${PROJECT_NAME} PRIVATE ${FONT_LIB_NAME})

# NCURSES
if (UNIX)
    if (APPLE)
    	add_definitions(-D_XOPEN_SOURCE_EXTENDED)
    endif()
    
    set(CURSES_NEED_WIDE TRUE)
    set(CURSES_NEED_NCURSES TRUE)
    find_package(Curses REQUIRED)

    message("CURSES FOUND!")
    message("CURSES_INCLUDE_DIRS => ${CURSES_INCLUDE_DIRS}")
    message("CURSES_LIBRARIES => ${CURSES_LIBRARIES}")
    target_compile_definitions(${PROJECT_NAME} PRIVATE APPCUI_HAS_NCURSES)
    target_compile_definitions(${PROJECT_NAME} PRIVATE NCURSES_WIDECHAR)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CURSES_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CURSES_LIBRARIES})
endif()

if (UNIX AND NOT APPLE AND NOT (CMAKE_SYSTEM_NAME MATCHES "FreeBSD"))
    target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
endif()

add_subdirectory("${SRC_DIR}")
add_subdirectory("${LIBS_DIR}")

# lodepng 
target_include_directories(${PROJECT_NAME} PUBLIC "${LIBS_DIR}/lodepng")
target_compile_definitions(${PROJECT_NAME} PUBLIC -DLODEPNG_NO_COMPILE_ENCODER)
target_compile_definitions(${PROJECT_NAME} PUBLIC -DLODEPNG_NO_COMPILE_DISK)
target_compile_definitions(${PROJECT_NAME} PUBLIC -DLODEPNG_NO_COMPILE_ANCILLARY_CHUNKS)

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
    get_target_property(APP_CUI_SOURCES ${PROJECT_NAME} SOURCES)
    source_group(TREE ${CMAKE_SOURCE_DIR}/${PROJECT_NAME} FILES ${APP_CUI_SOURCES} ${APP_CUI_HEADERS})
endif()
