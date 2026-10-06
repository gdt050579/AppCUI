# Code samples of the documentation (docs/source/*.rst show them with `literalinclude`). They are compiled together
# with the examples, so the documentation never shows code that does not build.
foreach(snippet hello getting_started handlers initialization custom_frontend)
    set(target_name DocSample_${snippet})
    add_executable(${target_name} ${snippet}.cpp)
    target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../AppCUI/include)
    add_dependencies(${target_name} AppCUI)
    target_link_libraries(${target_name} PRIVATE AppCUI)
    set_target_properties(${target_name} PROPERTIES FOLDER "Docs")
endforeach()
