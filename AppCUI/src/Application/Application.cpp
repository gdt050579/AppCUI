#include "../Terminal/TerminalFactory.hpp"
#include "../Terminal/TestTerminal/TestTerminal.hpp"

#include <math.h>

namespace AppCUI
{
using namespace Internal;

ApplicationImpl* app = nullptr;

bool Application::Init(Application::InitializationFlags flags)
{
    Application::InitializationData initData;
    initData.Flags = flags;
    return Application::Init(initData);
}
bool Application::InitForTests(uint32 width, uint32 height, Application::InitializationFlags flags, bool asciiMode)
{
    Application::InitializationData initData;
    initData.Flags               = flags;
    initData.Width               = width;
    initData.Height              = height;
    initData.Frontend            = AppCUI::Application::FrontendType::Tests;
    initData.SpecialCharacterSet = asciiMode ? AppCUI::Application::SpecialCharacterSetType::Ascii
                                             : AppCUI::Application::SpecialCharacterSetType::Unicode;
    return Application::Init(initData);
}
bool Application::Init(InitializationData& initData)
{
    CHECK(app == nullptr, false, "Application has already been initialized !");
    app = new ApplicationImpl();
    CHECK(app, false, "Fail to allocate space for application object !");
    if (app->Init(initData))
        return true;
    delete app;
    app = nullptr;
    RETURNERROR(false, "Fail to initialized application !");
}
bool Application::RunTestScript(std::string_view testScript)
{
    CHECK(app, false, "Application has not been initialized !");
    CHECK(app->Inited, false, "Application has not been corectly initialized !");
    auto testTerm = dynamic_cast<TestTerminal*>(app->terminal.get());
    CHECK(testTerm, false, "`RunTestScript` can only work with a TestTerminal frontend !");
    bool scriptResult = true;
    testTerm->CreateEventsQueue(testScript, &scriptResult);
    CHECK(Run(), false, "Execution error (Run method failed)");
    return scriptResult;
}
bool Application::Run()
{
    CHECK(app, false, "Application has not been initialized !");
    CHECK(app->Inited, false, "Application has not been corectly initialized !");
    app->loopStatus = LoopStatus::Normal;
    app->ExecuteEventLoop();
    LOG_INFO("Uninit text field/area default menu ");
    Controls::UninitTextFieldDefaultMenu();
    Controls::UninitTextAreaDefaultMenu();
    LOG_INFO("Starting to un-init AppCUI ...");
    app->UnInit();
    Log::Unit();
    LOG_INFO("Uninit successfully");
    delete app;
    app = nullptr;
    return true;
}
bool Application::RunSingleApp(unique_ptr<Controls::SingleApp> singleApp)
{
    CHECK(app, false, "Application has not been initialized !");
    CHECK(app->Inited, false, "Application has not been corectly initialized !");
    CHECK((app->InitFlags & InitializationFlags::SingleWindowApp) != InitializationFlags::None,
          false,
          "Flag `InitializationFlags::SingleWindowApp` was not set when initializing AppCUI !");
    CHECK(singleApp, false, "Expecting a valid (non-null) single App");
    auto Members    = reinterpret_cast<ControlContext*>(app->AppDesktop->Context);
    auto top        = Members->Margins.Top;
    auto bottom     = Members->Margins.Bottom;
    app->AppDesktop = singleApp.release();
    app->loopStatus = LoopStatus::Normal;

    CHECK(app->AppDesktop->Resize(app->terminal->screenCanvas.GetWidth(), app->terminal->screenCanvas.GetHeight()),
          false,
          "");
    ((ControlContext*) (app->AppDesktop->Context))->Margins.Top    = top;
    ((ControlContext*) (app->AppDesktop->Context))->Margins.Bottom = bottom;

    // all good - start the loop
    app->ExecuteEventLoop();
    LOG_INFO("Starting to un-init AppCUI ...");
    app->UnInit();
    Log::Unit();
    LOG_INFO("Uninit successfully");
    delete app;
    app = nullptr;
    return true;
}
bool Application::GetApplicationSize(Graphics::Size& size)
{
    CHECK(app, false, "Application has not been initialized !");
    size.Width  = app->terminal->screenCanvas.GetWidth();
    size.Height = app->terminal->screenCanvas.GetHeight();
    return true;
}
bool Application::GetDesktopSize(Graphics::Size& size)
{
    CHECK(app, false, "Application has not been initialized !");
    size = app->AppDesktop->GetClientSize();
    return true;
}

void Application::ArrangeWindows(ArrangeWindowsMethod method)
{
    if (app)
        app->ArrangeWindows(method);
}
void Application::Close()
{
    if (app)
        app->Terminate();
}
ItemHandle Application::AddWindow(unique_ptr<Window> wnd, ItemHandle referal)
{
    CHECK(app, InvalidItemHandle, "Application has not been initialized !");
    CHECK(app->Inited, InvalidItemHandle, "Application has not been corectly initialized !");
    CHECK((app->InitFlags & InitializationFlags::SingleWindowApp) == InitializationFlags::None,
          InvalidItemHandle,
          "This is a single app window (no desktop windows can be added !");
    CHECK(wnd, InvalidItemHandle, "Null pointer for Window object");
    auto resultHandle     = ItemHandle{ app->LastWindowID };
    const auto winMembers = reinterpret_cast<WindowControlContext*>(wnd->Context);
    CHECK(winMembers, InvalidItemHandle, "Invalid members !");
    winMembers->windowItemHandle  = resultHandle;
    winMembers->referalItemHandle = referal;
    app->LastWindowID             = (app->LastWindowID + 1) % 0x7FFFFFFF;
    if (((app->InitFlags & InitializationFlags::AutoHotKeyForWindow) != InitializationFlags::None) &&
        (wnd->GetHotKey() == Key::None))
    {
        // compute a possible hot key
        bool v[10] = {
            false, false, false, false, false, false, false, false, false, false,
        };
        // iterate from all top level windows and see if a hot-key is being associated
        auto winList          = app->AppDesktop->GetChildrenList();
        const auto endWinList = winList + app->AppDesktop->GetChildrenCount();
        if (winList)
        {
            for (; winList < endWinList; winList++)
            {
                if ((!(*winList)) || (!((*winList)->Context)))
                    continue;
                auto h_key = (*winList)->GetHotKey();
                if ((h_key & Key::Alt) != Key::None)
                {
                    h_key = h_key & ~((uint32) Key::Alt);
                    // Alt+0 will not be considered
                    if ((((uint32) h_key) > ((uint32) Key::N0)) && (((uint32) h_key) <= ((uint32) Key::N9)))
                    {
                        v[((uint32) h_key) - ((uint32) Key::N0)] = true;
                    }
                }
            }
        }
        // find first visible ID
        for (uint32 tr = 1; tr < 10; tr++)
            if (!v[tr])
            {
                wnd->SetHotKey('0' + tr);
                break;
            }
    }
    auto ptrWin = wnd.get();
    CHECK(app->AppDesktop->AddControl<Controls::Window>(std::move(wnd)).IsValid(),
          InvalidItemHandle,
          "Fail to add window to desktop !");
    ptrWin->SetFocus();
    return resultHandle;
}
ItemHandle Application::AddWindow(unique_ptr<Window> wnd, Window* referalWindow)
{
    if (!referalWindow)
        return Application::AddWindow(std::move(wnd), InvalidItemHandle);
    const auto winMembers = reinterpret_cast<WindowControlContext*>(referalWindow->Context);
    if (!winMembers)
        return Application::AddWindow(std::move(wnd), InvalidItemHandle);
    return Application::AddWindow(std::move(wnd), winMembers->windowItemHandle);
}
Controls::Menu* Application::AddMenu(const ConstString& name)
{
    CHECK(app, nullptr, "Application has not been initialized !");
    CHECK(app->Inited, nullptr, "Application has not been corectly initialized !");
    CHECK(app->menu, nullptr, "Application was not initialized with HAS_MENU option set up !");
    ItemHandle itm         = app->menu->AddMenu(name);
    Controls::Menu* result = app->menu->GetMenu(itm);
    CHECK(result, nullptr, "Fail to create menu !");
    return result;
}

Application::Config* Application::GetAppConfig()
{
    CHECK(app, nullptr, "Application has not been initialized !");
    return &app->config;
}
void Application::SetTheme(ThemeType themeType)
{
    if (app)
    {
        Internal::Config::SetTheme(app->config, themeType);
    }
}
Utils::IniObject* Application::GetAppSettings()
{
    CHECK(app, nullptr, "Application has not been initialized !");
    return &app->settings;
}
bool Application::SaveAppSettings()
{
    CHECK(app, false, "Application has not been initialized !");
    auto appPath = Application::GetAppSettingsFile();
    CHECK(!appPath.empty(), false, "OS::GetCurrentApplicationPath failed !");
    CHECK(app->settings.Save(appPath), false, "Fail to save ini setting to file: %s", appPath.string().c_str());
    return true;
}
std::filesystem::path Application::GetAppSettingsFile()
{
    auto appPath = OS::GetCurrentApplicationPath();
    CHECK(!appPath.empty(), appPath, "OS::GetCurrentApplicationPath failed !");
    appPath.replace_extension(".ini");
    return appPath;
}
void Application::UpdateAppCUISettings(Utils::IniObject& ini, bool clearExistingSettings)
{
    auto sect = ini["AppCUI"];
    if (clearExistingSettings)
        sect.Clear();
    sect.UpdateValue("Frontend", "default", true);
    sect.UpdateValue("Size", "default", true);
    sect.UpdateValue("CharacterSize", "default", true);
    sect.UpdateValue("Fixed", "default", true);
    sect.UpdateValue("Theme", "default", true);
    sect.UpdateValue("CharacterSet", "auto", true);
}
bool Application::UpdateAppCUISettings(bool clearExistingSettings)
{
    CHECK(app, false, "Application has not been initialized !");
    UpdateAppCUISettings(app->settings, clearExistingSettings);
    return true;
}
void Application::RaiseEvent(
      Utils::Reference<Controls::Control> control,
      Utils::Reference<Controls::Control> sourceControl,
      Controls::Event eventType,
      int controlID)
{
    app->RaiseEvent(control, sourceControl, eventType, controlID);
}
Utils::Reference<Controls::Desktop> Application::GetDesktop()
{
    return app->AppDesktop;
}
ApplicationImpl* Application::GetApplication()
{
    return app;
}
Controls::Control* GetFocusedControl(Controls::Control* ctrl);
void UpdateCommandBar()
{
    if (!app->cmdBar)
        return;
    app->cmdBar->Clear();
    Controls::Control* obj;
    if (app->ModalControlsCount == 0)
        obj = GetFocusedControl(app->AppDesktop);
    else
        obj = GetFocusedControl(app->ModalControlsStack[app->ModalControlsCount - 1]);
    while (obj != nullptr)
    {
        // on handler
        if (obj->OnUpdateCommandBar(app->CommandBarWrapper) == true)
            break;

        obj = ((ControlContext*) (obj->Context))->Parent;
    }
    // restore hover if case
    bool repaint;
    app->cmdBar->OnMouseMove(app->LastMouseX, app->LastMouseY, repaint);
    app->cmdBarUpdate = false;
}
void PaintControl(Controls::Control* ctrl, Graphics::Renderer& renderer, bool focused)
{
    CHECKRET(ctrl != nullptr, "");
    CREATE_CONTROL_CONTEXT(ctrl, Members, );
    if (((Members->Flags & GATTR_VISIBLE) == 0) || (!Members->ScreenClip.Visible))
        return;

    // check if started
    if (!Members->Started)
    {
        Members->Started = true;
        if ((Members->handlers) && (Members->handlers->OnStart.obj))
            Members->handlers->OnStart.obj->OnStart(ctrl);
        else
            ctrl->OnStart();
    }

    // set clip
    app->terminal->screenCanvas.SetAbsoluteClip(Members->ScreenClip);
    app->terminal->screenCanvas.SetTranslate(
          Members->ScreenClip.ScreenPosition.X, Members->ScreenClip.ScreenPosition.Y);

    if (focused != Members->Focused)
    {
        Members->Focused = focused;
        if (focused)
        {
            // on focus (handler)
            if ((Members->handlers) && (Members->handlers->OnFocus.obj))
                Members->handlers->OnFocus.obj->OnFocus(ctrl);
            else
                ctrl->OnFocus();
            app->cmdBarUpdate = true;
        }
        else
        {
            // on lose focus
            if ((Members->handlers) && (Members->handlers->OnLoseFocus.obj))
                Members->handlers->OnLoseFocus.obj->OnLoseFocus(ctrl);
            else
                ctrl->OnLoseFocus();
            if (ctrl == app->ExpandedControl)
                app->PackControl(false);
        }
    }
    // put the other clip
    if (ctrl == app->ExpandedControl)
    {
        app->terminal->screenCanvas.SetAbsoluteClip(Members->ExpandedViewClip);
        app->terminal->screenCanvas.SetTranslate(
              Members->ExpandedViewClip.ScreenPosition.X, Members->ExpandedViewClip.ScreenPosition.Y);
    }

    // draw current control
    if (Members->handlers)
    {
        if (Members->handlers->PaintControl.obj)
        {
            Members->handlers->PaintControl.obj->PaintControl(ctrl, renderer);
        }
        else
        {
            ctrl->Paint(renderer);
        }
    }
    else
    {
        ctrl->Paint(renderer);
    }

    if ((Members->Focused) && (Members->Flags & (GATTR_VSCROLL | GATTR_HSCROLL)))
    {
        renderer.ResetClip(); // make sure that the entire surface is available
        if (Members->ScrollBars.OutsideControl)
            app->terminal->screenCanvas.ExtendAbsoluteClipToRightBottomCorner();
        ctrl->OnUpdateScrollBars(); // update scroll bars value
        Members->PaintScrollbars(renderer);
    }

#if defined(APPCUI_ENABLE_CONTROL_BORDER_MODE)
    // draw border before checking any selection below
    renderer.ResetClip(); // make sure that the entire surface is available
    app->terminal->ScreenCanvas.ExtendAbsoluteClipInAllDirections(1);
    renderer.DrawRectSize(0, 0, ctrl->GetWidth(), ctrl->GetHeight(), { Color::White, Color::Transparent }, false);
#endif

    const auto cnt = Members->ControlsCount;
    const auto idx = Members->CurrentControlIndex;

    if (idx >= cnt)
    {
        // no selected control ==> draw all of them
        for (uint32 tr = 0; tr < cnt; tr++)
        {
            PaintControl(Members->Controls[tr], renderer, false);
        }
    }
    else
    {
        // one control is selected (paint controls that are not focused)
        for (uint32 tr = 1; tr < cnt; tr++)
        {
            PaintControl(Members->Controls[(tr + idx) % cnt], renderer, false);
        }

        // paint focused control
        PaintControl(Members->Controls[idx], renderer, focused);
    }
}
void PaintMenu(Controls::Menu* menu, Graphics::Renderer& renderer, bool activ)
{
    if (!menu)
        return;
    auto menuContext = reinterpret_cast<MenuContext*>(menu->Context);
    if (!menuContext)
        return;
    // go backwards and draw its parend first
    if (menuContext->Parent)
        PaintMenu(menuContext->Parent, renderer, false);
    // draw myself
    app->terminal->screenCanvas.SetAbsoluteClip(menuContext->ScreenClip);
    app->terminal->screenCanvas.SetTranslate(
          menuContext->ScreenClip.ScreenPosition.X, menuContext->ScreenClip.ScreenPosition.Y);
    menuContext->Paint(renderer, activ);
}
void ComputeControlLayout(Graphics::Clip& parentClip, Control* ctrl)
{
    if (ctrl == nullptr)
        return;
    CREATE_CONTROL_CONTEXT(ctrl, Members, );
    // compute the clip
    Members->ScreenClip.Set(
          parentClip, Members->Layout.X, Members->Layout.Y, Members->Layout.Width, Members->Layout.Height);
    // compute the expanded clip if neccesary
    if (ctrl == app->ExpandedControl)
    {
        if ((Members->Flags & GATTR_EXPANDED) == 0)
        {
            Members->ExpandedViewClip = Members->ScreenClip;
            ctrl->OnExpandView(Members->ExpandedViewClip);
            Members->Flags |= GATTR_EXPANDED;
        }
    }
    // calculez clip-ul client
    Graphics::Clip client;
    client.Set(
          parentClip,
          Members->Layout.X + Members->Margins.Left,
          Members->Layout.Y + Members->Margins.Top,
          Members->Layout.Width - (Members->Margins.Right + Members->Margins.Left),
          Members->Layout.Height - (Members->Margins.Bottom + Members->Margins.Top));
    // calculez pentru fiecare copil
    for (uint32 tr = 0; tr < Members->ControlsCount; tr++)
        ComputeControlLayout(client, Members->Controls[tr]);
}
void DestroyControl(Controls::Control* ctrl)
{
    if (!ctrl)
        return;
    auto Members = reinterpret_cast<ControlContext*>(ctrl->Context);
    if (Members)
    {
        for (uint32 tr = 0; tr < Members->ControlsCount; tr++)
        {
            DestroyControl(Members->Controls[tr]);
        }
        Members->ControlsCount = 0;
    }
    delete ctrl;
}
bool ProcessUpdateFrameEvent(Controls::Control* ctrl)
{
    CREATE_CONTROL_CONTEXT(ctrl, Members, false);
    if ((Members->Flags & (GATTR_VISIBLE | GATTR_ENABLE)) != (GATTR_VISIBLE | GATTR_ENABLE))
        return false; // no need to call the function
    bool res = ctrl->OnFrameUpdate();
    if (Members->ControlsCount > 0)
    {
        auto s = Members->Controls;
        auto e = s + Members->ControlsCount;
        while (s < e)
        {
            res |= ProcessUpdateFrameEvent(*s);
            s++;
        }
    }
    return res;
}
Controls::Control* RecursiveCoordinatesToControl(Controls::Control* ctrl, int x, int y)
{
    if (ctrl == nullptr)
        return nullptr;
    CREATE_CONTROL_CONTEXT(ctrl, Members, nullptr);
    if ((Members->Flags & (GATTR_ENABLE | GATTR_VISIBLE)) != (GATTR_ENABLE | GATTR_VISIBLE))
        return nullptr;
    if (Members->ScreenClip.Visible == false)
        return nullptr;
    if ((x < Members->ScreenClip.ClipRect.X) || (y < Members->ScreenClip.ClipRect.Y) ||
        (x >= (Members->ScreenClip.ClipRect.X + Members->ScreenClip.ClipRect.Width)) ||
        (y >= (Members->ScreenClip.ClipRect.Y + Members->ScreenClip.ClipRect.Height)))
        return nullptr;
    // controlul e ok - verific acuma pentru copii
    if (Members->ControlsCount > 0)
    {
        uint32 idx = Members->CurrentControlIndex;
        if (idx >= Members->ControlsCount)
            idx = 0;
        for (uint32 tr = 0; tr < Members->ControlsCount; tr++)
        {
            Control* res = RecursiveCoordinatesToControl(Members->Controls[idx], x, y);
            if (res != nullptr)
                return res;
            idx++;
            if (idx >= Members->ControlsCount)
                idx = 0;
        }
    }
    // daca nu e ok pe nici un copil sau nu am copii - atunci ma returnez pe mine
    return ctrl;
}

Controls::Control* CoordinatesToControl(Controls::Control* ctrl, int x, int y)
{
    if (app->ExpandedControl)
    {
        CREATE_CONTROL_CONTEXT(app->ExpandedControl, Members, nullptr);
        if ((x >= Members->ExpandedViewClip.ClipRect.X) && (y >= Members->ExpandedViewClip.ClipRect.Y) &&
            (x < (Members->ExpandedViewClip.ClipRect.X + Members->ExpandedViewClip.ClipRect.Width)) &&
            (y < (Members->ExpandedViewClip.ClipRect.Y + Members->ExpandedViewClip.ClipRect.Height)))
            return app->ExpandedControl;
    }
    return RecursiveCoordinatesToControl(ctrl, x, y);
}

Controls::Control* GetFocusedControl(Controls::Control* ctrl)
{
    if (ctrl == nullptr)
        return nullptr;
    CREATE_CONTROL_CONTEXT(ctrl, Members, nullptr);
    if ((Members->Flags & (GATTR_ENABLE | GATTR_VISIBLE)) != (GATTR_ENABLE | GATTR_VISIBLE))
        return nullptr;
    // altfel ma uit la copii lui
    if (Members->ControlsCount > 0)
    {
        if (Members->CurrentControlIndex < Members->ControlsCount)
        {
            Control* c = GetFocusedControl(Members->Controls[Members->CurrentControlIndex]);
            if (c != nullptr)
                return c;
        }
    }
    // altfel nici un copil nu e ok - cer eu
    return ctrl;
}

ApplicationImpl::ApplicationImpl()
{
    this->terminal           = nullptr;
    this->Inited             = false;
    this->cmdBarUpdate       = true;
    this->MouseLockedControl = nullptr;
    this->MouseOverControl   = nullptr;
    this->ExpandedControl    = nullptr;
    this->VisibleMenu        = nullptr;
    this->ModalControlsCount = 0;
    this->LastWindowID       = 0;
    this->loopStatus         = LoopStatus::Normal;
    this->RepaintStatus      = REPAINT_STATUS_ALL;
    this->mouseLockedObject  = MouseLockedObject::None;
    this->InitFlags          = Application::InitializationFlags::None;
    this->LastMouseX         = -1;
    this->LastMouseY         = -1;
}
ApplicationImpl::~ApplicationImpl()
{
    this->Destroy();
}
Application::FrontendType ApplicationImpl::GetFrontendType() const
{
    return app->frontend;
}
void ApplicationImpl::Destroy()
{
    this->Inited             = false;
    this->MouseLockedControl = nullptr;
    this->MouseOverControl   = nullptr;
    this->VisibleMenu        = nullptr;
    this->ModalControlsCount = 0;
    this->loopStatus         = LoopStatus::Normal;
    this->RepaintStatus      = REPAINT_STATUS_ALL;
    this->mouseLockedObject  = MouseLockedObject::None;
}
bool ApplicationImpl::LoadThemeFile(Application::InitializationData& initData)
{
    auto appPath = OS::GetCurrentApplicationPath();
    if (appPath.empty())
    {
        LOG_WARNING("OS::GetCurrentApplicationPath failed (without current application path, path to theme file can "
                    "not be found");
        return false;
    }
    appPath = appPath.remove_filename();
    appPath += (string_view) initData.ThemeName;
    appPath.replace_extension(".theme");
    if (Internal::Config::Load(this->config, appPath) == false)
    {
        LOG_WARNING("Fail to load theme file from: %s --> reverting to defaul theme", appPath.string().c_str());
        return false;
    }
    return true;
}
void ApplicationImpl::LoadSettingsFile(Application::InitializationData& initData)
{
    auto appPath = OS::GetCurrentApplicationPath();
    if (appPath.empty())
    {
        LOG_WARNING("OS::GetCurrentApplicationPath failed (without current application path, path to settings file can "
                    "not be found");
        return;
    }
    appPath.replace_extension(".ini");
    LOG_INFO("Initializing AppCUI using: %s", appPath.string().c_str());

    if (this->settings.CreateFromFile(appPath) == false)
    {
        LOG_WARNING("Fail to load ini file: %s ==> using default configuration", appPath.string().c_str());
        return;
    }
    // ini file is created --> let's load the section
    auto AppCUISection = this->settings.GetSection("appcui");
    if (AppCUISection.Exists() == false)
    {
        LOG_WARNING(
              "Section [AppCUI] was not found in %s ==> using the default configuration", appPath.string().c_str());
        return;
    }
    // we have the section ==> lets build up some parameters
    auto frontend     = AppCUISection.GetValue("frontend").ToString();
    auto terminalSize = AppCUISection.GetValue("size");
    auto charSize     = AppCUISection.GetValue("charactersize").ToString();
    bool fixedWindows = AppCUISection.GetValue("fixed").ToBool(false);
    auto themeName    = AppCUISection.GetValue("theme").ToString();
    auto charSet      = AppCUISection.GetValue("characterSet").ToString();

    // frontend
    if (frontend)
    {
        if (String::Equals(frontend, "default", true))
            initData.Frontend = Application::FrontendType::Default;
        else if (String::Equals(frontend, "SDL", true))
            initData.Frontend = Application::FrontendType::SDL;
        else if (String::Equals(frontend, "terminal", true))
            initData.Frontend = Application::FrontendType::Terminal;
        else if (String::Equals(frontend, "windows", true))
            initData.Frontend = Application::FrontendType::WindowsConsole;
    }

    // character size
    if (charSize)
    {
        if (String::Equals(charSize, "default", true))
            initData.CharSize = Application::CharacterSize::Default;
        else if (String::Equals(charSize, "tiny", true))
            initData.CharSize = Application::CharacterSize::Tiny;
        else if (String::Equals(charSize, "small", true))
            initData.CharSize = Application::CharacterSize::Small;
        else if (String::Equals(charSize, "normal", true))
            initData.CharSize = Application::CharacterSize::Normal;
        else if (String::Equals(charSize, "large", true))
            initData.CharSize = Application::CharacterSize::Large;
        else if (String::Equals(charSize, "huge", true))
            initData.CharSize = Application::CharacterSize::Huge;
    }

    // terminal size
    const char* s_terminalSize = terminalSize.ToString();
    if (s_terminalSize)
    {
        if (String::Equals(s_terminalSize, "fullscreen", true))
            initData.Flags |= Application::InitializationFlags::Fullscreen;
        else if (String::Equals(s_terminalSize, "maximized", true))
            initData.Flags |= Application::InitializationFlags::Maximized;
        else
        {
            auto termSize = terminalSize.AsSize();
            if (termSize.has_value())
            {
                initData.Width  = termSize->Width;
                initData.Height = termSize->Height;
            }
        }
    }

    // fixed size
    if (fixedWindows)
        initData.Flags |= Application::InitializationFlags::FixedSize;

    // themes
    if (themeName)
    {
        if (String::Equals(themeName, "default", true))
            initData.Theme = Application::ThemeType::Default;
        else if (String::Equals(themeName, "dark", true))
            initData.Theme = Application::ThemeType::Dark;
        else if (String::Equals(themeName, "light", true))
            initData.Theme = Application::ThemeType::Light;
        else
        {
            initData.Theme     = Application::ThemeType::Default;
            initData.ThemeName = themeName;
        }
    }

    // character set
    if (charSet)
    {
        if (String::Equals(charSet, "unicode", true))
            initData.SpecialCharacterSet = Application::SpecialCharacterSetType::Unicode;
        if (String::Equals(charSet, "ascii", true))
            initData.SpecialCharacterSet = Application::SpecialCharacterSetType::Ascii;
        else if (String::StartsWith(charSet, "linux", true))
            initData.SpecialCharacterSet = Application::SpecialCharacterSetType::LinuxTerminal;
        else
            initData.SpecialCharacterSet = Application::SpecialCharacterSetType::Auto;
    }

    // all good
}
bool ApplicationImpl::Init(Application::InitializationData& initData)
{
    CHECK(!Inited, false, "Application has already been initialized !");

    if ((initData.Flags & Application::InitializationFlags::LoadSettingsFile) != Application::InitializationFlags::None)
    {
        LoadSettingsFile(initData);
    }

    LOG_INFO("Starting AppCUI ...");
    LOG_INFO("Flags           = %08X", (uint32) initData.Flags);
    LOG_INFO("Requested Size  = %d x %d", initData.Width, initData.Height);

    // create the frontend
    frontend = initData.Frontend;
    CHECK((terminal = GetTerminal(initData)), false, "Fail to allocate a terminal object !");
    LOG_INFO("Terminal size: %d x %d", terminal->screenCanvas.GetWidth(), terminal->screenCanvas.GetHeight());

    // configur other objects and settings
    if ((initData.Flags & Application::InitializationFlags::CommandBar) != Application::InitializationFlags::None)
    {
        cmdBar = std::make_unique<Internal::CommandBarController>(
              terminal->screenCanvas.GetWidth(), terminal->screenCanvas.GetHeight(), &config);
        CommandBarWrapper.Init(cmdBar.get());
    }
    // configure menu
    if ((initData.Flags & Application::InitializationFlags::Menu) != Application::InitializationFlags::None)
    {
        menu = std::make_unique<Internal::MenuBar>();
        menu->SetWidth(terminal->screenCanvas.GetWidth());
    }

    // configure theme
    if (!initData.ThemeName.Empty())
    {
        if (!LoadThemeFile(initData))
        {
            Internal::Config::SetTheme(config, initData.Theme);
        }
    }
    else
    {
        Internal::Config::SetTheme(config, initData.Theme);
    }

    if (initData.CustomDesktopConstructor)
        AppDesktop = initData.CustomDesktopConstructor();
    else
        AppDesktop = Controls::Factory::Desktop::Create().release();

    CHECK(this->AppDesktop->Resize(terminal->screenCanvas.GetWidth(), terminal->screenCanvas.GetHeight()), false, "");
    if ((initData.Flags & Application::InitializationFlags::Menu) != Application::InitializationFlags::None)
        ((ControlContext*) (AppDesktop->Context))->Margins.Top = 1;
    if ((initData.Flags & Application::InitializationFlags::CommandBar) != Application::InitializationFlags::None)
        ((ControlContext*) (AppDesktop->Context))->Margins.Bottom = 1;

    // update special character set
    CHECK(Application::SetSpecialCharacterSet(initData.SpecialCharacterSet),
          false,
          "Fail to select a special character set for current application (value=%d) !",
          (int) initData.SpecialCharacterSet);

    loopStatus         = LoopStatus::Normal;
    RepaintStatus      = REPAINT_STATUS_ALL;
    mouseLockedObject  = MouseLockedObject::None;
    MouseLockedControl = nullptr;
    MouseOverControl   = nullptr;
    ModalControlsCount = 0;
    LastWindowID       = 0;
    InitFlags          = initData.Flags;

    Inited = true;
    LOG_INFO("AppCUI initialized succesifully");
    return true;
}
void ApplicationImpl::Paint()
{
    this->terminal->screenCanvas.Reset();
    // controalele

    if (ModalControlsCount > 0)
    {
        PaintControl(this->AppDesktop, this->terminal->screenCanvas, false);
        uint32 tmp = ModalControlsCount - 1;
        for (uint32 tr = 0; tr < tmp; tr++)
            PaintControl(ModalControlsStack[tr], this->terminal->screenCanvas, false);
        this->terminal->screenCanvas.DarkenScreen();
        PaintControl(ModalControlsStack[ModalControlsCount - 1], this->terminal->screenCanvas, true);
    }
    else
    {
        PaintControl(this->AppDesktop, this->terminal->screenCanvas, true);
    }

    // clip to the entire screen
    this->terminal->screenCanvas.ClearClip();
    this->terminal->screenCanvas.SetTranslate(0, 0);
    // draw command bar
    if (this->cmdBar)
        this->cmdBar->Paint(this->terminal->screenCanvas);
    // draw menu bar
    if (this->menu)
        this->menu->Paint(this->terminal->screenCanvas);
    // draw context menu
    if (this->VisibleMenu)
        PaintMenu(this->VisibleMenu, this->terminal->screenCanvas, true);
    // draw ToolTip if exists
    // ToolTip must be the last to be drawn (top-most)
    if (this->ToolTip.Visible)
    {
        this->terminal->screenCanvas.SetAbsoluteClip(this->ToolTip.ScreenClip);
        this->terminal->screenCanvas.SetTranslate(
              this->ToolTip.ScreenClip.ScreenPosition.X, this->ToolTip.ScreenClip.ScreenPosition.Y);
        this->ToolTip.Paint(this->terminal->screenCanvas);
    }
}
void ApplicationImpl::ComputePositions()
{
    Graphics::Clip full;
    full.Set(0, 0, app->terminal->screenCanvas.GetWidth(), app->terminal->screenCanvas.GetHeight());
    ComputeControlLayout(full, this->AppDesktop);
    for (uint32 tr = 0; tr < ModalControlsCount; tr++)
        ComputeControlLayout(full, ModalControlsStack[tr]);
}
void ApplicationImpl::ProcessKeyPress(Input::Key KeyCode, char16 unicodeCharacter)
{
    Reference<Control> ctrl = nullptr;

    // if a contextual menu is visible --> all keys will be handle by it
    if (this->VisibleMenu)
    {
        auto menuContext = reinterpret_cast<MenuContext*>(this->VisibleMenu->Context);
        if (menuContext->OnKeyEvent(KeyCode))
            RepaintStatus |= REPAINT_STATUS_DRAW;
        else if (menuContext->Owner)
        {
            if (menuContext->Owner->OnKeyEvent(KeyCode))
                RepaintStatus |= REPAINT_STATUS_DRAW;
        }
        return;
    }

    if (ModalControlsCount == 0)
        ctrl = GetFocusedControl(this->AppDesktop);
    else
        ctrl = GetFocusedControl(ModalControlsStack[ModalControlsCount - 1]);

    bool found  = false;
    bool result = false;
    while (ctrl != nullptr)
    {
        if (((ControlContext*) (ctrl->Context))->handlers)
        {
            const auto h = ((ControlContext*) (ctrl->Context))->handlers.get();
            if (h->OnKeyEvent.obj)
                result = h->OnKeyEvent.obj->OnKeyEvent(ctrl, KeyCode, unicodeCharacter);
            else
                result = ctrl->OnKeyEvent(KeyCode, unicodeCharacter);
        }
        else
        {
            result = ctrl->OnKeyEvent(KeyCode, unicodeCharacter);
        }
        if (result)
        {
            // if a key was handled --> repaint
            found = true;
            RepaintStatus |= REPAINT_STATUS_DRAW;
            break;
        }
        ctrl = ctrl->GetParent();
    }
    if (!found)
    {
        // check the menu bar
        if ((this->menu) && (this->menu->OnKeyEvent(KeyCode)))
            RepaintStatus |= REPAINT_STATUS_DRAW;
        else
        {
            // finally check command bar
            int cmd = -1;
            if (this->cmdBar)
                cmd = this->cmdBar->GetCommandForKey(KeyCode);
            if (cmd > 0)
                SendCommand(cmd);
        }
    }
}
void ApplicationImpl::ProcessMenuMouseClick(Controls::Menu* mnu, int x, int y)
{
    auto* mcx                 = reinterpret_cast<MenuContext*>(mnu->Context);
    MousePressedResult result = MousePressedResult::None;
    if (mnu == this->VisibleMenu)
    {
        result = mcx->OnMousePressed(x - mcx->ScreenClip.ScreenPosition.X, y - mcx->ScreenClip.ScreenPosition.Y);
    }
    else
    {
        // check and see if we should not change current visible menu
        if (mcx->IsOnMenu(x - mcx->ScreenClip.ScreenPosition.X, y - mcx->ScreenClip.ScreenPosition.Y))
            result = MousePressedResult::Activate;
        else
            result = MousePressedResult::CheckParent;
    }
    switch (result)
    {
    case MousePressedResult::None:
        break;
    case MousePressedResult::Repaint:
        RepaintStatus |= REPAINT_STATUS_DRAW;
        break;
    case MousePressedResult::CheckParent:
        if (mcx->Parent)
            ProcessMenuMouseClick(mcx->Parent, x, y);
        else
            this->CloseContextualMenu();
        break;
    case MousePressedResult::Activate:
        RepaintStatus |= REPAINT_STATUS_DRAW;
        ShowContextualMenu(mnu);
        break;
    }
}
void ApplicationImpl::ProcessMenuMouseReleased(Controls::Menu* mnu, int x, int y)
{
    auto* mcx   = reinterpret_cast<MenuContext*>(mnu->Context);
    bool result = mcx->OnMouseReleased(x - mcx->ScreenClip.ScreenPosition.X, y - mcx->ScreenClip.ScreenPosition.Y);
    if (result)
        RepaintStatus |= REPAINT_STATUS_DRAW;
}
bool ApplicationImpl::ProcessMenuAndCmdBarMouseMove(int x, int y)
{
    bool processed = false;
    bool repaint   = false;
    // Process event in the following order:
    // first the context menu and its owner, then the menu bar and then cmdbar
    if (this->VisibleMenu)
    {
        auto* mnuC = ((MenuContext*) (this->VisibleMenu->Context));
        processed =
              mnuC->OnMouseMove(x - mnuC->ScreenClip.ScreenPosition.X, y - mnuC->ScreenClip.ScreenPosition.Y, repaint);
        if ((!processed) && (mnuC->Owner))
            processed = mnuC->Owner->OnMouseMove(x, y, repaint);
    }
    // check menu bar
    if ((this->menu) && (!processed))
        processed = this->menu->OnMouseMove(x, y, repaint);

    // check command bar
    if ((this->cmdBar) && (!processed))
        processed = this->cmdBar->OnMouseMove(x, y, repaint);

    // analyze the data
    if (processed)
    {
        if (this->MouseOverControl)
        {
            if (this->MouseOverControl->OnMouseLeave())
                RepaintStatus |= REPAINT_STATUS_DRAW;
            ((ControlContext*) (MouseOverControl->Context))->MouseIsOver = false;
        }
        this->MouseOverControl = nullptr;
    }
    if (repaint)
        RepaintStatus |= REPAINT_STATUS_DRAW;

    return processed;
}
void ApplicationImpl::OnMouseDown(int x, int y, Input::MouseButton button)
{
    // Hide ToolTip
    this->ToolTip.Hide();
    // check contextual menus
    if (this->VisibleMenu)
    {
        ProcessMenuMouseClick(this->VisibleMenu, x, y);
        return;
    }
    if ((this->menu) && (this->menu->OnMousePressed(x, y, button)))
    {
        RepaintStatus |= REPAINT_STATUS_DRAW;
        return;
    }
    if ((this->cmdBar) && (this->cmdBar->OnMouseDown()))
    {
        RepaintStatus |= REPAINT_STATUS_DRAW;
        mouseLockedObject = MouseLockedObject::CommandBar;
        return;
    }
    // check controls
    if (ModalControlsCount == 0)
        MouseLockedControl = CoordinatesToControl(this->AppDesktop, x, y);
    else
        MouseLockedControl = CoordinatesToControl(ModalControlsStack[ModalControlsCount - 1], x, y);

    if (MouseLockedControl != nullptr)
    {
        if (this->ExpandedControl != this->MouseLockedControl)
            this->PackControl(true);
        MouseLockedControl->SetFocus();
        ControlContext* cc = ((ControlContext*) (MouseLockedControl->Context));

        MouseLockedControl->OnMousePressed(
              x - cc->ScreenClip.ScreenPosition.X, y - cc->ScreenClip.ScreenPosition.Y, button);

        // MouseLockedControl can be null afte OnMousePress if and Exit() call happens
        if (MouseLockedControl)
        {
            mouseLockedObject = MouseLockedObject::Control;
            RepaintStatus |= REPAINT_STATUS_DRAW;
        }
        return;
    }

    // else no object locked
    mouseLockedObject = MouseLockedObject::None;
}
void ApplicationImpl::OnMouseUp(int x, int y, Input::MouseButton button)
{
    int commandID;
    // check contextual menus
    if (this->VisibleMenu)
    {
        ProcessMenuMouseReleased(this->VisibleMenu, x, y);
    }
    switch (mouseLockedObject)
    {
    case MouseLockedObject::CommandBar:
        if (this->cmdBar)
        {
            if (this->cmdBar->OnMouseUp(commandID))
                RepaintStatus |= REPAINT_STATUS_DRAW;
            if (commandID > 0)
                SendCommand(commandID);
        }
        break;
    case MouseLockedObject::Control:
        ControlContext* cc = ((ControlContext*) (MouseLockedControl->Context));
        MouseLockedControl->OnMouseReleased(
              x - cc->ScreenClip.ScreenPosition.X, y - cc->ScreenClip.ScreenPosition.Y, button);
        RepaintStatus |= REPAINT_STATUS_DRAW;
        break;
    }
    MouseLockedControl = nullptr;
    mouseLockedObject  = MouseLockedObject::None;
}
void ApplicationImpl::OnMouseMove(int x, int y, Input::MouseButton button)
{
    Controls::Control* ctrl;
    LastMouseX = x;
    LastMouseY = y;
    switch (mouseLockedObject)
    {
    case MouseLockedObject::CommandBar:
        break;
    case MouseLockedObject::Control:
        this->ToolTip.Hide();
        if (MouseLockedControl->OnMouseDrag(
                  x - ((ControlContext*) (MouseLockedControl->Context))->ScreenClip.ScreenPosition.X,
                  y - ((ControlContext*) (MouseLockedControl->Context))->ScreenClip.ScreenPosition.Y,
                  button))
            RepaintStatus |= (REPAINT_STATUS_DRAW | REPAINT_STATUS_COMPUTE_POSITION);
        break;
    case MouseLockedObject::None:
        if (ProcessMenuAndCmdBarMouseMove(x, y))
            break;

        if (ModalControlsCount == 0)
            ctrl = CoordinatesToControl(this->AppDesktop, x, y);
        else
            ctrl = CoordinatesToControl(ModalControlsStack[ModalControlsCount - 1], x, y);
        if (ctrl != this->MouseOverControl)
        {
            this->ToolTip.Hide();
            if (this->MouseOverControl)
            {
                if (this->MouseOverControl->OnMouseLeave())
                    RepaintStatus |= REPAINT_STATUS_DRAW;
                ((ControlContext*) (MouseOverControl->Context))->MouseIsOver = false;
            }
            this->MouseOverControl = ctrl;
            if (this->MouseOverControl)
            {
                if (this->MouseOverControl->OnMouseEnter())
                    RepaintStatus |= REPAINT_STATUS_DRAW;
            }
            if (this->MouseOverControl)
            {
                // it is possible the OnMouseEnter might reset the MouseOverControl variable
                ControlContext* cc = ((ControlContext*) (MouseOverControl->Context));
                cc->MouseIsOver    = true;
                if (MouseOverControl->OnMouseOver(
                          x - cc->ScreenClip.ScreenPosition.X, y - cc->ScreenClip.ScreenPosition.Y))
                    RepaintStatus |= REPAINT_STATUS_DRAW;
            }
        }
        else
        {
            if (this->MouseOverControl)
            {
                ControlContext* cc = ((ControlContext*) (MouseOverControl->Context));
                if (MouseOverControl->OnMouseOver(
                          x - cc->ScreenClip.ScreenPosition.X, y - cc->ScreenClip.ScreenPosition.Y))
                    RepaintStatus |= REPAINT_STATUS_DRAW;
            }
        }
        break;
    }
}
void ApplicationImpl::OnMouseWheel(int x, int y, Input::MouseWheel direction)
{
    if (this->VisibleMenu)
    {
        auto* mcx = reinterpret_cast<MenuContext*>(this->VisibleMenu->Context);
        if (mcx->OnMouseWheel(x, y, direction))
            RepaintStatus |= REPAINT_STATUS_DRAW;
        return;
    }
    if (mouseLockedObject != MouseLockedObject::None)
        return;
    Controls::Control* ctrl;
    if (ModalControlsCount == 0)
        ctrl = CoordinatesToControl(this->AppDesktop, x, y);
    else
        ctrl = CoordinatesToControl(ModalControlsStack[ModalControlsCount - 1], x, y);
    if (ctrl)
    {
        ControlContext* cc = ((ControlContext*) (ctrl->Context));
        if (ctrl->OnMouseWheel(x - cc->ScreenClip.ScreenPosition.X, y - cc->ScreenClip.ScreenPosition.Y, direction))
            RepaintStatus |= REPAINT_STATUS_DRAW;
    }
}
void ApplicationImpl::PackControl(bool redraw)
{
    if (!this->ExpandedControl)
        return;
    CREATE_CONTROL_CONTEXT(this->ExpandedControl, Members, );
    this->ExpandedControl->OnPackView();
    Members->Flags -= (Members->Flags & GATTR_EXPANDED);
    this->ExpandedControl = nullptr;
    if (redraw)
        this->RepaintStatus = REPAINT_STATUS_ALL;
}
bool ApplicationImpl::ExpandControl(Controls::Control* ctrl)
{
    CHECK(ctrl, false, "Expecting a valid (non-null) control !");
    if (this->ExpandedControl != nullptr)
        PackControl(false);
    CREATE_CONTROL_CONTEXT(ctrl, Members, false);
    this->ExpandedControl = ctrl;
    // remove GATTR_EXPANDED flag if exists
    Members->Flags -= (Members->Flags & GATTR_EXPANDED);
    // compute current positions
    ComputePositions();
    this->ToolTip.Hide();
    this->RepaintStatus = REPAINT_STATUS_DRAW;
    return true;
}
void ApplicationImpl::CloseContextualMenu()
{
    if (this->VisibleMenu)
    {
        auto* mcx = reinterpret_cast<MenuContext*>(this->VisibleMenu->Context);
        if (mcx->Owner)
            mcx->Owner->Close();
    }
    this->ToolTip.Hide();
    this->VisibleMenu = nullptr;
    this->RepaintStatus |= REPAINT_STATUS_DRAW;
}
void ApplicationImpl::ShowContextualMenu(Controls::Menu* mnu)
{
    if (this->VisibleMenu != mnu)
    {
        this->VisibleMenu = mnu;
        this->RepaintStatus |= REPAINT_STATUS_ALL;
    }
}
void ApplicationImpl::ProcessShiftState(Input::Key ShiftState)
{
    if ((this->cmdBar) && (this->cmdBar->SetShiftKey(ShiftState)))
        RepaintStatus |= REPAINT_STATUS_DRAW;
}
void ApplicationImpl::CheckIfAppShouldClose()
{
    if ((ModalControlsCount == 0) &&
        ((this->InitFlags & Application::InitializationFlags::DisableAutoCloseDesktop) ==
         Application::InitializationFlags::None) &&
        (((ControlContext*) this->AppDesktop->Context)->ControlsCount == 0))
    {
        // no window on the desktop --> close the app
        loopStatus = LoopStatus::StopApp;
    }
}
bool ApplicationImpl::ExecuteEventLoop(Control* ctrl, bool resetState)
{
    CHECK(app->Inited, false, "Application has not been corectly initialized !");

    Internal::SystemEvent evnt;
    this->RepaintStatus      = REPAINT_STATUS_ALL;
    this->MouseLockedControl = nullptr;
    this->mouseLockedObject  = MouseLockedObject::None;

    if (resetState)
        this->loopStatus = LoopStatus::Normal;
    // hide current hovered control when new dialog is opened.
    if (this->MouseOverControl)
    {
        ((ControlContext*) (MouseOverControl->Context))->MouseIsOver = false;
        this->MouseOverControl                                       = nullptr;
    }

    PackControl(true);
    if (ctrl != nullptr)
    {
        CHECK(ModalControlsCount < MAX_MODAL_CONTROLS_STACK, false, "Too many modal calls !");
        ModalControlsStack[ModalControlsCount] = ctrl;
        ModalControlsCount++;
    }
    // update command bar
    UpdateCommandBar();

    while (loopStatus == LoopStatus::Normal)
    {
        if (!toDelete.empty())
        {
            for (auto c : toDelete)
            {
                // delete any potential references
                if (this->MouseLockedControl == c)
                    this->MouseLockedControl = nullptr;
                if (this->MouseOverControl == c)
                    this->MouseOverControl = nullptr;
                if (this->ExpandedControl == c)
                    this->ExpandedControl = nullptr;
                delete c;
            }
            toDelete.clear();
        }
        if (this->cmdBarUpdate)
        {
            UpdateCommandBar();
            RepaintStatus |= REPAINT_STATUS_DRAW;
        }
        if (RepaintStatus != REPAINT_STATUS_NONE)
        {
            if ((RepaintStatus & REPAINT_STATUS_COMPUTE_POSITION) != 0)
                ComputePositions();
            if ((RepaintStatus & REPAINT_STATUS_DRAW) != 0)
            {
                RepaintStatus = REPAINT_STATUS_NONE;
                this->Paint();
                // pentru cazul in care OnFocus sau OnLoseFocus schimba repaint status
                if ((RepaintStatus & REPAINT_STATUS_COMPUTE_POSITION) != 0)
                    ComputePositions();
                if (this->cmdBarUpdate)
                    UpdateCommandBar();
                if ((RepaintStatus & REPAINT_STATUS_DRAW) != 0)
                    this->Paint();
                this->terminal->Update();
            }
            RepaintStatus = REPAINT_STATUS_NONE;
        }
        this->terminal->GetSystemEvent(evnt);
        if (evnt.updateFrames)
        {
            if (ProcessUpdateFrameEvent(this->AppDesktop))
                this->RepaintStatus |= REPAINT_STATUS_DRAW;
            for (uint32 tr = 0; tr < ModalControlsCount; tr++)
                if (ProcessUpdateFrameEvent(this->ModalControlsStack[tr]))
                    this->RepaintStatus |= REPAINT_STATUS_DRAW;
        }
        switch (evnt.eventType)
        {
        case SystemEventType::AppClosed:
            loopStatus = LoopStatus::StopApp;
            break;
        case SystemEventType::AppResized:
            if (((evnt.newWidth != this->terminal->screenCanvas.GetWidth()) ||
                 (evnt.newHeight != this->terminal->screenCanvas.GetHeight())) &&
                (evnt.newWidth > 0) && (evnt.newHeight > 0))
            {
                LOG_INFO("New size for app: %dx%d", evnt.newWidth, evnt.newHeight);
                this->terminal->screenCanvas.Resize(evnt.newWidth, evnt.newHeight);
                this->AppDesktop->Resize(evnt.newWidth, evnt.newHeight);
                if (this->cmdBar)
                    this->cmdBar->SetDesktopSize(evnt.newWidth, evnt.newHeight);
                if (this->menu)
                    this->menu->SetWidth(evnt.newWidth);
                this->RepaintStatus = REPAINT_STATUS_ALL;
            }
            break;
        case SystemEventType::MouseDown:
            OnMouseDown(evnt.mouseX, evnt.mouseY, evnt.mouseButton);
            break;
        case SystemEventType::MouseUp:
            OnMouseUp(evnt.mouseX, evnt.mouseY, evnt.mouseButton);
            break;
        case SystemEventType::MouseMove:
            OnMouseMove(evnt.mouseX, evnt.mouseY, evnt.mouseButton);
            break;
        case SystemEventType::MouseWheel:
            OnMouseWheel(evnt.mouseX, evnt.mouseY, evnt.mouseWheel);
            break;
        case SystemEventType::KeyPressed:
            ProcessKeyPress(evnt.keyCode, evnt.unicodeCharacter);
            break;
        case SystemEventType::ShiftStateChanged:
            ProcessShiftState(evnt.keyCode);
            break;
        case SystemEventType::RequestRedraw:
            this->RepaintStatus = REPAINT_STATUS_ALL;
            break;
        default:
            break;
        }
    }
    if (ctrl != nullptr)
    {
        if (ModalControlsCount > 0)
            ModalControlsCount--;
        UpdateCommandBar();
        if (this->MouseOverControl)
        {
            ((ControlContext*) (MouseOverControl->Context))->MouseIsOver = false;
            this->MouseOverControl                                       = nullptr;
        }
        this->MouseLockedControl = nullptr;
        this->mouseLockedObject  = MouseLockedObject::None;
        RepaintStatus            = REPAINT_STATUS_ALL;
    }
    // check if current loop should be stop
    if (loopStatus == LoopStatus::StopCurrent)
    {
        loopStatus    = LoopStatus::Normal;
        RepaintStatus = REPAINT_STATUS_ALL;
        // check if desktop now has no children windows
        CheckIfAppShouldClose();
    }
    // pack extended control
    PackControl(true);
    return true;
}
void ApplicationImpl::SendCommand(int command)
{
    Control* ctrl = nullptr;

    if (ModalControlsCount == 0)
        ctrl = GetFocusedControl(this->AppDesktop);
    else
        ctrl = GetFocusedControl(ModalControlsStack[ModalControlsCount - 1]);

    if (ctrl != nullptr)
    {
        RaiseEvent(ctrl, nullptr, Controls::Event::Command, command);
        this->cmdBarUpdate = true;
    }
}
void ApplicationImpl::RaiseEvent(
      Reference<Controls::Control> control,
      Reference<Controls::Control> sourceControl,
      Controls::Event eventType,
      int controlID)
{
    while (control != nullptr)
    {
        if (((ControlContext*) (control->Context))->handlers)
        {
            const auto handle = ((ControlContext*) (control->Context))->handlers.get();
            if (handle->OnEvent.obj)
            {
                if (handle->OnEvent.obj->OnEvent(control, eventType, controlID))
                {
                    RepaintStatus |= REPAINT_STATUS_DRAW;
                    return;
                }
            }
            else
            {
                if (control->OnEvent(sourceControl, eventType, controlID) == true)
                {
                    RepaintStatus |= REPAINT_STATUS_DRAW;
                    return;
                }
            }
        }
        else
        {
            if (control->OnEvent(sourceControl, eventType, controlID) == true)
            {
                RepaintStatus |= REPAINT_STATUS_DRAW;
                return;
            }
        }
        control = control->GetParent();
    }
}
bool ApplicationImpl::SetToolTip(Reference<Controls::Control> control, const ConstString& text)
{
    return SetToolTip(control, text, -1, -1);
}
bool ApplicationImpl::SetToolTip(Reference<Controls::Control> control, const ConstString& text, int x, int y)
{
    if (!control.IsValid())
        control = this->AppDesktop;
    CREATE_CONTROL_CONTEXT(control, Members, false);
    if (!(Members->Flags & GATTR_VISIBLE))
        return false;
    if (!Members->ScreenClip.Visible)
        return false;
    // all good
    Graphics::Rect r;
    // compute point or rect
    if ((x >= 0) && (y >= 0)) // point coordonates
    {
        int w = (int) Members->Layout.Width;
        int h = (int) Members->Layout.Height;

        // recompute X and Y if object starts outside the screen
        if ((w != Members->ScreenClip.ClipRect.Width) && (Members->ScreenClip.ClipRect.X == 0))
            x += Members->ScreenClip.ClipRect.Width - w;

        if ((h != Members->ScreenClip.ClipRect.Height) && (Members->ScreenClip.ClipRect.Y == 0))
            y += Members->ScreenClip.ClipRect.Height - h;

        if ((x < Members->ScreenClip.ClipRect.Width) && (y < Members->ScreenClip.ClipRect.Height) && (x >= 0) &&
            (y >= 0))
        {
            r.Create(
                  Members->ScreenClip.ClipRect.X + x,
                  Members->ScreenClip.ClipRect.Y + y,
                  Members->ScreenClip.ClipRect.X + x,
                  Members->ScreenClip.ClipRect.Y + y);
        }
        else
        {
            return false; // nu se poate afisa
        }
    }
    else
    {
        r.Create(
              Members->ScreenClip.ClipRect.X,
              Members->ScreenClip.ClipRect.Y,
              Members->ScreenClip.ClipRect.X + Members->ScreenClip.ClipRect.Width - 1,
              Members->ScreenClip.ClipRect.Y + Members->ScreenClip.ClipRect.Height - 1);
    }

    return ToolTip.Show(text, r, terminal->screenCanvas.GetWidth(), terminal->screenCanvas.GetHeight());
}
void ApplicationImpl::Terminate()
{
    loopStatus = LoopStatus::StopApp;
}
bool ApplicationImpl::UnInit()
{
    CHECK(this->Inited, false, "Nothing to uninit --> have you called Application::Init(...) ?");
    this->terminal->UnInit();
    this->Inited = false;
    return true;
}
void ApplicationImpl::ArrangeWindows(Application::ArrangeWindowsMethod method)
{
    auto winList      = this->AppDesktop->GetChildrenList();
    auto winListCount = this->AppDesktop->GetChildrenCount();
    auto y            = 0;
    auto x            = 0;
    int tempSz        = 0;
    int gridX         = 0;
    int gridY         = 0;
    int gridWidth     = 0;
    int gridHeight    = 0;
    int gridRow       = 0;
    int gridColumn    = 0;
    int gridWinWidth  = 0;
    int gridWinHeight = 0;

    Graphics::Size sz = this->AppDesktop->GetClientSize();

    if (winListCount == 0)
        return; // nothing to arrange
    if ((sz.Width <= 1) || (sz.Height <= 1))
        return; // size too small --> nothing to arrange

    // all good - resize all existing wins

    // do the actual arrangement
    switch (method)
    {
    case Application::ArrangeWindowsMethod::MaximizedAll:
        while (winListCount > 0)
        {
            (*winList)->MoveTo(x, y);
            (*winList)->Resize(sz.Width, sz.Height);
            winList++;
            winListCount--;
        }
        break;
    case Application::ArrangeWindowsMethod::Cascade:
        while (winListCount > 0)
        {
            (*winList)->MoveTo(x, y);
            (*winList)->Resize(sz.Width, sz.Height);
            x++;
            y++;
            sz.Width  = std::max<>(sz.Width - 1, 10U);
            sz.Height = std::max<>(sz.Height - 1, 10U);
            winList++;
            winListCount--;
        }
        break;
    case Application::ArrangeWindowsMethod::Vertical:
        tempSz = sz.Width / winListCount;
        while (winListCount > 0)
        {
            (*winList)->MoveTo(x, y);
            if (winListCount == 1) // last one
                tempSz = std::max<>(1, ((int) sz.Width) - x);
            (*winList)->Resize(tempSz, sz.Height);
            x += (*winList)->GetWidth();
            winListCount--;
            winList++;
        }
        break;
    case Application::ArrangeWindowsMethod::Horizontal:
        tempSz = sz.Height / winListCount;
        while (winListCount > 0)
        {
            (*winList)->MoveTo(x, y);
            if (winListCount == 1) // last one
                tempSz = std::max<>(1, ((int) sz.Height) - y);
            (*winList)->Resize(sz.Width, tempSz);
            y += (*winList)->GetHeight();
            winListCount--;
            winList++;
        }
        break;
    case Application::ArrangeWindowsMethod::Grid:
        tempSz = (int) sqrt(winListCount);
        tempSz = std::max<>(tempSz, 1);
        gridX  = tempSz;
        gridY  = tempSz;
        if ((gridY * gridX) < (int) winListCount)
            gridX++; // more boxes on horizontal space
        if ((gridY * gridX) < (int) winListCount)
            gridY++; // more boxes on vertical space
        gridWidth  = sz.Width / gridX;
        gridHeight = sz.Height / gridY;
        gridRow    = 0;
        gridColumn = 0;
        tempSz     = x;
        while (winListCount > 0)
        {
            (*winList)->MoveTo(x, y);
            gridWinWidth  = gridWidth;
            gridWinHeight = gridHeight;
            if (((gridColumn + 1) == gridX) || (winListCount == 1)) // last column
                gridWinWidth = std::max<>(1, ((int) sz.Width) - x);
            if ((gridRow + 1) == gridY) // last row
                gridWinHeight = std::max<>(1, ((int) sz.Height) - y);

            (*winList)->Resize(gridWinWidth, gridWinHeight);
            x += (*winList)->GetWidth();
            gridColumn++;
            if (gridColumn >= gridX)
            {
                gridColumn = 0;
                x          = tempSz; // restore original "X" value
                y += (*winList)->GetHeight();
                gridRow++;
            }
            winListCount--;
            winList++;
        }
        break;
    default:
        break;
    }

    this->RepaintStatus = REPAINT_STATUS_ALL;
}
} // namespace AppCUI