Frontends (terminals)
=====================

AppCUI draws every control into an in-memory screen; a *frontend* displays that screen and produces the keyboard /
mouse events. The frontend is selected with ``InitializationData::Frontend`` or with the ``Frontend`` key of the
settings file (see :doc:`initialization`):

.. list-table::
   :header-rows: 1
   :widths: 20 15 65

   * - Frontend
     - Systems
     - Description
   * - ``WindowsConsole``
     - Windows
     - the Windows console (the default on Windows)
   * - ``Terminal``
     - Linux, macOS
     - any terminal emulator, through ncurses (the default on Linux and macOS)
   * - ``SDL``
     - all
     - a separate window rendered with SDL2 and an embedded font (JuliaMono)
   * - ``Browser``
     - web browsers
     - an HTML5 canvas inside a web page (``wasm32-emscripten`` builds; the default there) - see below
   * - ``Custom``
     - all
     - implemented by the application (headless, remote or recorded screens) - see below
   * - ``Tests``
     - all
     - an in-memory screen driven by a test script (``Application::InitForTests`` / ``RunTestScript``)

Frontends linked into libAppCUI
-------------------------------

Each native frontend is a separate CMake component, linked statically into ``libAppCUI``. ``AppCUI::Core`` holds
everything else (including the ``Tests`` and ``Custom`` frontends) and depends on none of the UI libraries:

.. list-table::
   :header-rows: 1
   :widths: 35 35 30

   * - CMake target
     - Option (default)
     - Dependencies
   * - ``AppCUI::Core``
     - always built
     - libpng, libjpeg-turbo (only with ``APPCUI_IMAGE_CODECS``, ``ON`` except for ``wasm32-emscripten``)
   * - ``AppCUI::Frontend::SDL``
     - ``APPCUI_FRONTEND_SDL`` (``ON``)
     - SDL2, SDL2_ttf, FreeType (+ X11 on Linux)
   * - ``AppCUI::Frontend::Ncurses``
     - ``APPCUI_FRONTEND_NCURSES`` (``ON`` on Linux / macOS)
     - ncursesw
   * - ``AppCUI::Frontend::WindowsConsole``
     - ``APPCUI_FRONTEND_WINDOWS_CONSOLE`` (``ON`` on Windows)
     - none
   * - ``AppCUI::Frontend::Browser``
     - ``APPCUI_FRONTEND_BROWSER`` (``ON`` for ``wasm32-emscripten``, the only frontend available there)
     - none (a JavaScript host, ``AppCUI/web/appcui-host.js``)

By default every frontend available on the platform is linked. Turning an option ``OFF`` (for example
``-DAPPCUI_FRONTEND_SDL=OFF`` for a console-only application) removes that frontend and its libraries from
``libAppCUI``; requesting it afterwards makes ``Application::Init`` fail (the error is logged).
``FrontendType::Default`` selects the native frontend of the platform (Windows console, ncurses or, in a web browser,
the browser frontend) or, when that one is not linked, SDL.

Supported features
------------------

.. list-table::
   :header-rows: 1
   :widths: 32 17 17 17 17

   * - Feature
     - Windows console
     - SDL
     - ncurses
     - Browser
   * - 16 colors
     - Yes
     - Yes
     - Yes (8-color terminals show bright colors as normal ones; no colors below 8)
     - Yes
   * - Unicode special characters (box drawing, arrows, ...)
     - Yes
     - Yes
     - Yes (not on the Linux text console, ``TERM=linux``)
     - Yes
   * - FPS mode (``InitializationFlags::EnableFPSMode``, ~30 updates per second)
     - Yes
     - Yes
     - Yes
     - Yes
   * - Text cursor (caret)
     - Yes
     - No
     - Yes
     - Yes
   * - Mouse: left button
     - Yes
     - Yes
     - Yes
     - Yes
   * - Mouse: right / middle button
     - Right only
     - Yes
     - No
     - Yes
   * - Mouse: double click
     - Yes
     - No
     - No
     - Yes
   * - Mouse: move and drag
     - Yes
     - Yes
     - Depends on the terminal (``REPORT_MOUSE_POSITION``)
     - Yes
   * - Mouse: wheel
     - Yes
     - Yes
     - No
     - Yes
   * - ``Ctrl+key``, ``Shift+key``, ``Alt+key`` and their combinations
     - Yes
     - Yes
     - Partial (see below)
     - Yes, except the shortcuts reserved by the browser (``Ctrl+W``, ``Ctrl+T``, ``Ctrl+N``, ...)
   * - Clipboard
     - Yes
     - Windows only
     - No
     - Private to the page
   * - Resize events
     - Yes
     - Yes
     - Yes
     - Yes (the size of the page element)
   * - Maximized / full screen / fixed size window
     - Yes
     - Yes
     - No (the terminal decides)
     - No (the page decides)
   * - Character size (``CharacterSize``)
     - Yes
     - Yes
     - No (the terminal decides)
     - No (``fontSizeCss`` of the host)
   * - Font name (``FontName``)
     - Yes
     - No (embedded font)
     - No (the terminal decides)
     - No (``fontFamily`` of the host)

Browser (WebAssembly)
---------------------

An AppCUI application compiled with `Emscripten <https://emscripten.org>`_ runs inside a web page: the browser frontend
renders the screen into an HTML5 canvas (with the JuliaMono font) and translates the keyboard / mouse events of the
page. The application code does not change - ``Application::Init()`` selects the browser frontend by default:

.. code-block:: bash

   emcmake cmake -S . -B build-wasm -G Ninja -DCMAKE_BUILD_TYPE=Release
   cmake --build build-wasm --target MessageBox

Every example becomes a small static site in ``bin/wasm``: ``<Example>.html`` + ``<Example>.mjs`` + ``<Example>.wasm``,
next to ``appcui-host.js`` and ``JuliaMono-Regular.ttf``. Browsers do not load WebAssembly from ``file://`` URLs, so
the pages must be served over HTTP; ``bin/wasm/serve.py`` does that and opens an example in the browser:

.. code-block:: bash

   python bin/wasm/serve.py MessageBox

Any other web server works too, as long as it serves ``.mjs`` as ``text/javascript`` and ``.wasm`` as
``application/wasm``. A page of your own boots an application with:

.. code-block:: javascript

   import createAppCUIModule from './MyApp.mjs';
   import { startAppCUI } from './appcui-host.js';

   await startAppCUI(createAppCUIModule, canvasElement, containerElement,
                     { fontFamily: '"JuliaMono", monospace', fontSizeCss: 16 });

The size of ``containerElement`` decides the number of columns and rows (resizing it resizes the application).

* Modal windows (``Window::Show``, ``MessageBox``, ``FileDialog``, ...) keep their synchronous API: while a modal
  window waits for input, the C++ stack is suspended and the browser keeps running. The default build suspends with
  Asyncify (every browser); ``-DAPPCUI_WASM_JSPI=ON`` uses JSPI instead (smaller and faster ``.wasm``, requires a
  browser with JavaScript Promise Integration).
* With Asyncify, ``Application::Run()`` does not return: the browser drives the event loop until the application
  closes and code placed after ``Run()`` in ``main`` is never executed. With JSPI, ``Run()`` returns normally.
* When the application exits (for example after its last window is closed), the page keeps the last frame, dimmed,
  under an *"The application has exited"* message with a **Restart** button. Pass ``onExit: (host) => { ... }`` in
  the options of ``startAppCUI`` to handle the exit differently.
* The file system is Emscripten's in-memory file system, separate from the user's machine, and its files do not persist
  across page reloads. The application starts in the home folder (``/home/web_user``).

Files of the user
~~~~~~~~~~~~~~~~~

The browser frontend moves files between the in-memory file system and the user's machine:

* ``FileDialog`` lists only real locations (**Home**, **Temp**, ``/``) and adds two buttons:

  * **Upload** opens the file picker of the browser. The picker suggests the extensions of the current file type
    filter. The picked files are copied into the folder shown by the dialog, and the first one is selected, so Enter
    opens it. The limit is 256 MB per file.
  * **Download** sends the selected file to the user as a browser download.

* When a ``ShowSaveFileWindow`` dialog returns a path, the file is downloaded automatically once the application has
  written it, typically in the same event handler. Existing applications therefore save to the user's machine without
  any change.
* ``OS::IsHostFileTransferAvailable()`` tells whether the frontend works this way, and ``OS::DownloadFile(path)``
  downloads any file explicitly, for example a log. On the other frontends the first returns ``false`` and the second
  does nothing.
* Pass ``onDownload: (name, data) => { ... }`` to ``startAppCUI`` to handle the downloaded bytes in the page instead of
  starting a browser download.
* PNG / JPG images are not supported (``APPCUI_IMAGE_CODECS`` is ``OFF``); BMP images are.

Keyboard on ncurses terminals
-----------------------------

Terminals send many key combinations as escape sequences and some combinations are not sent at all. AppCUI
recognizes:

* ``Ctrl+<letter>``, ``Ctrl+Space``, ``Shift+Tab``, the function keys and their Shift / Ctrl / Alt variants, and the
  modified navigation keys (arrows, ``Home``, ``End``, ``PageUp``, ``PageDown``, ``Insert``, ``Delete``) as
  described by the terminfo entry of the terminal (xterm-compatible terminals report all of them);
* ``Alt+<key>`` when the terminal sends Alt (Meta) as an ``Escape`` prefix (on macOS enable "Use Option as Meta key"
  in Terminal / iTerm2);
* **combo mode**, for combinations the terminal can not send: press :kbd:`\`` (backtick), toggle the modifiers with
  :kbd:`a` (Alt), :kbd:`c` (Ctrl) and :kbd:`s` (Shift), then press a letter or a digit (``1`` - ``9``, ``0`` are the
  function keys ``F1`` - ``F10``). :kbd:`Space` keeps combo mode active for several keys and :kbd:`Escape` leaves it.
  The selected modifiers are shown on the screen while combo mode is active.

The ``Keyboard.Ctrl`` / ``Keyboard.Alt`` keys of the settings file (see :doc:`initialization`) remap the modifiers
when a terminal or a keyboard layout makes some of them hard to use.

Custom frontends
----------------

An application can implement its own frontend (``FrontendType::Custom``): AppCUI then calls an object that implements
``Application::CustomFrontendInterface`` instead of a terminal. This is how an application can run without a terminal
(tests, screenshots), stream its screen over the network or record a session.

.. code-block:: c++

   class CustomFrontendInterface
   {
     public:
       virtual bool OnInit(uint32& width, uint32& height) = 0;          // choose the screen size
       virtual void OnUnInit() = 0;                                     // the application stops
       virtual void OnFlushToScreen(const Graphics::Character* characters, uint32 width, uint32 height) = 0;
       virtual void OnUpdateCursor(uint32 x, uint32 y, bool visible) = 0;
       virtual bool WaitForEvent(FrontendEvent& evnt, uint32 timeoutMs) = 0; // false = no event (timeout)
       virtual bool HasSupportFor(SpecialCharacterSetType type);        // default: every set
   };

* AppCUI calls every method from its own (UI) thread. ``WaitForEvent`` may be fed by other threads (for example a
  network thread that pushes events into a queue): it must return within ``timeoutMs`` milliseconds so that the frame
  updates of ``EnableFPSMode`` keep running.
* ``OnFlushToScreen`` receives the whole screen after every repaint (``width * height`` characters, row major); the
  pointer is only valid during the call.
* Events are ``FrontendEvent`` objects: key presses (key code + Unicode character), shift state changes, mouse
  down / up / move / wheel, resize, close and redraw requests.
* The object is not owned by AppCUI and must outlive ``Application::Run()``.
* The ``Frontend`` and ``Size`` keys of the settings file are ignored, and the clipboard is private to the process
  (a custom frontend never reads or writes the clipboard of the operating system).

A complete headless frontend that renders an application into memory and prints the screen:

.. literalinclude:: ../snippets/custom_frontend.cpp
   :language: c++
   :start-after: // [frontend]
   :end-before: // [/frontend]

.. literalinclude:: ../snippets/custom_frontend.cpp
   :language: c++
   :start-after: // [usage]
   :end-before: // [/usage]
   :dedent: 4

Output::

   ############################################################
   ############################################################
   ############################################################
   ##########+============ Headless =============[x]+##########
   ##########|                                      |##########
   ##########| Rendered without a terminal          |##########
   ##########|                                      |##########
   ##########|                                      |##########
   ##########+======================================+##########
   ############################################################
   ############################################################
   ############################################################
