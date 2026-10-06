Getting started
===============

Requirements
------------

AppCUI is a C++20 library built with CMake. Its dependencies (SDL2, SDL2_ttf, FreeType, libpng, libjpeg-turbo,
bzip2, brotli and, on Linux / macOS, ncurses) are built automatically by the `vcpkg <https://vcpkg.io>`_ copy that is
included in the repository as a git submodule.

* a C++20 compiler: Visual Studio 2022 (Windows), GCC or Clang (Linux), Apple Clang / Xcode command line tools (macOS)
* CMake 3.13 or newer (and, preferably, Ninja)
* git (the vcpkg submodule)
* Linux only: the X11 / OpenGL development packages needed to build SDL2. The packages installed by the CI are a
  known-good set:

  .. code-block:: bash

     sudo apt-get install -y build-essential cmake ninja-build libsdl2-dev libsdl2-ttf-dev libltdl-dev

Building
--------

.. code-block:: bash

   git clone --recursive https://github.com/gdt050579/AppCUI.git
   cd AppCUI
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release

The first configuration builds the dependencies (this takes a while; vcpkg caches them afterwards). When AppCUI is the
top level project, the library (``libAppCUI``) and every example of the ``Examples`` folder are written to the
``bin`` folder. If the repository was cloned without ``--recursive``, run ``git submodule update --init --recursive``.

Using AppCUI in your project
----------------------------

Add the repository as a sub-folder (for example a git submodule) and link the ``AppCUI`` target:

.. code-block:: cmake

   cmake_minimum_required(VERSION 3.13)
   # the toolchain must be set before project(): it builds the dependencies of AppCUI
   set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/AppCUI/vcpkg/scripts/buildsystems/vcpkg.cmake"
       CACHE STRING "Vcpkg toolchain file")
   project(MyApp LANGUAGES CXX)
   set(CMAKE_CXX_STANDARD 20)

   add_subdirectory(AppCUI)
   add_executable(MyApp main.cpp)
   target_include_directories(MyApp PRIVATE AppCUI/AppCUI/include)
   target_link_libraries(MyApp PRIVATE AppCUI)

As vcpkg runs in manifest mode, your project needs a ``vcpkg.json`` that lists the same dependencies as
``AppCUI/vcpkg.json`` (plus your own). AppCUI is a shared library: ship ``libAppCUI`` (and the libraries it depends
on) next to your executable.

A first application
-------------------

Everything is declared in one header, ``AppCUI.hpp``. An application initializes AppCUI, adds one or more windows to
the desktop and runs the event loop:

.. literalinclude:: ../snippets/getting_started.cpp
   :language: c++
   :start-after: // [window]
   :end-before: // [/window]

* every control is created with a :doc:`layout <layout>` string (``"d:c,w:50,h:8"`` = docked in the center, 50
  characters wide and 8 lines high);
* ``Factory::<Control>::Create(parent, ...)`` creates a control, adds it to ``parent`` and returns a non-owning
  ``Reference`` to it;
* events (button clicks, window close, ...) reach ``OnEvent`` of the control and then of its parents;
* ``Application::Run()`` returns when ``Application::Close()`` is called or when the last window is closed.

Instead of deriving classes, a control can also receive its events through *handlers* (a pointer to an object that
implements the handler interface, or a plain function / lambda without captures):

.. literalinclude:: ../snippets/handlers.cpp
   :language: c++
   :start-after: // [handlers]
   :end-before: // [/handlers]

Both samples are compiled with the examples (``docs/snippets``). The `Examples
<https://github.com/gdt050579/AppCUI/tree/main/Examples>`_ folder has a program for almost every control (buttons,
list views, tree views, grids, property lists, menus, tabs, splitters, images, ...).

Next steps
----------

* :doc:`initialization` - initialization options and the ``.ini`` settings file
* :doc:`layout` - how controls are positioned and resized
* :doc:`terminals` - the frontends (Windows console, SDL, ncurses, custom) and what each one supports
* :doc:`api/index` - every class, function and type of ``AppCUI.hpp``
