#pragma once

#include "AppCUI.hpp"
#include "OSDefinitions.hpp"
#include <memory>

#define REPAINT_STATUS_COMPUTE_POSITION 1
#define REPAINT_STATUS_DRAW             2
#define REPAINT_STATUS_ALL              (REPAINT_STATUS_COMPUTE_POSITION | REPAINT_STATUS_DRAW)
#define REPAINT_STATUS_NONE             0

#define MOUSE_LOCKED_OBJECT_NONE        0
#define MOUSE_LOCKED_OBJECT_ACCELERATOR 1
#define MOUSE_LOCKED_OBJECT_CONTROL     2

#define MAX_MODAL_CONTROLS_STACK 16

#define LOOP_STATUS_NORMAL       0
#define LOOP_STATUS_STOP_CURRENT 1
#define LOOP_STATUS_STOP_APP     2

#define IS_CONTROL_AVAILABLE(ctrl)                                                                                     \
    ((bool) ((ctrl->Members.Flags & (Flags::GATTR_VISIBLE | Flags::GATTR_VISIBLE)) == (Flags::GATTR_VISIBLE | Flags::GATTR_VISIBLE)))

#define MAX_COMMANDBAR_FIELD_NAME  24
#define MAX_COMMANDBAR_SHIFTSTATES 8

#define NEW_LINE_CODE 10

#define SET_CHARACTER_EX(ptrCharInfo, value, color)                                                                    \
    {                                                                                                                  \
        if (value >= 0)                                                                                                \
        {                                                                                                              \
            SET_CHARACTER_VALUE(ptrCharInfo, value);                                                                   \
        }                                                                                                              \
        if (color < 256)                                                                                               \
        {                                                                                                              \
            SET_CHARACTER_COLOR(ptrCharInfo, color);                                                                   \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            if (color != AppCUI::Graphics::Color::NoColor)                                                             \
            {                                                                                                          \
                unsigned int temp_color = color;                                                                       \
                if (color & 256)                                                                                       \
                    temp_color = (GET_CHARACTER_COLOR(ptrCharInfo) & 0x0F) | (temp_color & 0xFFFFF0);                  \
                if (color & (256 << 4))                                                                                \
                    temp_color = (GET_CHARACTER_COLOR(ptrCharInfo) & 0xF0) | (temp_color & 0xFFFF0F);                  \
                SET_CHARACTER_COLOR(ptrCharInfo, (temp_color & 0xFF));                                                 \
            }                                                                                                          \
        }                                                                                                              \
    }

namespace AppCUI
{
namespace Internal
{
    enum class SystemEventType : unsigned int
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
        unsigned int newWidth, newHeight;
        AppCUI::Input::MouseButton mouseButton;
        AppCUI::Input::MouseWheel mouseWheel;
        AppCUI::Input::Key keyCode;
        char16_t unicodeCharacter;
        bool updateFrames;
    };

    struct CommandBarField
    {
        int Command, StartScreenPos, EndScreenPos;
        AppCUI::Input::Key KeyCode;
        std::string_view KeyName;
        AppCUI::Graphics::CharacterBuffer Name;
        unsigned int ClearCommandUniqueID;
    };
    struct CommandBarFieldIndex
    {
        CommandBarField* Field;
    };
    class CommandBarController
    {
        CommandBarField Fields[MAX_COMMANDBAR_SHIFTSTATES][(unsigned int) AppCUI::Input::Key::Count];
        CommandBarFieldIndex VisibleFields[MAX_COMMANDBAR_SHIFTSTATES][(unsigned int) AppCUI::Input::Key::Count];
        int IndexesCount[MAX_COMMANDBAR_SHIFTSTATES];
        bool HasKeys[MAX_COMMANDBAR_SHIFTSTATES];

        struct
        {
            int Y, Width;
        } BarLayout;

        std::string_view ShiftStatus;

        AppCUI::Application::Config* Cfg;
        AppCUI::Input::Key CurrentShiftKey;
        int LastCommand;
        CommandBarField* PressedField;
        CommandBarField* HoveredField;
        unsigned int ClearCommandUniqueID;
        bool RecomputeScreenPos;

        void ComputeScreenPos();
        bool CleanFieldStatus();
        CommandBarField* MousePositionToField(int x, int y);

      public:
        CommandBarController(unsigned int desktopWidth, unsigned int desktopHeight, AppCUI::Application::Config* cfg);
        void Paint(AppCUI::Graphics::Renderer& renderer);
        void Clear();
        void SetDesktopSize(unsigned int width, unsigned int height);
        bool Set(AppCUI::Input::Key keyCode, const AppCUI::Utils::ConstString& caption, int Command);
        bool SetShiftKey(AppCUI::Input::Key keyCode);
        bool OnMouseMove(int x, int y, bool& repaint);
        bool OnMouseDown();
        bool OnMouseUp(int& command);
        int GetCommandForKey(AppCUI::Input::Key keyCode);
    };

    struct MenuBarItem
    {
        AppCUI::Controls::Menu Mnu;
        AppCUI::Graphics::CharacterBuffer Name;
        AppCUI::Input::Key HotKey;
        unsigned int HotKeyOffset;
        int X;
        MenuBarItem();
    };
    class MenuBar
    {
        static const constexpr unsigned int MAX_ITEMS = 32;
        std::unique_ptr<MenuBarItem> Items[MAX_ITEMS];
        AppCUI::Application::Config* Cfg;
        AppCUI::Controls::Control* Parent;
        unsigned int ItemsCount;
        unsigned int OpenedItem;
        unsigned int HoveredItem;
        unsigned int Width;
        int X, Y;

        unsigned int MousePositionToItem(int x, int y);
        void Open(unsigned int menuIndex);

      public:
        MenuBar(AppCUI::Controls::Control* parent = nullptr, int x = 0, int y = 0);

        AppCUI::Controls::ItemHandle AddMenu(const AppCUI::Utils::ConstString& name);
        AppCUI::Controls::Menu* GetMenu(AppCUI::Controls::ItemHandle itemHandle);
        void RecomputePositions();
        void SetWidth(unsigned int value);
        void Paint(AppCUI::Graphics::Renderer& renderer);
        bool OnMouseMove(int x, int y, bool& repaint);
        bool OnMousePressed(int x, int y, AppCUI::Input::MouseButton button);
        void Close();
        bool IsOpened();
        bool OnKeyEvent(AppCUI::Input::Key keyCode);
    };

    class TextControlDefaultMenu
    {
        AppCUI::Controls::ItemHandle itemCopy;
        AppCUI::Controls::ItemHandle itemCut;
        AppCUI::Controls::ItemHandle itemDelete;
        AppCUI::Controls::ItemHandle itemPaste;
        AppCUI::Controls::ItemHandle itemSelectAll;
        AppCUI::Controls::Menu menu;

      public:
        TextControlDefaultMenu();
        ~TextControlDefaultMenu();
        void Show(AppCUI::Utils::Reference<AppCUI::Controls::Control> parent, int x, int y, bool hasSelection);

        static constexpr int TEXTCONTROL_CMD_COPY            = 1200001;
        static constexpr int TEXTCONTROL_CMD_CUT             = 1200002;
        static constexpr int TEXTCONTROL_CMD_PASTE           = 1200003;
        static constexpr int TEXTCONTROL_CMD_SELECT_ALL      = 1200004;
        static constexpr int TEXTCONTROL_CMD_DELETE_SELECTED = 1200005;
    };

    class ToolTipController
    {
        AppCUI::Graphics::CharacterBuffer Text;
        AppCUI::Application::Config* Cfg;
        AppCUI::Graphics::Rect TextRect;
        AppCUI::Graphics::Point Arrow;
        AppCUI::Graphics::SpecialChars ArrowChar;
        AppCUI::Graphics::WriteTextParams TxParams;

      public:
        AppCUI::Graphics::Clip ScreenClip;

        bool Visible;

      public:
        ToolTipController();
        bool Show(
              const AppCUI::Utils::ConstString& text,
              AppCUI::Graphics::Rect& objRect,
              int screenWidth,
              int screenHeight);
        void Hide();
        void Paint(AppCUI::Graphics::Renderer& renderer);
    };

    class AbstractTerminal
    {
      protected:
        AbstractTerminal();

      public:
        unsigned int LastCursorX, LastCursorY;
        AppCUI::Graphics::Canvas OriginalScreenCanvas, ScreenCanvas;
        bool Inited, LastCursorVisibility;

        virtual bool OnInit(const AppCUI::Application::InitializationData& initData) = 0;
        virtual void RestoreOriginalConsoleSettings()                                = 0;
        virtual void OnUninit()                                                      = 0;
        virtual void OnFlushToScreen()                                               = 0;
        virtual bool OnUpdateCursor()                                                = 0;
        virtual void GetSystemEvent(AppCUI::Internal::SystemEvent& evnt)             = 0;
        virtual bool IsEventAvailable()                                              = 0;

        virtual ~AbstractTerminal();

        bool Init(const AppCUI::Application::InitializationData& initData);
        void Uninit();
        void Update();
    };

    struct Application
    {
        AppCUI::Application::Config config;
        AppCUI::Utils::IniObject settings;
        std::unique_ptr<AbstractTerminal> terminal;
        std::unique_ptr<CommandBarController> cmdBar;
        std::unique_ptr<MenuBar> menu;
        std::vector<AppCUI::Controls::Control*> toDelete;

        AppCUI::Controls::Desktop* AppDesktop;
        ToolTipController ToolTip;
        AppCUI::Application::CommandBar CommandBarWrapper;

        AppCUI::Controls::Control* ModalControlsStack[MAX_MODAL_CONTROLS_STACK];
        AppCUI::Controls::Control* MouseLockedControl;
        AppCUI::Controls::Control* MouseOverControl;
        AppCUI::Controls::Control* ExpandedControl;
        AppCUI::Controls::Menu* VisibleMenu;
        unsigned int ModalControlsCount;
        int LoopStatus;
        unsigned int RepaintStatus;
        int MouseLockedObject;

        AppCUI::Application::InitializationFlags InitFlags;
        unsigned int LastWindowID;
        int LastMouseX, LastMouseY;
        bool Inited;
        bool cmdBarUpdate;

        Application();
        ~Application();

        void Destroy();
        void ComputePositions();
        void ProcessKeyPress(AppCUI::Input::Key keyCode, char16_t unicodeCharacter);
        void ProcessShiftState(AppCUI::Input::Key ShiftState);
        void ProcessMenuMouseClick(AppCUI::Controls::Menu* mnu, int x, int y);
        bool ProcessMenuAndCmdBarMouseMove(int x, int y);
        void OnMouseDown(int x, int y, AppCUI::Input::MouseButton button);
        void OnMouseUp(int x, int y, AppCUI::Input::MouseButton button);
        void OnMouseMove(int x, int y, AppCUI::Input::MouseButton button);
        void OnMouseWheel(int x, int y, AppCUI::Input::MouseWheel direction);
        void SendCommand(int command);
        void Terminate();

        // Pack/Expand
        void PackControl(bool redraw);
        bool ExpandControl(AppCUI::Controls::Control* ctrl);

        // Menu
        void CloseContextualMenu();
        void ShowContextualMenu(AppCUI::Controls::Menu* mnu);

        // Common implementations
        void LoadSettingsFile(AppCUI::Application::InitializationData& initData);
        bool Init(AppCUI::Application::InitializationData& initData);
        bool Uninit();
        bool ExecuteEventLoop(AppCUI::Controls::Control* control = nullptr);
        void Paint();
        void RaiseEvent(
              AppCUI::Utils::Reference<AppCUI::Controls::Control> control,
              AppCUI::Utils::Reference<AppCUI::Controls::Control> sourceControl,
              AppCUI::Controls::Event eventType,
              int controlID);
        bool SetToolTip(AppCUI::Controls::Control* control, const AppCUI::Utils::ConstString& text);
        bool SetToolTip(AppCUI::Controls::Control* control, const AppCUI::Utils::ConstString& text, int x, int y);

        void ArrangeWindows(AppCUI::Application::ArangeWindowsMethod method);
    };
} // namespace Internal
namespace Application
{
    AppCUI::Internal::Application* GetApplication();
}
namespace Utils
{
    struct UnicodeChar
    {
        unsigned short Value;
        unsigned int Length;
    };
    bool ConvertUTF8CharToUnicodeChar(const char8_t* p, const char8_t* end, UnicodeChar& result);
} // namespace Utils
namespace Log
{
    void Unit(); // needed to release some alocation buffers
}
namespace Controls
{
    void UninitTextFieldDefaultMenu();
}
} // namespace AppCUI
