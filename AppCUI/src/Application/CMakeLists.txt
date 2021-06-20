target_sources(AppCUI PUBLIC
    Application.cpp
    CommandBarController.cpp
    Config.cpp
    ConsoleRenderer.cpp
    Desktop.cpp
)
if (CMAKE_WIN)
target_sources(AppCUI WindowsImplementation.cpp)
endif()