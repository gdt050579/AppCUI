cmake_minimum_required(VERSION 3.13)

# Project setup
project(appcui VERSION 1.0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

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

if (UNIX)
    add_compile_definitions(-fPIC)
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


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")


add_subdirectory(AppCUI)
add_subdirectory(Tests/Tester)
add_subdirectory(Tests/FileTest)

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
