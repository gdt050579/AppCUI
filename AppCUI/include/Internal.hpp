#ifndef __APPCUI_INTERNAL_HEADER__
#define __APPCUI_INTERNAL_HEADER__

#include "AppCUI.hpp"
#include "OSDefinitions.h"
#include <memory>

#define REPAINT_STATUS_COMPUTE_POSITION		1
#define REPAINT_STATUS_DRAW					2
#define REPAINT_STATUS_ALL					(REPAINT_STATUS_COMPUTE_POSITION|REPAINT_STATUS_DRAW)
#define REPAINT_STATUS_NONE					0

#define MOUSE_LOCKED_OBJECT_NONE			0
#define MOUSE_LOCKED_OBJECT_ACCELERATOR		1
#define MOUSE_LOCKED_OBJECT_CONTROL			2

#define MAX_MODAL_CONTROLS_STACK			16

#define LOOP_STATUS_NORMAL					0
#define LOOP_STATUS_STOP_CURRENT			1
#define LOOP_STATUS_STOP_APP				2

#define IS_CONTROL_AVAILABLE(ctrl)			((bool)((ctrl->Members.Flags & (Flags::GATTR_VISIBLE | Flags::GATTR_VISIBLE)) == (Flags::GATTR_VISIBLE | Flags::GATTR_VISIBLE)))

#define MAX_COMMANDBAR_FIELD_NAME		    24
#define MAX_COMMANDBAR_SHIFTSTATES		    8

#define CURRENT_CONSOLE_WIDTH               0xFFFFFFFF
#define CURRENT_CONSOLE_HEIGHT              0xFFFFFFFF

#define NEW_LINE_CODE                       10

#define SET_CHARACTER_EX(ptrCharInfo,value,color) {\
    if (value>=0) { SET_CHARACTER_VALUE(ptrCharInfo,value); } \
    if (color<256) { \
        SET_CHARACTER_COLOR(ptrCharInfo, color); \
    } else { \
        if (color != AppCUI::Console::Color::NoColor) { \
            unsigned int temp_color = color; \
            if (color & 256) temp_color = (GET_CHARACTER_COLOR(ptrCharInfo) & 0x0F)|(temp_color & 0xFFFFF0); \
            if (color & (256<<4)) temp_color = (GET_CHARACTER_COLOR(ptrCharInfo) & 0xF0)|(temp_color & 0xFFFF0F); \
            SET_CHARACTER_COLOR(ptrCharInfo,(temp_color & 0xFF));\
        } \
    } \
}


namespace AppCUI
{
    namespace Internal
    {
        namespace SystemEvents
        {
            enum Type : unsigned int
            {
                NONE = 0,
                MOUSE_DOWN,
                MOUSE_UP,
                MOUSE_MOVE,
                APP_CLOSE,
                APP_RESIZED,
                KEY_PRESSED,
                SHIFT_STATE_CHANGED
            };
            struct Event
            {
                SystemEvents::Type      eventType;
                int                     mouseX, mouseY;
                unsigned int            newWidth, newHeight;
                unsigned int            mouseButtonState;
                AppCUI::Input::Key      keyCode;
                char                    asciiCode;
            };
        }

        struct CommandBarField
        {
            int				         Command, StartScreenPos, EndScreenPos;
            AppCUI::Input::Key KeyCode;
            int                      NameWidth;
            char			         Name[MAX_COMMANDBAR_FIELD_NAME];
            int                      KeyNameWidth;
            const char*		         KeyName;
            unsigned int	         Version;
        };
        struct CommandBarFieldIndex
        {
            CommandBarField*			Field;
        };
        class CommandBarController
        {
            CommandBarField	        Fields[MAX_COMMANDBAR_FIELD_NAME][(unsigned int)AppCUI::Input::Key::Count];
            CommandBarFieldIndex    VisibleFields[MAX_COMMANDBAR_SHIFTSTATES][(unsigned int)AppCUI::Input::Key::Count];
            int					    IndexesCount[MAX_COMMANDBAR_SHIFTSTATES];
            bool			        HasKeys[MAX_COMMANDBAR_SHIFTSTATES];

            struct {
                int Y, Width;                
            } BarLayout;

            struct {
                const char *                Name;
                unsigned int                Size;
            } ShiftStatus;
            
            AppCUI::Application::Config	*   Cfg;
            AppCUI::Input::Key              CurrentShiftKey;
            int						        LastCommand;
            CommandBarField*	            PressedField;
            CommandBarField*	            HoveredField;
            unsigned int			        CurrentVersion;
            bool					        RecomputeScreenPos;
            bool                            Visible;

            void					        ComputeScreenPos();
            bool                            CleanFieldStatus();
            CommandBarField*	            MousePositionToField(int x, int y);
        public:
            void	    Init(unsigned int desktopWidth, unsigned int desktopHeight, AppCUI::Application::Config * cfg, bool visible);
            void	    Paint(AppCUI::Console::Renderer & renderer);
            void	    Clear();
            void        SetDesktopSize(unsigned int width, unsigned int height);
            bool	    Set(AppCUI::Input::Key keyCode, const char* Name, int Command);
            bool	    SetShiftKey(AppCUI::Input::Key keyCode);
            bool        OnMouseOver(int x, int y, bool & repaint);
            bool        OnMouseDown();
            bool        OnMouseUp(int & command);            
            int		    GetCommandForKey(AppCUI::Input::Key keyCode);
            inline bool IsVisible() const { return Visible; }
        };

        class DesktopControl: public AppCUI::Controls::Control 
        {
        public:
            bool    Create(unsigned int width, unsigned int height);
            void    Paint(AppCUI::Console::Renderer & renderer) override;
            bool    OnKeyEvent(AppCUI::Input::Key keyCode, char AsciiCode) override;
        };

        enum class TerminalType : unsigned int
        {
            Default = 0,
            SDL,
            Terminal,
            Windows
        };
        enum class TerminalSize : unsigned int
        {
            Default = 0,
            CustomSize,
            Maximized,
            FullScreen
        };
        enum class CharacterSize : unsigned int
        {
            Default = 0,
            Tiny,
            Small,
            Normal,
            Large,
            Huge
        };

        struct InitializationData
        {
            unsigned int    Width, Height;
            TerminalType    FrontEnd;
            CharacterSize   CharSize;
            TerminalSize    TermSize;
            bool            FixedSize;

            bool BuildFrom(AppCUI::Application::InitializationFlags flags, unsigned int Width, unsigned int Height);
        };

        class AbstractTerminal
        {
        protected:
            AbstractTerminal();

        public:
            unsigned int                LastCursorX, LastCursorY;
            AppCUI::Console::Canvas     OriginalScreenCanvas, ScreenCanvas;
            bool                        Inited, LastCursorVisibility;

            virtual bool    OnInit(const InitializationData& initData) = 0;
            virtual void    RestoreOriginalConsoleSettings() = 0;
            virtual void    OnUninit() = 0;
            virtual void    OnFlushToScreen() = 0;
            virtual bool    OnUpdateCursor() = 0;
            virtual void    GetSystemEvent(AppCUI::Internal::SystemEvents::Event & evnt) = 0;
            virtual bool    IsEventAvailable() = 0;

            virtual ~AbstractTerminal();

            bool    Init(const InitializationData& initData);
            void    Uninit();
            void    Update();

        };

        struct Application
        {
            AppCUI::Application::Config             config;
            std::unique_ptr<AbstractTerminal>       terminal;
            bool                                    Inited;
            
            DesktopControl			                Desktop;
            CommandBarController	                CommandBarObject;
            AppCUI::Application::CommandBar         CommandBarWrapper;
            AppCUI::Controls::Control *             ModalControlsStack[MAX_MODAL_CONTROLS_STACK];
            AppCUI::Controls::Control *             MouseLockedControl;
            AppCUI::Controls::Control *             MouseOverControl;
            AppCUI::Controls::Control *             ExpandedControl;
            unsigned int			                ModalControlsCount;            
            int						                LoopStatus;
            unsigned int			                RepaintStatus;
            int						                MouseLockedObject;

            Application();
            ~Application();
            
            void    Destroy();
            void	ComputePositions();
            void	ProcessKeyPress(AppCUI::Input::Key keyCode, int AsciiCode);
            void	ProcessShiftState(AppCUI::Input::Key ShiftState);
            void	OnMouseDown(int x, int y, int buttonState);
            void	OnMouseUp(int x, int y, int buttonState);
            void	OnMouseMove(int x, int y, int buttonState);
            void	OnMouseWheel();
            void	SendCommand(int command);
            void	Terminate();

            // Pack/Expand
            void    PackControl(bool redraw);
            bool    ExpandControl(AppCUI::Controls::Control * ctrl);

            //Common implementations
            bool    Init(AppCUI::Application::InitializationFlags flags, unsigned int width, unsigned int height);
            bool    Uninit();
            bool    ExecuteEventLoop(AppCUI::Controls::Control *control = nullptr);
            void    Paint();
            void    RaiseEvent(AppCUI::Controls::Control *control, AppCUI::Controls::Control *sourceControl, AppCUI::Controls::Event eventType, int controlID);
        };
    }
    namespace Application
    {
        AppCUI::Internal::Application* GetApplication();
    }
    namespace Log
    {
        void Unit(); // needed to release some alocation buffers
    }
}


#endif
