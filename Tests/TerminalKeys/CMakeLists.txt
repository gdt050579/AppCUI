set(PROJECT_NAME terminalkeys)
# the key decisions of the ncurses frontend, compiled on their own: no terminal and no AppCUI library needed
add_executable(${PROJECT_NAME} TerminalKeys.cpp ../../AppCUI/src/Terminal/NcursesTerminal/TerminalKeyDecoding.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Tests")
add_test(NAME TerminalKeys COMMAND ${PROJECT_NAME})
