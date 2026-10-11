Live demos
==========

The examples below are the AppCUI examples from the `Examples <https://github.com/gdt050579/AppCUI/tree/main/Examples>`_
folder, compiled to WebAssembly and running in your browser through the browser frontend (see :doc:`terminals`). Each
one is the same C++ code that runs in the Windows console, in a terminal or in an SDL window - only the frontend
differs. Click inside a demo to give it the keyboard focus; close its last window to end it.

..
   The publishing workflow (.github/workflows/gh-pages.yml) compiles exactly the examples linked below as
   `demo/<Example>.html` and copies them next to this page: adding a link adds a demo.

Controls
--------

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Demo
     - What it shows
   * - `Buttons <demo/Buttons.html>`_
     - push buttons, hot keys, keyboard navigation
   * - `Checkboxes <demo/Checkboxes.html>`_
     - check boxes and their states
   * - `Radioboxes <demo/Radioboxes.html>`_
     - radio buttons grouped by id
   * - `TextField <demo/TextField.html>`_
     - single / multi-line text fields, read-only and syntax highlighted text, custom context menus
   * - `ComboBox <demo/ComboBox.html>`_
     - drop-down lists
   * - `ListView <demo/ListView.html>`_
     - sortable columns, column separators, item selection (US states)
   * - `TreeView <demo/TreeView.html>`_
     - expandable hierarchies
   * - `Grid <demo/Grid.html>`_
     - a spreadsheet-like grid with cell selection
   * - `Property <demo/Property.html>`_
     - a property list that edits the properties of an object
   * - `NumericSelector <demo/NumericSelector.html>`_
     - numeric input with increment / decrement
   * - `ColorPicker <demo/ColorPicker.html>`_
     - picking one of the 16 colors
   * - `KeySelector <demo/KeySelector.html>`_
     - capturing a key combination
   * - `TabControl <demo/TabControl.html>`_
     - tabs on top, bottom, left or as a list
   * - `Splitter <demo/Splitter.html>`_
     - resizable vertical / horizontal splitters
   * - `ToolTip <demo/ToolTip.html>`_
     - tool tips on hover
   * - `ProgressStatus <demo/ProgressStatus.html>`_
     - progress reporting for long operations

Windows, menus and dialogs
--------------------------

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Demo
     - What it shows
   * - `MessageBox <demo/MessageBox.html>`_
     - notifications, errors, warnings and questions (modal dialogs)
   * - `FileDialog <demo/FileDialog.html>`_
     - open / save dialogs (browsing the in-memory file system of the page)
   * - `MenuBar <demo/MenuBar.html>`_
     - an application menu bar with sub-menus
   * - `ContextMenu <demo/ContextMenu.html>`_
     - right-click menus
   * - `CommandBar <demo/CommandBar.html>`_
     - the command bar (shortcut keys) on the last line
   * - `WindowManager <demo/WindowManager.html>`_
     - the window manager dialog
   * - `WindowArrange <demo/WindowArrange.html>`_
     - cascade, grid, vertical and horizontal window arrangements
   * - `ThemeEditor <demo/ThemeEditor.html>`_
     - editing the color theme live

Graphics and games
------------------

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Demo
     - What it shows
   * - `CharacterTable <demo/CharacterTable.html>`_
     - the characters AppCUI can display
   * - `UnicodeText <demo/UnicodeText.html>`_
     - Unicode text rendering
   * - `Rectangles <demo/Rectangles.html>`_
     - custom drawing with lines and box characters
   * - `Logo <demo/Logo.html>`_
     - the AppCUI logo, drawn by an AppCUI application
   * - `Tetris <demo/Tetris.html>`_
     - a game driven by the FPS mode (~30 updates per second)
   * - `Snake <demo/Snake.html>`_
     - another FPS-mode game
   * - `TicTacToe <demo/TicTacToe.html>`_
     - a mouse-driven game
