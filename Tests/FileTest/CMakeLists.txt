set(PROJECT_NAME TestFile)
include_directories(../../AppCUI/include)
add_executable(${PROJECT_NAME} FileTest.cpp)
add_dependencies(${PROJECT_NAME} AppCUI)
target_link_libraries(${PROJECT_NAME} PRIVATE AppCUI)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Tests")

