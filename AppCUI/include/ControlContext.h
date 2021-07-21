#ifndef __CONTROL_STANDARD_MEMBERS__
#define __CONTROL_STANDARD_MEMBERS__

#include "AppCUI.h"
#include <string.h>
#include <vector>

using namespace AppCUI;
using namespace AppCUI::Console;
using namespace AppCUI::Controls;
using namespace AppCUI::Utils;
using namespace AppCUI::Input;

#define GATTR_ENABLE	0x000001
#define GATTR_VISIBLE	0x000002
#define GATTR_CHECKED	0x000004
#define GATTR_TABSTOP	0x000008
#define GATTR_VSCROLL   0x000010
#define GATTR_HSCROLL   0x000020
#define GATTR_EXPANDED  0x000040



struct ControlContext
{

public:
	AppCUI::Console::Clip			        ScreenClip,ExpandedViewClip;
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
    AppCUI::Input::Key                HotKey;
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
    bool	OnKeyEvent(AppCUI::Input::Key KeyCode, char AsciiCode);
    void	OnAfterResize();
    void	AnalyzeCurrentText();
    void	SetSelection(unsigned int start, unsigned int end);
    void	SetTabCharacter(char tabCharacter);
    void	SendMsg(Event eventType);
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

#define MAX_LISTVIEW_COLUMNS			64
#define MAX_LISTVIEW_HEADER_TEXT        32

struct ListViewItem
{
    Utils::String	    SubItem[MAX_LISTVIEW_COLUMNS];    
    ListViewItemType    Type;
    unsigned short		Flags;
    unsigned int		XOffset;
    unsigned int		Height;
    ColorPair           ItemColor;
    ItemData			Data;
    ListViewItem();
    ListViewItem(const ColorPair col) : ListViewItem() { this->ItemColor = col; }
    ListViewItem(const ListViewItem & obj);
};
struct ListViewColumn
{
    char				Name[MAX_LISTVIEW_HEADER_TEXT];
    unsigned short		Width;
    unsigned char       HotKeyOffset;
    unsigned char       NameLength;
    Key			HotKeyCode;
    unsigned int		Flags;
    TextAlignament		Align;

    void                Reset();
    bool                SetName(const char * text);
    bool                SetAlign(TextAlignament align);
    void                SetWidth(unsigned int width);
};

class ListViewControlContext : public ControlContext
{
public:
    struct {
        ListViewColumn		            List[MAX_LISTVIEW_COLUMNS];
        unsigned int		            Count;
        unsigned int                    TotalWidth;
        unsigned int				    ResizeColumnIndex;
        unsigned int				    HoverColumnIndex;
        unsigned int                    HoverSeparatorColumnIndex;
        int                             XOffset;
        bool				            ResizeModeEnabled;
    } Columns;

    struct {
        unsigned int			        ColumnIndex;
        bool                            Ascendent;
        Handlers::ListViewItemComparer	CompareCallbak;
        void*					        CompareCallbakContext;
    } SortParams;

    struct {
        std::vector<ListViewItem>	List;
        Utils::Array32              Indexes;
        int					        FirstVisibleIndex, CurentItemIndex;
    } Items;

    struct
    {
        Utils::String               SearchText;
        bool				        FilterModeEnabled;
    } Filter;
   
    struct {
        char				        Status[20];
        unsigned int                StatusLength;
    } Selection;
    char				        clipboardSeparator;

    Controls::Control*	        Host;

    ListViewItem*		GetFilteredItem(unsigned int index);

    int					SearchItem(unsigned int startPoz, unsigned int colIndex);
    void				UpdateSearch(int startPoz);
    void                UpdateSelectionInfo();
    void                DrawColumnSeparatorsForResizeMode(Console::Renderer & renderer);
    void				DrawColumn(Console::Renderer & renderer);
    void				DrawItem(Console::Renderer & renderer, ListViewItem * item, int y, bool currentItem);

    // movement
    void				UpdateSelection(int start, int end, bool select);
    void				MoveTo(int newItem);
    void				ColumnSort(unsigned int columnIndex);

    // columns
    void                UpdateColumnsWidth();
    bool				AddColumn(const char *text, TextAlignament Align, unsigned int width = 10);
    bool				DeleteColumn(unsigned int index);
    void				DeleteAllColumns();
    int					GetNrColumns();

    // itemuri
    ItemHandle			AddItem(const char *text);
    bool				SetItemText(ItemHandle item, unsigned int subItem, const char *text);
    const char*			GetItemText(ItemHandle item, unsigned int subItem);
    bool				SetItemCheck(ItemHandle item, bool check);
    bool				SetItemSelect(ItemHandle item, bool select);
    bool				SetItemColor(ItemHandle item, ColorPair color);
    bool				SetItemType(ItemHandle item, ListViewItemType type);
    void				SetClipboardSeparator(char ch);
    bool				IsItemChecked(ItemHandle item);
    bool				IsItemSelected(ItemHandle item);
    void				SelectAllItems();
    void				UnSelectAllItems();
    void				CheckAllItems();
    void				UncheckAllItems();
    unsigned int		GetCheckedItemsCount();

    void				DeleteAllItems();
    bool				SetColumnClipboardCopyState(unsigned int columnIndex, bool allowCopy);
    bool				SetColumnFilterMode(unsigned int columnIndex, bool allowFilterForThisColumn);

    bool				SetCurrentIndex(ItemHandle item);
    int					GetFirstVisibleLine();
    bool				SetFirstVisibleLine(ItemHandle item);
    int					GetVisibleItemsCount();

    bool				SetItemData(ItemHandle item, ItemData Data);
    ItemData*			GetItemData(ItemHandle item);
    bool				SetItemXOffset(ItemHandle item, unsigned int XOffset);
    unsigned int		GetItemXOffset(ItemHandle item);
    bool				SetItemHeight(ItemHandle item, unsigned int Height);
    unsigned int		GetItemHeight(ItemHandle item);

    void				Paint(Console::Renderer & renderer);
    void				OnMouseReleased(int x, int y, int butonState);
    bool                MouseToHeader(int x, int y, unsigned int &HeaderIndex, unsigned int &HeaderColumnIndex);
    void                OnMousePressed(int x, int y, int butonState);
    bool                OnMouseDrag(int x, int y, int butonState);
    bool                OnMouseOver(int x, int y);
    void				SetSortColumn(unsigned int colIndex);
    bool				OnKeyEvent(AppCUI::Input::Key keyCode, char AsciiCode);
    void				SendMsg(Event eventType);
    bool				Sort();

    void				FilterItems();

};

#define COMBOBOX_ITEM_SIZE		48

struct COMBO_ITEM_INFO
{
    char	                    Nume[COMBOBOX_ITEM_SIZE];
    AppCUI::Controls::ItemData	Data;
};
class ComboBoxControlContext : public ControlContext
{
public:

    std::vector<COMBO_ITEM_INFO>	Items;
    unsigned int                    ExpandedHeight, FirstVisibleItem, CurentItemIndex, VisibleItems, HoveredIndexItem;
};



#define CREATE_CONTROL_CONTEXT(object,name,retValue)		ControlContext * name = (ControlContext*)((object)->Context); if (name == nullptr) return retValue;
#define CREATE_TYPECONTROL_CONTEXT(type,name,retValue)		type * name = (type*)((this)->Context); if (name == nullptr) return retValue;
#define CREATE_TYPE_CONTEXT(type,object,name,retValue)		type * name = (type*)((object)->Context); if (name == nullptr) return retValue;
#define CONTROL_INIT_CONTEXT(type)                          CHECK((Context = new type())!=nullptr,false,"Unable to create control context");
#define DELETE_CONTROL_CONTEXT(type) 	                    if (Context != nullptr) { type *c = (type*)Context; delete c;	Context = nullptr; }

#endif