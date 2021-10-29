cmake_minimum_required(VERSION 3.13)

# Project setup
project(appcui VERSION 1.0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DEBUG_BUILD 1)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(APPCUI_ENABLE_TESTS 1)
    set(APPCUI_ENABLE_EXAMPLES 1)
    set(APPCUI_ENABLE_OUTPUT_LOCATION 1)
endif()

if (MSVC)
    add_definitions(-DBUILD_FOR_WINDOWS)
    add_compile_options(-W3)
elseif (APPLE)
    add_definitions(-DBUILD_FOR_OSX)
    if (DEBUG_BUILD)
        add_compile_options(-g)
        add_compile_options(-W)
    endif()
elseif (UNIX)
    add_definitions(-DBUILD_FOR_UNIX)
    if (DEBUG_BUILD)
        add_compile_options(-g)
        add_compile_options(-W)
    endif()
endif()

add_compile_definitions(APPCUI_ENABLE_CONTROL_BORDER_MODE)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
     add_compile_options(-Wall -Wextra
             -Wno-switch
             -Wpointer-arith
             -Wcast-qual
             -Wredundant-decls
             -Wctor-dtor-privacy
             -Wwrite-strings
             -Wdisabled-optimization
             -Wzero-as-null-pointer-constant
             -Wvla) # -pedantic
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.0)
    add_compile_options(-Wtrampolines -Wduplicated-cond -Wduplicated-branches -Wnull-dereference)
endif()

if (NOT DEBUG_BUILD)
    if (MSVC)
        #add_compile_options(-O2)
        add_compile_options(-Ob2)
	#string( REPLACE "/RTC1" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} )
    else()
        add_compile_options(-O3)
    endif()
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(APPCUI_ENABLE_OUTPUT_LOCATION)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
endif()
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")


add_subdirectory(AppCUI)

if(APPCUI_ENABLE_TESTS)
    add_subdirectory(Tests/Tester)
    add_subdirectory(Tests/FileTest)
endif()

if(APPCUI_ENABLE_EXAMPLES)
    #examples
    add_subdirectory(Examples/SimpleWindow)
    add_subdirectory(Examples/MultipleWindows)
    add_subdirectory(Examples/Checkboxes)
    add_subdirectory(Examples/Radioboxes)
    add_subdirectory(Examples/Buttons)
    add_subdirectory(Examples/CommandBar)
    add_subdirectory(Examples/TextField)
    add_subdirectory(Examples/MessageBox)
    add_subdirectory(Examples/Splitter)
    add_subdirectory(Examples/TabControl)
    add_subdirectory(Examples/TicTacToe)
    add_subdirectory(Examples/Logo)
    add_subdirectory(Examples/Logs)
    add_subdirectory(Examples/PythonEditor)
    add_subdirectory(Examples/ListView)
    add_subdirectory(Examples/ProgressStatus)
    add_subdirectory(Examples/ComboBox)
    add_subdirectory(Examples/TerminalSettings)
    add_subdirectory(Examples/IniObject)
    add_subdirectory(Examples/IniInitialization)
    add_subdirectory(Examples/FileDialog)
    add_subdirectory(Examples/UnicodeText)
    add_subdirectory(Examples/TextRenderer)
    add_subdirectory(Examples/NumericSelector)
    add_subdirectory(Examples/ContextMenu)
    add_subdirectory(Examples/MenuBar)
    add_subdirectory(Examples/WindowMenuBar)
    add_subdirectory(Examples/ImageView)
    add_subdirectory(Examples/SimpleHexView)
    add_subdirectory(Examples/ToolTip)
    add_subdirectory(Examples/FontTest)
    add_subdirectory(Examples/Layout)
    add_subdirectory(Examples/WindowArrange)
    add_subdirectory(Examples/CustomDesktop)
    add_subdirectory(Examples/WindowControlsBar)
    add_subdirectory(Examples/WindowReferal)
    add_subdirectory(Examples/CurrentPath)
    add_subdirectory(Examples/BadApple)
    add_subdirectory(Examples/Frames)
    add_subdirectory(Examples/TreeView)
    add_subdirectory(Examples/LogIn)
    add_subdirectory(Examples/SingleAppWindow)
    add_subdirectory(Examples/RemoveControl)
    add_subdirectory(Examples/Tetris)
endif()
