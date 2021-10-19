AppCUI Initialization
=====================

AppCUI framework must be initilized using the following APIs:

.. code-block:: c++

   bool AppCUI::Application::Init(InitializationFlags flags = InitializationFlags::None);
   bool AppCUI::Application::Init(InitializationData& initData);

Where **InitializationFlags** is defined as follows:

.. code-block:: c++

    enum class InitializationFlags : unsigned int
    {
        None                = 0,       

        CommandBar          = 0x0001,
        Menu                = 0x0002,
        Maximized           = 0x0004,
        Fullscreen          = 0x0008,
        FixedSize           = 0x0010,
        LoadSettingsFile    = 0x0020,
        AutoHotKeyForWindow = 0x0040,
        EnableFPSMode       = 0x0080,
        SingleAppWindow     = 0x0100,
    };

Width:

* **CommandBar** - if set, this flags specifies that a command bar (with shortcut keys) will be available to use
* **Menu** - if set, am application maenu bar will be created and ``Application::AddMenu(...)`` can be used to add Menus and Sub-menus to it 
* **Maximized** - maximize AppCUI OS window. Depending on the selected front-end, this feature may be limited. See AppCUI front ends for more details.
* **Fullscreen** - full screen mode. Depending on the selected front-end, this feature may be limited. See AppCUI front ends for more details.
* **FixedSize** - if set, the AppCUI OS window can not change its size. Depending on the selected front-end, this feature may be limited. See AppCUI front ends for more details.
* **LoadSettingsFile** - if set, a file with the same name as the main executable but with extension ``.ini`` will be search in the same folder as the main executable. If found, that file will be automatically loaded and its content will be available through ``Application::GetAppSettings()`` API. If a section named ``[APPCUI]`` exists in this file, the values described in this section will be used to initialized AppCUI envinronment. An example of such a ini file can be seen in the following example:
* **AutoHotKeyForWindow** - if set, any new window that is being added in the application, will automatically be assign with a hotkey (from Alt+1 to Alt+9) if that hotkey is not being used by another window and if the window that is currently being added does not already have a hot key associated.
* **EnableFPSMode** - if set, enables a mode when ``Control::OnFrameUpdate`` is called 30 times per second (basically enabling a game or video display mode).
* **SingleAppWindow** - if set, it enable single app mode. This mode will not allow one to add multiple windows to the desktop or to use ``Application::Run()`` API. Instead, a class derived from `SingleApp` must be derived and AppCUI execution has to be started using ``Application::RunSingleApp(...)`` API.

.. code-block:: ini

   [AppCUI]
   Frontend = default      ; possible values: default,SDL, terminal, windows
   Size = default          ; possible values: a size (width x height), maximized, fullscreen
   CharacterSize = default ; possible values: default, tiny, small, normal, large, huge
   Fixed = false           ; possible values: true or false


InitializationData structure
----------------------------

**InitializationData** represent a struct that described all parameters needed to initialized AppCUI framework

.. code-block:: c++

   struct InitializationData
   {
      unsigned int                  Width, Height;
      FrontendType                  Frontend;
      CharacterSize                 CharSize;
      InitializationFlags           Flags;
      std::string_view              FontName;
      AppCUI::Controls::Desktop*    CustomDesktop;
   }

width **FrontendType** defined as follows:

.. code-block:: c++

   enum class FrontendType: unsigned int
   {
      Default        = 0,
      SDL            = 1,
      Terminal       = 2,
      WindowsConsole = 3
   };

and **CharacterSize** defined as:

.. code-block:: c++

   enum class CharacterSize: unsigned int
   {
      Default = 0,
      Tiny,
      Small,
      Normal,
      Large,
      Huge
   };

Examples
--------

1. Quick intialization
   
.. code-block:: c++

   if (AppCUI::Application::Init()==false) {
      // Appcui failed to initialize
   }

2. Initialize from **INI** file
   
.. code-block:: c++

   if (AppCUI::Application::Init(InitializationFlags::LoadSettingsFile)==false) {
      // Appcui failed to initialize
   }

3. Initialize from **INI** file but also specify that a Menu and Command bar will be used
   
.. code-block:: c++

   if (AppCUI::Application::Init(InitializationFlags::LoadSettingsFile | 
                                 InitializationFlags::Menu | 
                                 InitializationFlags::CommandBar) == false) {
      // Appcui failed to initialize
   }

4. Full customized initialization (an SDL based application , with the size of ``120x30`` characters, using **Consolas** font (small size) and with a Menu and CommandBar.

.. code-block:: c++

   InitializationData initData;
   initData.Width          = 120;
   initData.Height         = 30;
   initData.FrontendType   = Frontend::SDL;
   initData.CharSize       = CharacterSize::Small;
   initData.FontName       = "Consolas";
   initData.Flags          = InitializationFlags::Menu | InitializationFlags::CommandBar;

   if (AppCUI::Application::Init(initData) == false) {
      // Appcui failed to initialize
   } 

For more example check out our examples code:

* `Initialization via INI file <https://github.com/gdt050579/AppCUI/tree/main/Examples/IniInitialization>`_
* `Custom desktop <https://github.com/gdt050579/AppCUI/tree/main/Examples/CustomDesktop>`_
* `Terminal settings <https://github.com/gdt050579/AppCUI/tree/main/Examples/TerminalSettings>`_