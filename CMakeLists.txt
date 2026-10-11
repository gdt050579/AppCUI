cmake_minimum_required(VERSION 3.13)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
  CACHE STRING "Vcpkg toolchain file")

message("CMAKE_TOOLCHAIN_FILE => ${CMAKE_TOOLCHAIN_FILE}")

# Project setup
project(AppCUI VERSION 1.0)

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
    add_compile_options(-W3 /MP)
elseif (APPLE)
    add_definitions(-DBUILD_FOR_OSX)
    if (DEBUG_BUILD)
        add_compile_options(-g)
        add_compile_options(-W)
    endif()

    set(CMAKE_MACOSX_RPATH TRUE)
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)

    SET(CMAKE_INSTALL_NAME_DIR @executable_path)
    message(STATUS "CMAKE_INSTALL_NAME_DIR => ${CMAKE_INSTALL_NAME_DIR}")
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
    SET(CMAKE_INSTALL_RPATH @executable_path)
    message(STATUS "CMAKE_INSTALL_RPATH => ${CMAKE_INSTALL_RPATH}")
    
    if(CMAKE_LINK_LIBRARY_USING_weak_library_SUPPORTED)
        message(STATUS "${PROJECT_NAME} => CMAKE_LINK_LIBRARY_USING_weak_library_SUPPORTED")
    	target_link_libraries(${PROJECT_NAME} PRIVATE "$<LINK_LIBRARY:weak_library,lib,external>")
    else()
        message(STATUS "${PROJECT_NAME} => NOT CMAKE_LINK_LIBRARY_USING_weak_library_SUPPORTED")
    	# target_link_libraries(${PROJECT_NAME} PRIVATE lib external)
    endif()

elseif (EMSCRIPTEN)
    # wasm32-emscripten (emcmake also sets UNIX): POSIX code paths, no rpath, no debug info by default (it would
    # bloat the .wasm)
    add_definitions(-DBUILD_FOR_UNIX)
    add_compile_options(-W)
    # Modal loops suspend the C++ stack (see AppCUI/src/Terminal/BrowserTerminal). Asyncify (default, every browser)
    # only supports the JS-based exception model; JSPI also works with native wasm exceptions. Every translation unit
    # must agree on the exception model.
    option(APPCUI_WASM_JSPI "Suspend modal loops with JSPI (native wasm exceptions) instead of Asyncify" OFF)
    if (APPCUI_WASM_JSPI)
        set(APPCUI_WASM_EXCEPTIONS -fwasm-exceptions)
    else()
        set(APPCUI_WASM_EXCEPTIONS -fexceptions)
    endif()
    add_compile_options(${APPCUI_WASM_EXCEPTIONS})
    add_link_options(${APPCUI_WASM_EXCEPTIONS})
    # libAppCUI is consumed whole-archive (see AppCUI/CMakeLists.txt); wasm-ld supports --whole-archive
    if (CMAKE_VERSION VERSION_LESS 3.24)
        message(FATAL_ERROR "A wasm32-emscripten build of AppCUI requires CMake 3.24 or newer ($<LINK_LIBRARY>)")
    endif()
    if (NOT CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE_SUPPORTED)
        set(CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE "LINKER:--whole-archive" "<LINK_ITEM>" "LINKER:--no-whole-archive")
        set(CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE_SUPPORTED TRUE)
    endif()
elseif (UNIX)
    add_definitions(-DBUILD_FOR_UNIX)
    add_compile_options(-g)
    add_compile_options(-W)

    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath -Wl,$ORIGIN")
endif()

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
        add_compile_options(-Ob2)
    else()
        add_compile_options(-O3)
    endif()
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(APPCUI_ENABLE_OUTPUT_LOCATION)
    if (EMSCRIPTEN)
        set(APPCUI_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin/wasm) # a static web site: .html + .mjs + .wasm
    else()
        set(APPCUI_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
    endif()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${APPCUI_OUTPUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${APPCUI_OUTPUT_DIRECTORY})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${APPCUI_OUTPUT_DIRECTORY})
endif()
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_subdirectory(AppCUI)

if (APPLE AND NOT APPCUI_ENABLE_OUTPUT_LOCATION)
    if (CMAKE_GENERATOR STREQUAL "Xcode")
        set_target_properties(${PROJECT_NAME} PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../bin/${CMAKE_BUILD_TYPE}"
            LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../bin/${CMAKE_BUILD_TYPE}"
            RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../bin/${CMAKE_BUILD_TYPE}"
            ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../bin/${CMAKE_BUILD_TYPE}"
            LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../bin/${CMAKE_BUILD_TYPE}"
            RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../bin/${CMAKE_BUILD_TYPE}")
    endif()
endif()

if(APPCUI_ENABLE_TESTS)
    if (NOT UNIX)
        add_subdirectory(Tests/Tester)
        add_subdirectory(Tests/FileTest)
    endif()
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
    add_subdirectory(Examples/Handlers)
    add_subdirectory(Examples/Interfaces)
    add_subdirectory(Examples/NumericFormat)
    add_subdirectory(Examples/Grid)
    add_subdirectory(Examples/Property)
    add_subdirectory(Examples/KeySelector)
    add_subdirectory(Examples/ColorPicker)
    add_subdirectory(Examples/CharacterTable)
    add_subdirectory(Examples/Tetris)
    add_subdirectory(Examples/Rectangles)
    add_subdirectory(Examples/Snake)
    add_subdirectory(Examples/CodePage)
    add_subdirectory(Examples/ThemeEditor)
    add_subdirectory(Examples/ThemeExample)
    add_subdirectory(Examples/WindowMoveBetweenControls)
    add_subdirectory(Examples/MemoryFile)
    add_subdirectory(Examples/TestScript)
    add_subdirectory(Examples/WindowManager)
    # code samples shown in the documentation (they must keep compiling)
    add_subdirectory(docs/snippets)
endif()

if (APPLE)
    	set_property(TARGET "${PROJECT_NAME}" PROPERTY INSTALL_RPATH "@loader_path")
elseif (UNIX)
	set_property(TARGET "${PROJECT_NAME}" PROPERTY INSTALL_RPATH "$ORIGIN")
endif()
