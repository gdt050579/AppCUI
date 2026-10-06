Initialization
==============

AppCUI must be initialized before any control is created, with one of:

.. code-block:: c++

   bool AppCUI::Application::Init(InitializationFlags flags = InitializationFlags::None);
   bool AppCUI::Application::Init(InitializationData& initData);

Both return ``false`` when AppCUI could not be initialized (for example when the selected frontend is not available).
``Application::Run()`` runs the event loop and un-initializes AppCUI when it returns.

Initialization flags
--------------------

``InitializationFlags`` values can be combined with ``|``:

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Flag
     - Effect
   * - ``CommandBar``
     - a command bar with shortcut keys is shown on the last line of the screen (controls fill it in
       ``Control::OnUpdateCommandBar``)
   * - ``Menu``
     - a menu bar is shown on the first line; menus are added with ``Application::AddMenu(...)``
   * - ``Maximized``
     - the terminal window is maximized (Windows console, SDL)
   * - ``Fullscreen``
     - the terminal window is full screen (Windows console, SDL)
   * - ``FixedSize``
     - the terminal window can not be resized (Windows console, SDL)
   * - ``LoadSettingsFile``
     - the file with the name of the executable and the ``.ini`` extension (``Application::GetAppSettingsFile()``)
       is loaded. Its content is available through ``Application::GetAppSettings()`` and its ``[AppCUI]`` section
       (see below) overrides the initialization parameters
   * - ``AutoHotKeyForWindow``
     - every new desktop window receives a free hot key (``Alt+1`` ... ``Alt+9``) unless it already has one
   * - ``EnableFPSMode``
     - ``Control::OnFrameUpdate`` is called about 30 times per second (animations, games, polling background work)
   * - ``SingleWindowApp``
     - single application mode: windows can not be added to the desktop and ``Application::Run()`` can not be used.
       A class derived from ``Controls::SingleApp`` replaces the desktop and is started with
       ``Application::RunSingleApp(...)``
   * - ``DisableAutoCloseDesktop``
     - the application keeps running after its last window was closed (by default it closes). Useful with a custom
       desktop or with menus that open new windows

The settings file
-----------------

With ``InitializationFlags::LoadSettingsFile``, the ``[AppCUI]`` section of ``<executable>.ini`` configures the
application. Missing keys keep the values of the initialization data. ``Application::UpdateAppCUISettings(...)``
writes the section with its default values:

.. code-block:: ini

   [AppCUI]
   Frontend = default        ; default, SDL, terminal (ncurses) or windows (Windows console)
   Size = default            ; default, maximized, fullscreen or <width>x<height> (for example 120x40)
   CharacterSize = default   ; default, tiny, small, normal, large or huge
   Fixed = false             ; true = the terminal window can not be resized
   Theme = default           ; default, dark, light or the name of a .theme file from ThemeFolder
   ThemeFolder = Themes      ; folder (relative to the executable) with .theme files
   CharacterSet = auto       ; auto, unicode, ascii or linux (special characters used to draw lines and borders)
   Keyboard.Ctrl = Ctrl      ; keyboard profile: the modifier(s) produced by the physical Ctrl key
   Keyboard.Alt = Alt        ; keyboard profile: the modifier(s) produced by the physical Alt key

``Keyboard.Ctrl`` / ``Keyboard.Alt`` remap the modifier keys for every key press and mouse event (for example
``Keyboard.Ctrl = Alt`` and ``Keyboard.Alt = Ctrl`` swap them, which helps on keyboards or terminals where some
combinations are not available). Values are combinations of ``Ctrl``, ``Alt`` (or ``Opt`` / ``Option``) and
``Shift`` joined with ``+``; the resulting mapping must be one-to-one, otherwise the default profile is used. The
profile can also be changed at runtime with ``Application::SetModifierMap(...)``.

The ``Frontend`` and ``Size`` keys are ignored by the test frontend (``Application::InitForTests``) and by custom
frontends (see :doc:`terminals`).

InitializationData
------------------

``InitializationData`` describes every parameter of ``Application::Init``:

.. code-block:: c++

   struct InitializationData
   {
       uint32 Width, Height;                             // size in characters (0 = frontend default)
       FrontendType Frontend;                            // frontend used to display the application
       CharacterSize CharSize;                           // character size (SDL, Windows console)
       InitializationFlags Flags;                        // see above
       string_view FontName;                             // font name (Windows console)
       Utils::FixSizeString<32> ThemeName;               // name of a .theme file to load from ThemeFolder
       Utils::String ThemeFolder;                        // default: "Themes"
       ThemeType Theme;                                  // Default, Dark or Light
       SpecialCharacterSetType SpecialCharacterSet;      // Auto, Unicode, LinuxTerminal or Ascii
       Controls::Desktop* (*CustomDesktopConstructor)(); // creates a custom desktop (nullptr = default desktop)
       CustomFrontendInterface* CustomFrontend;          // required when Frontend is FrontendType::Custom
   };

with:

.. code-block:: c++

   enum class FrontendType : uint32
   {
       Default        = 0, // Windows console on Windows, ncurses on Linux / macOS
       SDL            = 1, // a separate window rendered with SDL2 (every OS)
       Terminal       = 2, // ncurses (Linux / macOS)
       WindowsConsole = 3, // Windows console
       Tests          = 4, // in-memory screen driven by a test script (Application::InitForTests)
       Custom         = 5, // implemented by the application (CustomFrontendInterface)
   };

   enum class CharacterSize : uint32 { Default = 0, Tiny, Small, Normal, Large, Huge };

   enum class ThemeType : uint32 { Default = 0, Dark = 1, Light = 2 };

   enum class SpecialCharacterSetType : uint32 { Auto = 0, Unicode = 1, LinuxTerminal = 2, Ascii = 3 };

``SpecialCharacterSetType::Auto`` selects the best set the frontend supports (Unicode, then the Linux terminal
subset, then ASCII). See :doc:`api/index` for the complete declarations.

Examples
--------

1. Quick initialization

   .. literalinclude:: ../snippets/initialization.cpp
      :language: c++
      :start-after: // [quick]
      :end-before: // [/quick]
      :dedent: 4

2. Initialization from the ``.ini`` file

   .. literalinclude:: ../snippets/initialization.cpp
      :language: c++
      :start-after: // [ini]
      :end-before: // [/ini]
      :dedent: 4

3. Initialization from the ``.ini`` file, with a menu bar and a command bar

   .. literalinclude:: ../snippets/initialization.cpp
      :language: c++
      :start-after: // [ini-menu]
      :end-before: // [/ini-menu]
      :dedent: 4

4. Fully customized initialization: an SDL window of ``120x30`` small characters, the dark theme, Unicode special
   characters, a menu bar and a command bar

   .. literalinclude:: ../snippets/initialization.cpp
      :language: c++
      :start-after: // [custom]
      :end-before: // [/custom]
      :dedent: 4

These samples are compiled with the examples (``docs/snippets/initialization.cpp``). More examples:

* `Initialization via INI file <https://github.com/gdt050579/AppCUI/tree/main/Examples/IniInitialization>`_
* `Custom desktop <https://github.com/gdt050579/AppCUI/tree/main/Examples/CustomDesktop>`_
* `Terminal settings <https://github.com/gdt050579/AppCUI/tree/main/Examples/TerminalSettings>`_
* `Single window application <https://github.com/gdt050579/AppCUI/tree/main/Examples/SingleAppWindow>`_
