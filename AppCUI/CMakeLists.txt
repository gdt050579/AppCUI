include_directories(include)
if (MSVC)
    include_directories(include/Windows)
else()
    include_directories(include/Unix)
endif()

# Add libAppCUI
add_library(AppCUI SHARED)

if (UNIX)
    include(CMakeRC)
    cmrc_add_resource_library(font resources/CourierNew.ttf)

    find_package(SDL2 REQUIRED)
    find_package(SDL2TTF REQUIRED)
    target_include_directories(AppCUI PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2TTF_INCLUDE_DIR})
    if (APPLE)
        target_link_libraries(AppCUI PRIVATE ${SDL2_LIBRARIES} ${SDL2TTF_LIBRARY} font)
    else()
        target_link_libraries(AppCUI PRIVATE ${SDL2_LIBRARIES} ${SDL2TTF_LIBRARY} font lstdc++fs)
    endif()
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
file(GLOB_RECURSE APP_CUI_HEADERS include/*.h)
target_sources(AppCUI PRIVATE ${APP_CUI_HEADERS})
get_target_property(APP_CUI_SOURCES AppCUI SOURCES)
source_group(TREE ${CMAKE_SOURCE_DIR}/AppCUI FILES ${APP_CUI_SOURCES} ${APP_CUI_HEADERS})
