cmake_minimum_required(VERSION 3.13)

# Project setup
project(appcui VERSION 1.0)

if (MSVC)
add_definitions(-DBUILD_FOR_WINDOWS)
set(CMAKE_CXX_FLAGS "-W3")
else()
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS "-W")
endif()

# removed temporarily for build
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(AppCUI/include)

# libAppCUI
add_library(AppCUI SHARED)
add_subdirectory(AppCUI/src)
target_compile_definitions(AppCUI PRIVATE -DBUILD_AS_DYNAMIC_LIB)

# sln configuration for MSVC
# headers have to be inserted manually as target_sources 
# for VS to see them
file(GLOB_RECURSE APP_CUI_HEADERS AppCUI/include/*.h)
target_sources(AppCUI PRIVATE ${APP_CUI_HEADERS})
get_target_property(APP_CUI_SOURCES AppCUI SOURCES)
source_group(TREE ${CMAKE_SOURCE_DIR}/AppCUI FILES ${APP_CUI_SOURCES} ${APP_CUI_HEADERS})


# tester
add_executable(tester Tester/Tester.cpp)
target_link_libraries(tester AppCUI)
