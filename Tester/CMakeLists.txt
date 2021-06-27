# tester
include_directories(../AppCUI/include)
add_executable(tester Tester.cpp)
target_link_libraries(tester PUBLIC AppCUI)
set_target_properties(tester PROPERTIES FOLDER "Tests")

