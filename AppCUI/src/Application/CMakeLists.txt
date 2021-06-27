target_sources(AppCUI PRIVATE
    Application.cpp
    CommandBar.cpp
    CommandBarController.cpp
    Config.cpp
    ConsoleRenderer.cpp
    Desktop.cpp
)
if (MSVC)
target_sources(AppCUI PRIVATE WindowsImplementation.cpp)
else()
target_sources(AppCUI PRIVATE MacImplementation.cpp)
endif()