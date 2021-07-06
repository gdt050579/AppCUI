#ifndef __CONTROL_STANDARD_MEMBERS__
#define __CONTROL_STANDARD_MEMBERS__

#include "AppCUI.h"
#include <string.h>

using namespace AppCUI;
using namespace AppCUI::Controls;
using namespace AppCUI::Utils;

#define GATTR_ENABLE	0x000001
#define GATTR_VISIBLE	0x000002
#define GATTR_CHECKED	0x000004
#define GATTR_TABSTOP	0x000008

struct ControlContext
{

public:
	AppCUI::Console::Clip			        ScreenClip;
    struct 
    {
        struct
        {
            int                             Width, Height;
            int                             AnchorLeft, AnchorRight, AnchorTop, AnchorBottom;
            unsigned short                  PercentageMask;
            unsigned char                   LayoutMode;
        } Format;
        int									X, Y;
        int                                 Width, MinWidth, MaxWidth;
        int                                 Height, MinHeight, MaxHeight;
    } Layout;
    struct 
    {
        int                                 Left, Top, Right, Bottom;
    } Margins;
	int										ControlID;
	int										GroupID;
    AppCUI::Input::Key::Type                HotKey;
    unsigned int                            HotKeyOffset;
	unsigned int							Flags, ControlsCount, CurrentControlIndex;
	AppCUI::Controls::Control				**Controls;
    AppCUI::Controls::Control				*Parent;
    AppCUI::Application::Config              *Cfg;
    AppCUI::Console::CharacterBuffer		Text;
	bool									Inited,Focused,MouseIsOver;

	// Handlers
	struct {
		Handlers::AfterResizeHandler		OnAfterResizeHandler;
		void*								OnAfterResizeHandlerContext;
		Handlers::BeforeResizeHandler		OnBeforeResizeHandler;
		void*								OnBeforeResizeHandlerContext;
		Handlers::AfterMoveHandler			OnAfterMoveHandler;
		void*								OnAfterMoveHandlerContext;
		Handlers::UpdateCommandBarHandler	OnUpdateCommandBarHandler;
		void*								OnUpdateCommandBarHandlerContext;
		Handlers::KeyEventHandler			OnKeyEventHandler;
		void*								OnKeyEventHandlerContext;
		Handlers::PaintHandler				OnPaintHandler;
		void*								OnPaintHandlerContext;
		Handlers::OnFocusHandler			OnFocusHandler;
		void*								OnFocusHandlerContext;
        Handlers::OnFocusHandler			OnLoseFocusHandler;
        void*								OnLoseFocusHandlerContext;
		Handlers::MouseReleasedHandler		OnMouseReleasedHandler;
		void*								OnMouseReleasedHandlerContext;
		Handlers::MousePressedHandler		OnMousePressedHandler;
		void*								OnMousePressedHandlerContext;
		Handlers::EventHandler				OnEventHandler;
		void*								OnEventHandlerContext;
	} Handlers;

	ControlContext();

    bool    UpdateLayoutFormat(const char * format);
    bool    RecomputeLayout(Control *parent);
};

#define WINDOW_DRAG_STATUS_NONE	0
#define WINDOW_DRAG_STATUS_MOVE	1
#define WINDOW_DRAG_STATUS_SIZE 2

struct WindowControlContext : public ControlContext
{
public:
    int							oldPosX, oldPosY, oldW, oldH;
    int							dragStatus, dragOffsetX, dragOffsetY;
    int							MinWidth, MaxWidth, MinHeight, MaxHeight;
    int							DialogResult;
    struct {
        unsigned int            Left, Right, Y;
        inline bool             Contains(unsigned int x, unsigned int y) const { return (y == Y) && (x >= Left) && (x <= Right); }
    } rCloseButton, rMaximizeButton, rResizeButton;
    unsigned char               winButtonState;
    bool						Maximized;
};

class SplitterControlContext : public ControlContext
{
public:
    int					SecondPanelSize;
    unsigned int        DragStatus;
};
class TextFieldControlContext : public ControlContext
{
public:
    struct {
        int StartOffset, Pos;
    } Cursor;
    struct {
        int Start, End, Origin;
    } Selection;
    struct {
        Handlers::TextFieldSyntaxHighlightHandler   Handler;
        void*                                       Context;
    } SyntaxHighlight;
    bool Modified;
};
struct TabControlContext : public ControlContext
{
public:
    Control*        currentTab;
    unsigned int    TabTitleSize;
    int             HoveredTabIndex;


    int             MousePositionToPanel(int x, int y);
    void            UpdateMargins();
    void            PaintTopPanelTab(Console::Renderer & renderer);
    void            PaintBottomPanelTab(Console::Renderer & renderer);
    void            PaintLeftPanelTab(Console::Renderer & renderer);
    void            PaintListPanelTab(Console::Renderer & renderer);
};
struct CanvasControlContext : public ControlContext
{
    AppCUI::Console::Canvas     canvas;
    int                         CanvasScrollX, CanvasScrollY;
};


#define CREATE_CONTROL_CONTEXT(object,name,retValue)		ControlContext * name = (ControlContext*)((object)->Context); if (name == nullptr) return retValue;
#define CREATE_TYPECONTROL_CONTEXT(type,name,retValue)		type * name = (type*)((this)->Context); if (name == nullptr) return retValue;
#define CREATE_TYPE_CONTEXT(type,object,name,retValue)		type * name = (type*)((object)->Context); if (name == nullptr) return retValue;
#define CONTROL_INIT_CONTEXT(type)                          CHECK((Context = new type())!=nullptr,false,"Unable to create control context");
#define DELETE_CONTROL_CONTEXT(type) 	                    if (Context != nullptr) { type *c = (type*)Context; delete c;	Context = nullptr; }

#endif