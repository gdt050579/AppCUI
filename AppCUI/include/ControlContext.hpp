#ifndef __CONTROL_STANDARD_MEMBERS__
#define __CONTROL_STANDARD_MEMBERS__

#include "AppCUI.hpp"
#include "Internal.hpp"
#include <string.h>
#include <vector>

using namespace AppCUI;
using namespace AppCUI::Graphics;
using namespace AppCUI::Controls;
using namespace AppCUI::Utils;
using namespace AppCUI::Input;

constexpr unsigned int GATTR_ENABLE   = 0x000001;
constexpr unsigned int GATTR_VISIBLE  = 0x000002;
constexpr unsigned int GATTR_CHECKED  = 0x000004;
constexpr unsigned int GATTR_TABSTOP  = 0x000008;
constexpr unsigned int GATTR_VSCROLL  = 0x000010;
constexpr unsigned int GATTR_HSCROLL  = 0x000020;
constexpr unsigned int GATTR_EXPANDED = 0x000040;

enum class LayoutFormatMode: unsigned short
{
    None,
    PointAndSize,
    LeftRightAnchorsAndHeight,
    TopBottomAnchorsAndWidth,
    LeftTopRightBottomAnchors
};
enum class LayoutValueType: unsigned short
{
    CharacterOffset = 0,
    Percentage
};
struct LayoutValue
{
    short Value;
    LayoutValueType Type;
    inline int ToInt(int parentSize) const
    {
        if (Type == LayoutValueType::Percentage)
            return ((int) Value * parentSize) / 10000;
        else
            return (int) Value;
    }
};
struct LayoutInformation
{
    unsigned int flags;
    LayoutValue x, y;
    LayoutValue width, height;
    LayoutValue a_left, a_top, a_right, a_bottom;
    Alignament align, dock;
};
struct LayoutMetricData
{
    int ParentWidth, ParentHeigh;
    int X, Y, Width, Height, AnchorLeft, AnchorRight, AnchorTop, AnchorBottom;
    Alignament Align, Anchor;
};
struct ControlContext
{
  public:
    AppCUI::Graphics::Clip ScreenClip, ExpandedViewClip;
    struct
    {
        struct
        {
            LayoutValue Width, Height;
            LayoutValue AnchorLeft, AnchorRight, AnchorTop, AnchorBottom, X, Y;
            Alignament Align, Anchor;
            LayoutFormatMode LayoutMode;
        } Format;
        int X, Y;
        int Width, MinWidth, MaxWidth;
        int Height, MinHeight, MaxHeight;
    } Layout;
    struct
    {
        int Left, Top, Right, Bottom;
    } Margins;
    struct
    {
        unsigned int TopMargin;
        unsigned int LeftMargin;
        unsigned long long MaxHorizontalValue;
        unsigned long long MaxVerticalValue;
        unsigned long long HorizontalValue;
        unsigned long long VerticalValue;
        bool OutsideControl;
    } ScrollBars;
    int ControlID;
    int GroupID;
    AppCUI::Input::Key HotKey;
    unsigned int HotKeyOffset;
    unsigned int Flags, ControlsCount, CurrentControlIndex;
    AppCUI::Controls::Control** Controls;
    AppCUI::Controls::Control* Parent;
    AppCUI::Application::Config* Cfg;
    AppCUI::Graphics::CharacterBuffer Text;
    bool Inited, Focused, MouseIsOver;

    // Handlers
    struct
    {
        Handlers::AfterResizeHandler OnAfterResizeHandler;
        void* OnAfterResizeHandlerContext;
        Handlers::BeforeResizeHandler OnBeforeResizeHandler;
        void* OnBeforeResizeHandlerContext;
        Handlers::AfterMoveHandler OnAfterMoveHandler;
        void* OnAfterMoveHandlerContext;
        Handlers::UpdateCommandBarHandler OnUpdateCommandBarHandler;
        void* OnUpdateCommandBarHandlerContext;
        Handlers::KeyEventHandler OnKeyEventHandler;
        void* OnKeyEventHandlerContext;
        Handlers::PaintHandler OnPaintHandler;
        void* OnPaintHandlerContext;
        Handlers::OnFocusHandler OnFocusHandler;
        void* OnFocusHandlerContext;
        Handlers::OnFocusHandler OnLoseFocusHandler;
        void* OnLoseFocusHandlerContext;
        Handlers::MouseReleasedHandler OnMouseReleasedHandler;
        void* OnMouseReleasedHandlerContext;
        Handlers::MousePressedHandler OnMousePressedHandler;
        void* OnMousePressedHandlerContext;
        Handlers::EventHandler OnEventHandler;
        void* OnEventHandlerContext;
    } Handlers;

    ControlContext();

    bool ProcessDockedLayout(LayoutInformation& inf);
    bool ProcessXYWHLayout(LayoutInformation& inf);
    bool ProcessCornerAnchorLayout(LayoutInformation& inf, Alignament anchor);
    bool UpdateLayoutFormat(const std::string_view& format);
    bool RecomputeLayout_PointAndSize(const LayoutMetricData& md);
    bool RecomputeLayout(Control* parent);
    void PaintScrollbars(Graphics::Renderer& renderer);
};

constexpr unsigned int WINDOW_DRAG_STATUS_NONE = 0;
constexpr unsigned int WINDOW_DRAG_STATUS_MOVE = 1;
constexpr unsigned int WINDOW_DRAG_STATUS_SIZE = 2;

struct WindowControlContext : public ControlContext
{
  public:
    int oldPosX, oldPosY, oldW, oldH;
    int dragStatus, dragOffsetX, dragOffsetY;
    int MinWidth, MaxWidth, MinHeight, MaxHeight;
    int DialogResult;
    struct
    {
        unsigned int Left, Right, Y;
        inline bool Contains(unsigned int x, unsigned int y) const
        {
            return (y == Y) && (x >= Left) && (x <= Right);
        }
    } rCloseButton, rMaximizeButton, rResizeButton;
    unsigned char winButtonState;
    bool Maximized;
    std::unique_ptr<AppCUI::Internal::MenuBar> menu;
};

class SplitterControlContext : public ControlContext
{
  public:
    int SecondPanelSize;
    unsigned int DragStatus;
};
class TextFieldControlContext : public ControlContext
{
  public:
    struct
    {
        int StartOffset, Pos;
    } Cursor;
    struct
    {
        int Start, End, Origin;
    } Selection;
    struct
    {
        Handlers::SyntaxHighlightHandler Handler;
        void* Context;
    } Syntax;
    bool Modified;
};
class TextAreaControlContext : public ControlContext
{
  public:
    Utils::Array32 Lines;

    struct
    {
        unsigned int CurrentLine;
        unsigned int CurrentRow;
        unsigned int CurrentPosition;
        unsigned int HorizontalOffset;
        unsigned int TopLine;
        unsigned int VisibleLinesCount;
        unsigned int VisibleRowsCount;
    } View;
    struct
    {
        unsigned int Start, End, Origin;
    } Selection;
    struct
    {
        Handlers::SyntaxHighlightHandler Handler;
        void* Context;
    } Syntax;
    char tabChar;
    AppCUI::Controls::Control* Host;

    void ComputeVisibleLinesAndRows();

    void UpdateView();
    void UpdateLines();
    void SelAll();
    void ClearSel();
    void MoveSelectionTo(unsigned int poz);
    void UpdateViewXOffset();
    void DeleteSelected();
    unsigned int GetLineStart(unsigned int lineIndex);
    bool GetLineRange(unsigned int lineIndex, unsigned int& start, unsigned int& end);
    void DrawLineNumber(
          Graphics::Renderer& renderer, int lineIndex, int pozY, const Graphics::ColorPair lineNumberColor);
    void DrawLine(
          Graphics::Renderer& renderer,
          unsigned int lineIndex,
          int ofsX,
          int pozY,
          const Graphics::ColorPair textColor);
    void DrawToolTip();

    void MoveLeft(bool selected);
    void MoveRight(bool selected);
    void MoveUpDown(unsigned int times, bool moveUp, bool selected);
    void MoveTo(int newPoz, bool selected);
    void MoveHome(bool selected);
    void MoveEnd(bool selected);
    void MoveToStartOfTheFile(bool selected);
    void MoveToEndOfTheFile(bool selected);
    void AddChar(char16_t ch);
    void KeyDelete();
    void KeyBack();
    void CopyToClipboard();
    void PasteFromClipboard();
    bool HasSelection();

    void SetReadOnly(bool value);
    bool IsReadOnly();

    void SetToolTip(char* ss);
    void Paint(Graphics::Renderer& renderer);
    bool OnKeyEvent(AppCUI::Input::Key KeyCode, char16_t UnicodeChar);
    void OnAfterResize();
    void AnalyzeCurrentText();
    void SetSelection(unsigned int start, unsigned int end);
    void SetTabCharacter(char tabCharacter);
    void SendMsg(Event eventType);
};

struct TabControlContext : public ControlContext
{
  public:
    Control* currentTab;
    unsigned int TabTitleSize;
    int HoveredTabIndex;

    int MousePositionToPanel(int x, int y);
    void UpdateMargins();
    void PaintTopBottomPanelTab(Graphics::Renderer& renderer, bool onTop);
    void PaintLeftPanelTab(Graphics::Renderer& renderer);
    void PaintListPanelTab(Graphics::Renderer& renderer);
};
struct CanvasControlContext : public ControlContext
{
    AppCUI::Graphics::Canvas canvas;
    int CanvasScrollX, CanvasScrollY, mouseDragX, mouseDragY;
    bool dragModeEnabled;
    void MoveScrollTo(int newX, int newY);
};

constexpr unsigned int MAX_LISTVIEW_COLUMNS     = 64;
constexpr unsigned int MAX_LISTVIEW_HEADER_TEXT = 32;

struct ListViewItem
{
    CharacterBuffer SubItem[MAX_LISTVIEW_COLUMNS];
    ListViewItemType Type;
    unsigned short Flags;
    unsigned int XOffset;
    unsigned int Height;
    ColorPair ItemColor;
    ItemData Data;
    ListViewItem();
    ListViewItem(const ColorPair col) : ListViewItem()
    {
        this->ItemColor = col;
    }
    ListViewItem(const ListViewItem& obj);
    ListViewItem(ListViewItem&& obj) noexcept;
};
struct ListViewColumn
{
    CharacterBuffer Name;
    unsigned int HotKeyOffset;
    unsigned int Flags;
    Key HotKeyCode;
    unsigned short Width;
    TextAlignament Align;

    void Reset();
    bool SetName(const AppCUI::Utils::ConstString& text);
    bool SetAlign(TextAlignament align);
    void SetWidth(unsigned int width);
};

class ListViewControlContext : public ControlContext
{
  public:
    struct
    {
        ListViewColumn List[MAX_LISTVIEW_COLUMNS];
        unsigned int Count;
        unsigned int TotalWidth;
        unsigned int ResizeColumnIndex;
        unsigned int HoverColumnIndex;
        unsigned int HoverSeparatorColumnIndex;
        int XOffset;
        bool ResizeModeEnabled;
    } Columns;

    struct
    {
        unsigned int ColumnIndex;
        bool Ascendent;
        Handlers::ListViewItemComparer CompareCallbak;
        void* CompareCallbakContext;
    } SortParams;

    struct
    {
        std::vector<ListViewItem> List;
        Utils::Array32 Indexes;
        int FirstVisibleIndex, CurentItemIndex;
    } Items;

    struct
    {
        Utils::String SearchText;
        bool FilterModeEnabled;
    } Filter;

    struct
    {
        char Status[20];
        unsigned int StatusLength;
    } Selection;
    char clipboardSeparator;

    Controls::Control* Host;

    ListViewItem* GetFilteredItem(unsigned int index);

    int SearchItem(unsigned int startPoz, unsigned int colIndex);
    void UpdateSearch(int startPoz);
    void UpdateSelectionInfo();
    void DrawColumnSeparatorsForResizeMode(Graphics::Renderer& renderer);
    void DrawColumn(Graphics::Renderer& renderer);
    void DrawItem(Graphics::Renderer& renderer, ListViewItem* item, int y, bool currentItem);

    // movement
    void UpdateSelection(int start, int end, bool select);
    void MoveTo(int newItem);
    void ColumnSort(unsigned int columnIndex);

    // columns
    void UpdateColumnsWidth();
    bool AddColumn(const AppCUI::Utils::ConstString& text, TextAlignament Align, unsigned int width = 10);
    bool DeleteColumn(unsigned int index);
    void DeleteAllColumns();
    int GetNrColumns();

    // itemuri
    ItemHandle AddItem(const AppCUI::Utils::ConstString& text);
    bool SetItemText(ItemHandle item, unsigned int subItem, const AppCUI::Utils::ConstString& text);
    AppCUI::Graphics::CharacterBuffer* GetItemText(ItemHandle item, unsigned int subItem);
    bool SetItemCheck(ItemHandle item, bool check);
    bool SetItemSelect(ItemHandle item, bool select);
    bool SetItemColor(ItemHandle item, ColorPair color);
    bool SetItemType(ItemHandle item, ListViewItemType type);
    void SetClipboardSeparator(char ch);
    bool IsItemChecked(ItemHandle item);
    bool IsItemSelected(ItemHandle item);
    void SelectAllItems();
    void UnSelectAllItems();
    void CheckAllItems();
    void UncheckAllItems();
    unsigned int GetCheckedItemsCount();

    void DeleteAllItems();
    bool SetColumnClipboardCopyState(unsigned int columnIndex, bool allowCopy);
    bool SetColumnFilterMode(unsigned int columnIndex, bool allowFilterForThisColumn);

    bool SetCurrentIndex(ItemHandle item);
    int GetFirstVisibleLine();
    bool SetFirstVisibleLine(ItemHandle item);
    int GetVisibleItemsCount();

    bool SetItemData(ItemHandle item, ItemData Data);
    ItemData* GetItemData(ItemHandle item);
    bool SetItemXOffset(ItemHandle item, unsigned int XOffset);
    unsigned int GetItemXOffset(ItemHandle item);
    bool SetItemHeight(ItemHandle item, unsigned int Height);
    unsigned int GetItemHeight(ItemHandle item);

    void Paint(Graphics::Renderer& renderer);
    void OnMouseReleased(int x, int y, AppCUI::Input::MouseButton button);
    bool MouseToHeader(int x, int y, unsigned int& HeaderIndex, unsigned int& HeaderColumnIndex);
    void OnMousePressed(int x, int y, AppCUI::Input::MouseButton button);
    bool OnMouseDrag(int x, int y, AppCUI::Input::MouseButton button);
    bool OnMouseWheel(int x, int y, AppCUI::Input::MouseWheel direction);
    bool OnMouseOver(int x, int y);
    void SetSortColumn(unsigned int colIndex);
    bool OnKeyEvent(AppCUI::Input::Key keyCode, char16_t UnicodeChar);
    void SendMsg(Event eventType);
    bool Sort();

    void FilterItems();
};

struct ComboBoxItem
{
    AppCUI::Graphics::CharacterBuffer Text;
    AppCUI::Controls::ItemData Data;
    unsigned int Index;
    bool Separator;
    ComboBoxItem();
    ComboBoxItem(
          const AppCUI::Utils::ConstString& caption, ItemData userData, unsigned int index, bool separator = false);
    ~ComboBoxItem();
    ComboBoxItem(const ComboBoxItem&);
    ComboBoxItem(ComboBoxItem&&) noexcept;
    ComboBoxItem& operator=(const ComboBoxItem&);
    ComboBoxItem& operator=(ComboBoxItem&&) noexcept;
};
class ComboBoxControlContext : public ControlContext
{
  public:
    std::vector<ComboBoxItem> Items;
    AppCUI::Utils::Array32 Indexes;
    unsigned int ExpandedHeight, FirstVisibleItem, VisibleItemsCount, HoveredIndexItem;
    unsigned int CurentItemIndex;
};

class NumericSelectorControlContext : public ControlContext
{
  public:
    long long minValue;
    long long maxValue;
    long long value;

    const int buttonPadding = 4;
    LocalString<256> stringValue;
    bool intoInsertionMode       = false;
    bool wasMinusPressed         = false;
    long long insertionModevalue = 0;
    bool wrongValueInserted      = false;
    long long sliderPosition     = 0;
    bool isMouseLeftClickPressed = false;
    
    enum class IsMouseOn
    {
        None,
        MinusButton,
        TextField,
        PlusButton
    } isMouseOn{ IsMouseOn::None };
};

enum class MenuItemType : unsigned int
{
    Invalid,
    Command,
    Check,
    Radio,
    Line,
    SubMenu
};
struct MenuItem
{
    CharacterBuffer Name;
    unsigned int HotKeyOffset;
    AppCUI::Input::Key HotKey;
    AppCUI::Input::Key ShortcutKey;
    MenuItemType Type;
    int CommandID;
    bool Enabled;
    bool Checked;
    Menu* SubMenu;

    MenuItem(); // line
    MenuItem(
          MenuItemType type,
          const AppCUI::Utils::ConstString& text,
          int CommandID,
          bool checked,
          AppCUI::Input::Key shortcutKey);                           // commands
    MenuItem(const AppCUI::Utils::ConstString& text, Menu* subMenu); // submenu
    MenuItem(const MenuItem& obj) = delete;
    MenuItem(MenuItem&& obj)      = delete;
    ~MenuItem();
};
enum class MousePressedResult : unsigned int
{
    None,
    Repaint,
    CheckParent,
    Activate
};
enum class MenuButtonState : unsigned char
{
    Normal,
    Hovered,
    Pressed
};

struct MenuMousePositionInfo
{
    unsigned int ItemIndex;
    bool IsOnMenu;
    bool IsOnUpButton;
    bool IsOnDownButton;
};

constexpr unsigned int MAX_NUMBER_OF_MENU_ITEMS = 256;
struct MenuContext
{
    // std::vector messes up with inter-items pointers when calling copy/move ctor
    // as a result, opening a sub-menu from another is likely to produce a crash (as the pointers will be invalid)
    // switching to regular pointer of std::unique_ptr type to avoid this

    std::unique_ptr<MenuItem> Items[MAX_NUMBER_OF_MENU_ITEMS];
    unsigned int ItemsCount;
    Menu* Parent;
    AppCUI::Internal::MenuBar* Owner;
    AppCUI::Graphics::Clip ScreenClip;
    AppCUI::Application::Config* Cfg;
    unsigned int FirstVisibleItem;
    unsigned int VisibleItemsCount;
    unsigned int CurrentItem;
    unsigned int Width, TextWidth;
    MenuButtonState ButtonUp, ButtonDown;

    MenuContext();
    ItemHandle AddItem(std::unique_ptr<MenuItem> itm);

  public:
    // methods
    void Paint(AppCUI::Graphics::Renderer& renderer, bool activ);

    void RunItemAction(unsigned int index);
    void CloseMenu();

    // Move
    void UpdateFirstVisibleItem();
    void CreateAvailableItemsList(unsigned int* indexes, unsigned int& count);
    void MoveCurrentItemTo(AppCUI::Input::Key keyCode);

    // Check
    bool SetChecked(unsigned int index, bool status);

    // mouse events
    void ComputeMousePositionInfo(int x, int y, MenuMousePositionInfo& mpi);
    bool OnMouseMove(int x, int y, bool& repaint);
    MousePressedResult OnMousePressed(int x, int y);
    bool IsOnMenu(int x, int y);
    bool OnMouseWheel(int x, int y, AppCUI::Input::MouseWheel direction);

    // key events
    bool OnKeyEvent(AppCUI::Input::Key keyCode);
    bool ProcessShortCut(AppCUI::Input::Key keyCode);

    // Show
    void Show(
          AppCUI::Controls::Menu* me,
          AppCUI::Controls::Control* relativeControl,
          int x,
          int y,
          const AppCUI::Graphics::Size& maxSize);
};

#define CREATE_CONTROL_CONTEXT(object, name, retValue)                                                                 \
    ControlContext* name = (ControlContext*) ((object)->Context);                                                      \
    if (name == nullptr)                                                                                               \
        return retValue;
#define CREATE_TYPECONTROL_CONTEXT(type, name, retValue)                                                               \
    type* name = (type*) ((this)->Context);                                                                            \
    if (name == nullptr)                                                                                               \
        return retValue;
#define CREATE_TYPE_CONTEXT(type, object, name, retValue)                                                              \
    type* name = (type*) ((object)->Context);                                                                          \
    if (name == nullptr)                                                                                               \
        return retValue;
#define CONTROL_INIT_CONTEXT(type) CHECK((Context = new type()) != nullptr, false, "Unable to create control context");
#define DELETE_CONTROL_CONTEXT(type)                                                                                   \
    if (Context != nullptr)                                                                                            \
    {                                                                                                                  \
        type* c = (type*) Context;                                                                                     \
        delete c;                                                                                                      \
        Context = nullptr;                                                                                             \
    }

#endif
