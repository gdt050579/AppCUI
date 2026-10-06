AppCUI
======

**AppCUI** is a cross-platform C++20 framework for text user interfaces (TUI). An application builds a virtual
desktop with windows and controls - buttons, text fields, list views, tree views, grids, property lists, tabs,
splitters, menus, images, ... - that work with the keyboard and the mouse, and AppCUI displays it in the Windows
console, in any terminal (ncurses), in an SDL window or through a frontend implemented by the application.

.. literalinclude:: ../snippets/hello.cpp
   :language: c++
   :start-after: // [hello]
   :end-before: // [/hello]

.. toctree::
   :maxdepth: 2
   :caption: Guides

   getting_started
   initialization
   layout
   colors
   terminals

.. toctree::
   :maxdepth: 2
   :caption: Reference

   api/index

.. toctree::
   :maxdepth: 1
   :caption: Contributing

   development

* :ref:`genindex`
* :ref:`search`
