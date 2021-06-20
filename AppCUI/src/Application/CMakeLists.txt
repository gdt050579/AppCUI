target_sources(AppCUI PUBLIC
    Application.cpp
    CommandBarController.cpp
    Config.cpp
    ConsoleRenderer.cpp
    Desktop.cpp
)
if (MSVC)
target_sources(AppCUI PUBLIC WindowsImplementation.cpp)
endif()