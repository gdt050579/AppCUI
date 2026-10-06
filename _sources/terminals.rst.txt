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
   * - ``Custom``
     - all
     - implemented by the application (headless, remote or recorded screens) - see below
   * - ``Tests``
     - all
     - an in-memory screen driven by a test script (``Application::InitForTests`` / ``RunTestScript``)

Supported features
------------------

.. list-table::
   :header-rows: 1
   :widths: 40 20 20 20

   * - Feature
     - Windows console
     - SDL
     - ncurses
   * - 16 colors
     - Yes
     - Yes
     - Yes (8-color terminals show bright colors as normal ones; no colors below 8)
   * - Unicode special characters (box drawing, arrows, ...)
     - Yes
     - Yes
     - Yes (not on the Linux text console, ``TERM=linux``)
   * - FPS mode (``InitializationFlags::EnableFPSMode``, ~30 updates per second)
     - Yes
     - Yes
     - Yes
   * - Text cursor (caret)
     - Yes
     - No
     - Yes
   * - Mouse: left button
     - Yes
     - Yes
     - Yes
   * - Mouse: right / middle button
     - Right only
     - Yes
     - No
   * - Mouse: double click
     - Yes
     - No
     - No
   * - Mouse: move and drag
     - Yes
     - Yes
     - Depends on the terminal (``REPORT_MOUSE_POSITION``)
   * - Mouse: wheel
     - Yes
     - Yes
     - No
   * - ``Ctrl+key``, ``Shift+key``, ``Alt+key`` and their combinations
     - Yes
     - Yes
     - Partial (see below)
   * - Clipboard
     - Yes
     - Windows only
     - No
   * - Resize events
     - Yes
     - Yes
     - Yes
   * - Maximized / full screen / fixed size window
     - Yes
     - Yes
     - No (the terminal decides)
   * - Character size (``CharacterSize``)
     - Yes
     - Yes
     - No (the terminal decides)
   * - Font name (``FontName``)
     - Yes
     - No (embedded font)
     - No (the terminal decides)

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
