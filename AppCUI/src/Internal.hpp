#pragma once

#include "AppCUI.hpp"

#ifdef _WIN32
#    include <windows.h>
#    include <Shlobj.h>
#else
#    include <unistd.h>
#    include <sys/ioctl.h>
#    include <stdlib.h>
#    include <cstdlib>
#    include <fcntl.h>
#    include <errno.h>
#    include <sys/stat.h>
#endif

#include <stdio.h>
#include <iostream>

namespace AppCUI
{
constexpr uint32 REPAINT_STATUS_COMPUTE_POSITION = 1;
constexpr uint32 REPAINT_STATUS_DRAW             = 2;
constexpr uint32 REPAINT_STATUS_ALL              = (REPAINT_STATUS_COMPUTE_POSITION | REPAINT_STATUS_DRAW);
constexpr uint32 REPAINT_STATUS_NONE             = 0;

constexpr uint32 MAX_MODAL_CONTROLS_STACK   = 16;
constexpr uint32 MAX_COMMANDBAR_SHIFTSTATES = 8;

constexpr char NEW_LINE_CODE = 10;

namespace Internal
{
    enum class LoopStatus : uint32
    {
        Normal = 0,
        StopCurrent,
        StopApp
    };
    enum class MouseLockedObject : uint32
    {
        None = 0,
        CommandBar,
        Control
    };
    enum class SystemEventType : uint32
    {
        None = 0,
        MouseDown,
        MouseUp,
        MouseMove,
        MouseWheel,
        AppClosed,
        AppResized,
        KeyPressed,
        ShiftStateChanged,
        RequestRedraw,
    };
    struct SystemEvent
    {
        SystemEventType eventType;
        int mouseX, mouseY;
        uint32 newWidth, newHeight;
        Input::MouseButton mouseButton;
        Input::MouseWheel mouseWheel;
        Input::Key keyCode;
        char16_t unicodeCharacter;
        bool updateFrames;
    };

    struct CommandBarField
    {
        int Command, StartScreenPos, EndScreenPos;
        Input::Key KeyCode;
        string_view KeyName;
        Graphics::CharacterBuffer Name;
        uint32 ClearCommandUniqueID;
    };
    struct CommandBarFieldIndex
    {
        CommandBarField* Field;
    };
    class CommandBarController
    {
        CommandBarField Fields[MAX_COMMANDBAR_SHIFTSTATES][(uint32) Input::Key::Count];
        CommandBarFieldIndex VisibleFields[MAX_COMMANDBAR_SHIFTSTATES][(uint32) Input::Key::Count];
        int IndexesCount[MAX_COMMANDBAR_SHIFTSTATES];
        bool HasKeys[MAX_COMMANDBAR_SHIFTSTATES];

        struct
        {
            int Y, Width;
        } BarLayout;

        string_view ShiftStatus;

        Application::Config* Cfg;
        Input::Key CurrentShiftKey;
        int LastCommand;
        CommandBarField* PressedField;
        CommandBarField* HoveredField;
        uint32 ClearCommandUniqueID;
        bool RecomputeScreenPos;

        void ComputeScreenPos();
        bool CleanFieldStatus();
        CommandBarField* MousePositionToField(int x, int y);

      public:
        CommandBarController(uint32 desktopWidth, uint32 desktopHeight, Application::Config* cfg);
        void Paint(Graphics::Renderer& renderer);
        void Clear();
        void SetDesktopSize(uint32 width, uint32 height);
        bool Set(Input::Key keyCode, const ConstString& caption, int Command);
        bool SetShiftKey(Input::Key keyCode);
        bool OnMouseMove(int x, int y, bool& repaint);
        bool OnMouseDown();
        bool OnMouseUp(int& command);
        int GetCommandForKey(Input::Key keyCode);
    };

    struct MenuBarItem
    {
        Controls::Menu Mnu;
        Graphics::CharacterBuffer Name;
        Input::Key HotKey;
        uint32 HotKeyOffset;
        int X;
        MenuBarItem();
    };
    class MenuBar
    {
        static const constexpr uint32 MAX_ITEMS = 32;
        unique_ptr<MenuBarItem> Items[MAX_ITEMS];
        Application::Config* Cfg;
        Controls::Control* Parent;
        uint32 ItemsCount;
        uint32 OpenedItem;
        uint32 HoveredItem;
        uint32 Width;
        int X, Y;

        uint32 MousePositionToItem(int x, int y);
        void Open(uint32 menuIndex);

      public:
        MenuBar(Controls::Control* parent = nullptr, int x = 0, int y = 0);

        Controls::ItemHandle AddMenu(const ConstString& name);
        Controls::Menu* GetMenu(Controls::ItemHandle itemHandle);
        void RecomputePositions();
        void SetWidth(uint32 value);
        void Paint(Graphics::Renderer& renderer);
        bool OnMouseMove(int x, int y, bool& repaint);
        bool OnMousePressed(int x, int y, Input::MouseButton button);
        void Close();
        bool IsOpened();
        bool OnKeyEvent(Input::Key keyCode);
    };

    class TextControlDefaultMenu
    {
        Controls::ItemHandle itemCopy;
        Controls::ItemHandle itemCut;
        Controls::ItemHandle itemDelete;
        Controls::ItemHandle itemPaste;
        Controls::ItemHandle itemSelectAll;
        Controls::ItemHandle itemToUpper;
        Controls::ItemHandle itemToLower;
        Controls::Menu menu;

      public:
        TextControlDefaultMenu();
        ~TextControlDefaultMenu();
        void Show(Utils::Reference<Controls::Control> parent, int x, int y, bool hasSelection);

        static constexpr int TEXTCONTROL_CMD_COPY            = 1200001;
        static constexpr int TEXTCONTROL_CMD_CUT             = 1200002;
        static constexpr int TEXTCONTROL_CMD_PASTE           = 1200003;
        static constexpr int TEXTCONTROL_CMD_SELECT_ALL      = 1200004;
        static constexpr int TEXTCONTROL_CMD_DELETE_SELECTED = 1200005;
        static constexpr int TEXTCONTROL_CMD_TO_UPPER        = 1200006;
        static constexpr int TEXTCONTROL_CMD_TO_LOWER        = 1200007;
    };

    class ToolTipController
    {
        Graphics::CharacterBuffer Text;
        Application::Config* Cfg;
        Graphics::Rect TextRect;
        Graphics::Point Arrow;
        Graphics::SpecialChars ArrowChar;
        Graphics::WriteTextParams TxParams;

      public:
        Graphics::Clip ScreenClip;

        bool Visible;

      public:
        ToolTipController();
        bool Show(const ConstString& text, Graphics::Rect& objRect, int screenWidth, int screenHeight);
        void Hide();
        void Paint(Graphics::Renderer& renderer);
    };

    class AbstractTerminal
    {
      protected:
        AbstractTerminal();

      public:
        uint32 LastCursorX, LastCursorY;
        Graphics::Canvas OriginalScreenCanvas, ScreenCanvas;
        bool Inited, LastCursorVisibility;

        virtual bool OnInit(const Application::InitializationData& initData)  = 0;
        virtual void RestoreOriginalConsoleSettings()                         = 0;
        virtual void OnUninit()                                               = 0;
        virtual void OnFlushToScreen()                                        = 0;
        virtual void OnFlushToScreen(const Graphics::Rect& r)                 = 0;
        virtual bool OnUpdateCursor()                                         = 0;
        virtual void GetSystemEvent(Internal::SystemEvent& evnt)              = 0;
        virtual bool IsEventAvailable()                                       = 0;
        virtual bool HasSupportFor(Application::SpecialCharacterSetType type) = 0;

        virtual ~AbstractTerminal();

        bool Init(const Application::InitializationData& initData);
        void Uninit();
        void Update();
    };

    namespace Config
    {
        void SetTheme(AppCUI::Application::Config& config, AppCUI::Application::ThemeType type);
        bool Save(AppCUI::Application::Config& config, const std::filesystem::path& outputFile);
        bool Load(AppCUI::Application::Config& config, const std::filesystem::path& inputFile);
    }; // namespace Config

    struct ApplicationImpl
    {
        Application::Config config;
        Utils::IniObject settings;
        unique_ptr<AbstractTerminal> terminal;
        Application::FrontendType frontend;
        unique_ptr<CommandBarController> cmdBar;
        unique_ptr<MenuBar> menu;
        vector<Controls::Control*> toDelete;

        Controls::Desktop* AppDesktop;
        ToolTipController ToolTip;
        Application::CommandBar CommandBarWrapper;

        Controls::Control* ModalControlsStack[MAX_MODAL_CONTROLS_STACK];
        Controls::Control* MouseLockedControl;
        Controls::Control* MouseOverControl;
        Controls::Control* ExpandedControl;
        Controls::Menu* VisibleMenu;
        uint32 ModalControlsCount;
        LoopStatus loopStatus;
        uint32 RepaintStatus;
        MouseLockedObject mouseLockedObject;

        Application::InitializationFlags InitFlags;
        uint32 LastWindowID;
        int LastMouseX, LastMouseY;
        bool Inited;
        bool cmdBarUpdate;

        ApplicationImpl();
        ~ApplicationImpl();

        
        Application::FrontendType GetFrontendType() const;

        void Destroy();
        void ComputePositions();
        void ProcessKeyPress(Input::Key keyCode, char16_t unicodeCharacter);
        void ProcessShiftState(Input::Key ShiftState);
        void ProcessMenuMouseClick(Controls::Menu* mnu, int x, int y);
        void ProcessMenuMouseReleased(Controls::Menu* mnu, int x, int y);
        bool ProcessMenuAndCmdBarMouseMove(int x, int y);
        void OnMouseDown(int x, int y, Input::MouseButton button);
        void OnMouseUp(int x, int y, Input::MouseButton button);
        void OnMouseMove(int x, int y, Input::MouseButton button);
        void OnMouseWheel(int x, int y, Input::MouseWheel direction);
        void SendCommand(int command);
        void Terminate();

        // Pack/Expand
        void PackControl(bool redraw);
        bool ExpandControl(Controls::Control* ctrl);

        // Menu
        void CloseContextualMenu();
        void ShowContextualMenu(Controls::Menu* mnu);

        // Common implementations
        void LoadSettingsFile(Application::InitializationData& initData);
        bool LoadThemeFile(Application::InitializationData& initData);
        bool Init(Application::InitializationData& initData);
        bool Uninit();
        void CheckIfAppShouldClose();
        bool ExecuteEventLoop(Controls::Control* control = nullptr);
        void Paint();
        void RaiseEvent(
              Utils::Reference<Controls::Control> control,
              Utils::Reference<Controls::Control> sourceControl,
              Controls::Event eventType,
              int controlID);
        bool SetToolTip(Utils::Reference<Controls::Control> control, const ConstString& text);
        bool SetToolTip(Utils::Reference<Controls::Control> control, const ConstString& text, int x, int y);

        void ArrangeWindows(Application::ArrangeWindowsMethod method);
    };
} // namespace Internal
namespace Application
{
    Internal::ApplicationImpl* GetApplication();
}
namespace Utils
{
    struct UnicodeChar
    {
        uint16 Value;
        uint32 Length;
    };
    struct UTF8Char
    {
        uint8 Values[8];
        uint32 Length;
    };
    bool ConvertUTF8CharToUnicodeChar(const char8* p, const char8* end, UnicodeChar& result);
    bool ConvertUnicodeCharToUTF8Chat(char16 ch, UTF8Char& result);
    struct KeyValuePair
    {
        enum class Type : uint8
        {
            None,
            String,
            Number,
            Percentage
        };
        struct
        {
            const void* data;
            uint32 dataSize;
            uint64 hash;
            int32 number;
            Type type;
        } Key, Value;
    };
    class KeyValueParser
    {
        constexpr static uint32 MAX_ITEMS = 32;
        constexpr static uint32 NO_ERRORS = 0xFFFFFFFF;
        KeyValuePair items[MAX_ITEMS];
        uint32 errorPos;
        string_view errorName;
        uint32 count;

      public:
        KeyValueParser() : errorPos(NO_ERRORS), count(0)
        {
        }
        bool Parse(std::string_view text);
        bool Parse(std::u16string_view text);
        inline bool HasError() const
        {
            return errorPos != NO_ERRORS;
        }
        inline uint32 GetErrorOffset() const
        {
            return errorPos;
        }
        inline std::string_view GetErrorName() const
        {
            return errorName;
        }
        inline uint32 GetCount() const
        {
            return count;
        }
        inline const KeyValuePair& operator[](int index) const
        {
            return items[index];
        }
    };
} // namespace Utils
namespace Log
{
    void Unit(); // needed to release some alocation buffers
}
namespace Controls
{
    void UninitTextFieldDefaultMenu();
    void UninitTextAreaDefaultMenu();
    namespace ToolTip
    {
        void Hide();
        bool Show(Reference<Control> host, const ConstString& caption, int x, int y);
    }; // namespace ToolTip
} // namespace Controls
} // namespace AppCUI
