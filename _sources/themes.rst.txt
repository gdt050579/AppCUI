Themes
======

A theme gives a color to every element AppCUI draws (window backgrounds, borders, buttons, menus, list cursors,
selections, scroll bars, ...). The colors of the current theme are in ``Application::GetAppConfig()``.

Built-in themes
---------------

The built-in themes are defined in code: they need no file and work on every frontend, the web browser included.

.. list-table::
   :header-rows: 1
   :widths: 20 20 60

   * - ``ThemeType``
     - name
     - description
   * - ``Default``
     - ``Default``
     - dark blue windows, light buttons and menus
   * - ``Dark``
     - ``Dark``
     - black windows
   * - ``Light``
     - ``Light``
     - light windows, dark text
   * - ``HighContrast``
     - ``HighContrast``
     - accessibility: bright text on black, every highlight (focus, hover, selection, cursor) is a solid background,
       the focused window border is yellow
   * - ``Midnight``
     - ``Midnight``
     - black windows, dark blue panels and menus, blue / aqua accents
   * - ``Ocean``
     - ``Ocean``
     - dark blue windows with aqua text and panels, teal desktop and scroll bars
   * - ``Forest``
     - ``Forest``
     - black windows with olive frames, green / olive panels
   * - ``RetroGreen``
     - ``RetroGreen``
     - monochrome green terminal: green on black, highlights in inverse video

New themes are only appended to ``ThemeType``, so the value of an existing theme never changes. The colors of the
built-in themes were checked against the 16-color palettes of the frontends (the Windows console, SDL and the browser
use the Campbell palette, ncurses the xterm one): two roles shown next to each other never share a color pair.

Switching themes
----------------

.. code-block:: c++

   Application::SetTheme(Application::ThemeType::Ocean);   // a built-in theme
   Application::SetTheme("Ocean");                          // the same, by name
   Application::SetTheme("MyColors");                       // MyColors.theme (or a settings section, see below)
   auto name = Application::GetCurrentThemeName();          // "Ocean", "MyColors", ...

Every switch notifies the listeners registered with ``Dialogs::ThemeEditor::RegisterOnThemeChangeCallback``. The colors
registered with ``Dialogs::ThemeEditor::RegisterCustomColors`` return to their registered values first (the built-in
themes do not define custom colors); a ``.theme`` file may then change them.

The theme registry
------------------

The registry maps theme names to themes; it is used by the settings file, the ``Theme`` property of
``Application::GetAppPropertiesObject()``, the theme editor and the theme picker.

.. code-block:: c++

   std::string_view GetThemeName(ThemeType type);          // "HighContrast"
   std::string_view GetThemeDisplayName(ThemeType type);   // "High Contrast"
   std::optional<ThemeType> GetThemeType(std::string_view name);
   bool IsBuiltInTheme(std::string_view name);
   std::vector<ThemeInfo> EnumerateThemes();

``GetThemeType`` ignores the case, spaces, ``-`` and ``_``: ``"High Contrast"``, ``"high-contrast"`` and
``"HIGHCONTRAST"`` all name ``ThemeType::HighContrast``. ``EnumerateThemes`` lists every built-in theme (in
``ThemeType`` order), then the themes stored in the settings file, then the ``*.theme`` files of the themes folder
(``ThemeInfo::Source`` tells them apart; a name is listed once, the way ``SetTheme`` resolves it).

A ``<Name>.theme`` file named after a built-in theme (for example ``Dark.theme``) customizes that theme: it is applied
over the built-in colors whenever the built-in theme is selected.

Theme names that are not built-in
---------------------------------

A name that is not built-in (in ``[AppCUI] Theme=``, ``InitializationData::ThemeName`` or ``Application::SetTheme``)
is looked up, in this order:

1. in an ``[AppCUI.Theme.<name>]`` section of the settings file;
2. in ``<name>.theme`` from the themes folder (``[AppCUI] ThemeFolder``, default ``Themes`` next to the executable).

The theme is applied over the ``Default`` theme (over ``InitializationData::Theme`` at start-up), so a theme that sets
only a few colors is still complete. If it can not be found, the current theme stays.

A ``.theme`` file is an ini file (the theme editor writes it with **Save...**):

.. code-block:: ini

   [Text]
   Normal = Silver,Transparent
   HotKey = Aqua,Transparent

   [Window]
   Normal = DarkBlue

An ``[AppCUI.Theme.<name>]`` settings section holds the same keys, prefixed with the name of their section:

.. code-block:: ini

   [AppCUI.Theme.Custom]
   Text.Normal = Silver,Transparent
   Text.HotKey = Aqua,Transparent
   Window.Normal = DarkBlue

The theme picker
----------------

``Dialogs::ThemePicker::Show()`` lists the themes of ``EnumerateThemes``. Moving through the list previews the theme
on the whole application; **Enter** / **Apply** keeps it, **Esc** / **Cancel** restores the theme that was active when
the dialog opened. **Edit...** opens the theme editor on the selected theme. ``Show`` returns ``true`` if a theme was
applied.

.. code-block:: c++

   if (Dialogs::ThemePicker::Show())
       LOG_INFO("New theme: %s", std::string(Application::GetCurrentThemeName()).c_str());

The theme editor
----------------

``Dialogs::ThemeEditor::Show()`` edits every color of the current theme. **Theme mode** starts from any theme of the
registry; **Save...** / **Load...** write and read ``.theme`` files; **Apply** applies the colors.

Keeping a theme after a restart
-------------------------------

When the application uses a settings file (``InitializationFlags::LoadSettingsFile``), **Apply** in the theme picker
and in the theme editor asks *"Keep this theme after a restart? (saved to the .ini)"*:

* **No**: the theme lasts for this session only.
* **Yes**, a built-in theme or a theme file that was not edited: ``[AppCUI] Theme=<name>`` is written to the settings
  file.
* **Yes**, colors edited in the theme editor: they are saved as ``Custom.theme`` in the themes folder (the folder is
  created if needed; an existing ``Custom.theme`` is only overwritten after a confirmation) and ``Theme=Custom`` is
  written.

In the web browser (WebAssembly), files written to the themes folder do not survive a page reload, but the settings file
does: the host page keeps it in ``localStorage`` (see the ``settingsStorage`` option of ``startAppCUI``), one entry per
page, so two AppCUI applications of the same site do not share their settings. Every key of the settings file persists
this way, not only the theme. Edited colors are therefore stored in an ``[AppCUI.Theme.Custom]`` section of the settings
file instead of ``Custom.theme``.

Without a settings file, the question is not asked and the theme lasts for this session only. An application can also
write the setting itself:

.. code-block:: c++

   Application::SaveThemeToSettings("Midnight");   // [AppCUI] Theme=Midnight, saved to <executable>.ini

Only the theme keys change on disk: the settings file is read again and saved with the new value (the in-memory
settings, ``GetAppSettings()``, receive the same value but are not written, so unsaved changes of other modules stay
in memory). ``Application::UpdateAppCUISettings(ini, true)`` (reset to the defaults) also removes the
``[AppCUI.Theme.<Name>]`` sections.

Custom colors
-------------

Applications add their own color categories with ``Dialogs::ThemeEditor::RegisterCustomColors``; the theme editor
shows them and ``.theme`` files save them (``[CustomColors.<category>]`` sections). A category name can not contain
spaces or ``.``. Implement
``Dialogs::OnThemeChangedInterface`` and register it with ``Dialogs::ThemeEditor::RegisterOnThemeChangeCallback`` to
read the new values (``config.CustomColors``) after every theme switch: preview in the picker, Apply and Cancel.

A theme that saves only some colors of a category (for example a ``.theme`` written before a color was registered)
changes only those: the other colors of the category keep their values, so a listener can always read every name it
registered.
