#ifndef __CONTROL_STANDARD_MEMBERS__
#define __CONTROL_STANDARD_MEMBERS__

#include "AppCUI.h"
#include <string.h>
#include <vector>

using namespace AppCUI;
using namespace AppCUI::Controls;
using namespace AppCUI::Utils;

#define GATTR_ENABLE	0x000001
#define GATTR_VISIBLE	0x000002
#define GATTR_CHECKED	0x000004
#define GATTR_TABSTOP	0x000008
#define GATTR_VSCROLL   0x000010
#define GATTR_HSCROLL   0x000020



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
    struct {
        unsigned int                        TopMargin;
        unsigned int                        LeftMargin;
        unsigned long long                  MaxHorizontalValue;
        unsigned long long                  MaxVerticalValue;
        unsigned long long                  HorizontalValue;
        unsigned long long                  VerticalValue;
        bool                                OutsideControl;
    } ScrollBars;
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
    void    PaintScrollbars(Console::Renderer & renderer);
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
        Handlers::SyntaxHighlightHandler   Handler;
        void*                              Context;
    } Syntax;
    bool Modified;
};
class TextAreaControlContext : public ControlContext
{
public:
    Utils::Array32              Lines;

    struct {
        unsigned int            CurrentLine;
        unsigned int            CurrentRow;
        unsigned int            CurrentPosition;
        unsigned int            HorizontalOffset;
        unsigned int            TopLine;
        unsigned int            VisibleLinesCount;
        unsigned int            VisibleRowsCount;
    } View;
    struct {
        unsigned int            Start, End, Origin;
    } Selection;
    struct {
        Handlers::SyntaxHighlightHandler    Handler;
        void*                               Context;
    } Syntax;
    char						tabChar;
    AppCUI::Controls::Control*  Host;

    void    ComputeVisibleLinesAndRows();

    void	        UpdateView();
    void	        UpdateLines();
    void	        SelAll();
    void	        ClearSel();
    void	        MoveSelectionTo(unsigned int poz);
    void            UpdateViewXOffset();
    void	        DeleteSelected();
    unsigned int	GetLineStart(unsigned int lineIndex);
    bool            GetLineRange(unsigned int lineIndex, unsigned int & start, unsigned int &end);
    void	        DrawLineNumber(Console::Renderer & renderer, int lineIndex, int pozY, const Console::ColorPair lineNumberColor);
    void	        DrawLine(Console::Renderer & renderer, unsigned int lineIndex, int ofsX, int pozY, const Console::ColorPair textColor);
    void	        DrawToolTip();


    void    MoveLeft(bool selected);
    void    MoveRight(bool selected);
    void	MoveUpDown(unsigned int times,bool moveUp, bool selected);
    void	MoveTo(int newPoz, bool selected);
    void	MoveHome(bool selected);
    void	MoveEnd(bool selected);
    void    MoveToStartOfTheFile(bool selected);
    void    MoveToEndOfTheFile(bool selected);
    void	AddChar(char ch);
    void	KeyDelete();
    void	KeyBack();
    void	CopyToClipboard();
    void	PasteFromClipboard();
    bool	HasSelection();


    void	SetReadOnly(bool value);
    bool	IsReadOnly();

    void	SetToolTip(char *ss);
    void	Paint(Console::Renderer & renderer);
    bool	OnKeyEvent(int KeyCode, char AsciiCode);
    void	OnAfterResize();
    void	AnalyzeCurrentText();
    void	SetSelection(unsigned int start, unsigned int end);
    void	SetTabCharacter(char tabCharacter);
    void	SendMsg(Event::Type eventType);
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