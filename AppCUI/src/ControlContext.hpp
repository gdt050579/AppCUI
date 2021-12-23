#pragma once

#include "AppCUI.hpp"
#include "Internal.hpp"
#include <string.h>
#include <vector>
#include <map>
#include <set>

namespace AppCUI
{
using namespace Graphics;
using namespace Controls;
using namespace Utils;
using namespace Input;

constexpr uint32 GATTR_ENABLE   = 0x000001;
constexpr uint32 GATTR_VISIBLE  = 0x000002;
constexpr uint32 GATTR_CHECKED  = 0x000004;
constexpr uint32 GATTR_TABSTOP  = 0x000008;
constexpr uint32 GATTR_VSCROLL  = 0x000010;
constexpr uint32 GATTR_HSCROLL  = 0x000020;
constexpr uint32 GATTR_EXPANDED = 0x000040;

enum class LayoutFormatMode : uint16
{
    None,
    PointAndSize,
    LeftRightAnchorsAndHeight,
    TopBottomAnchorsAndWidth,

    LeftTopRightAnchorsAndHeight,
    LeftBottomRightAnchorsAndHeight,
    TopLeftBottomAnchorsAndWidth,
    TopRightBottomAnchorsAndWidth,

    LeftTopRightBottomAnchors
};
enum class LayoutValueType : uint16
{
    CharacterOffset = 0,
    Percentage
};
struct LayoutValue
{
    int16 Value;
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
    uint32 flags;
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
    Graphics::Clip ScreenClip, ExpandedViewClip;
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
        uint32 TopMargin;
        uint32 LeftMargin;
        uint64 MaxHorizontalValue;
        uint64 MaxVerticalValue;
        uint64 HorizontalValue;
        uint64 VerticalValue;
        bool OutsideControl;
    } ScrollBars;
    int ControlID;
    int GroupID;
    Input::Key HotKey;
    uint32 HotKeyOffset;
    uint32 Flags, ControlsCount, CurrentControlIndex;
    Controls::Control** Controls;
    Controls::Control* Parent;
    Application::Config* Cfg;
    Graphics::CharacterBuffer Text;
    bool Inited, Focused, MouseIsOver;

    // Handlers
    unique_ptr<Controls::Handlers::Control> handlers;

    ControlContext();

    bool ProcessDockedLayout(LayoutInformation& inf);
    bool ProcessXYWHLayout(LayoutInformation& inf);
    bool ProcessCornerAnchorLayout(LayoutInformation& inf, Alignament anchor);
    bool ProcessHorizontalParalelAnchors(LayoutInformation& inf);
    bool ProcessVerticalParalelAnchors(LayoutInformation& inf);
    bool ProcessLTRAnchors(LayoutInformation& inf);
    bool ProcessLBRAnchors(LayoutInformation& inf);
    bool ProcessTLBAnchors(LayoutInformation& inf);
    bool ProcessTRBAnchors(LayoutInformation& inf);
    bool ProcessLTRBAnchors(LayoutInformation& inf);
    bool UpdateLayoutFormat(string_view format);
    void SetControlSize(uint32 width, uint32 heigh);
    bool RecomputeLayout_PointAndSize(const LayoutMetricData& md);
    bool RecomputeLayout_LeftRightAnchorsAndHeight(const LayoutMetricData& md);
    bool RecomputeLayout_TopBottomAnchorsAndWidth(const LayoutMetricData& md);

    bool RecomputeLayout(Control* parent);
    void PaintScrollbars(Graphics::Renderer& renderer);
};

constexpr uint32 WINDOW_DRAG_STATUS_NONE = 0;
constexpr uint32 WINDOW_DRAG_STATUS_MOVE = 1;
constexpr uint32 WINDOW_DRAG_STATUS_SIZE = 2;

constexpr uint32 MAX_WINDOWBAR_ITEMS = 32;

enum class WindowBarItemType : unsigned char
{
    None = 0,
    HotKeY,
    CloseButton,
    MaximizeRestoreButton,
    WindowResize,
    Tag,
    Button,
    SingleChoice,
    CheckBox,
    Text
};
enum class WindowBarItemFlags : unsigned char
{
    None             = 0x00,
    Visible          = 0x01,
    Hidden           = 0x02,
    Checked          = 0x04,
    LeftGroupMarker  = 0x08,
    RightGroupMarker = 0x10
};
struct WindowControlContext;
struct WindowBarItem
{
    Graphics::CharacterBuffer ToolTipText;
    Graphics::CharacterBuffer Text;
    int Size, X, Y, ID;
    uint32 HotKeyOffset;
    Key HotKey;
    WindowControlsBarLayout Layout;
    WindowBarItemType Type;
    WindowBarItemFlags Flags;
    inline bool IsVisible() const
    {
        return (((unsigned char) Flags) & (unsigned char) WindowBarItemFlags::Visible) != 0;
    }
    inline bool IsHidden() const
    {
        return (((unsigned char) Flags) & (unsigned char) WindowBarItemFlags::Hidden) != 0;
    }
    inline bool IsChecked() const
    {
        return (((unsigned char) Flags) & (unsigned char) WindowBarItemFlags::Checked) != 0;
    }
    inline bool Contains(int x, int y) const
    {
        return (y == Y) && (x >= X) && (x < (X + Size)) && (IsVisible()) && (!IsHidden());
    }
    inline int CenterX() const
    {
        return X + Size / 2;
    }
    inline void SetFlag(WindowBarItemFlags flg)
    {
        Flags = static_cast<WindowBarItemFlags>(((unsigned char) Flags) | ((unsigned char) flg));
    }
    inline void RemoveFlag(WindowBarItemFlags flg)
    {
        Flags = static_cast<WindowBarItemFlags>(((unsigned char) Flags) & (~((unsigned char) flg)));
    }
    bool Init(WindowBarItemType type, WindowControlsBarLayout layout, unsigned char size, string_view toolTipText);
    bool Init(
          WindowBarItemType type, WindowControlsBarLayout layout, const ConstString& name, const ConstString& toolTip);
};
struct WindowControlContext : public ControlContext
{
  public:
    unique_ptr<Internal::MenuBar> menu;
    Controls::ItemHandle referalItemHandle;
    Controls::ItemHandle windowItemHandle;
    int oldPosX, oldPosY, oldW, oldH;
    int dragStatus, dragOffsetX, dragOffsetY;
    int TitleLeftMargin, TitleMaxWidth;
    int DialogResult;
    struct
    {
        WindowBarItem Items[MAX_WINDOWBAR_ITEMS];
        uint32 Count;
        unsigned char Current;
        bool IsCurrentItemPressed;
    } ControlBar;
    bool Maximized;
};

enum class SplitterMouseStatus : unsigned char
{
    None = 0,
    OnButton1,
    OnButton2,
    OnBar,
    ClickOnButton1,
    ClickOnButton2,
    Drag
};
class SplitterControlContext : public ControlContext
{
  public:
    int SecondPanelSize;
    SplitterMouseStatus mouseStatus;
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
    bool Modified;
    bool FullSelectionDueToOnFocusEvent;
};
class TextAreaControlContext : public ControlContext
{
  public:
    Utils::Array32 Lines;

    struct
    {
        uint32 CurrentLine;
        uint32 CurrentRow;
        uint32 CurrentPosition;
        uint32 HorizontalOffset;
        uint32 TopLine;
        uint32 VisibleLinesCount;
        uint32 VisibleRowsCount;
    } View;
    struct
    {
        uint32 Start, End, Origin;
    } Selection;
    char tabChar;
    Controls::Control* Host;

    void ComputeVisibleLinesAndRows();

    void UpdateView();
    void UpdateLines();
    void SelAll();
    void ClearSel();
    void MoveSelectionTo(uint32 poz);
    void UpdateViewXOffset();
    void DeleteSelected();
    uint32 GetLineStart(uint32 lineIndex);
    bool GetLineRange(uint32 lineIndex, uint32& start, uint32& end);
    void DrawLineNumber(
          Graphics::Renderer& renderer, int lineIndex, int pozY, const Graphics::ColorPair lineNumberColor);
    void DrawLine(
          Graphics::Renderer& renderer, uint32 lineIndex, int ofsX, int pozY, const Graphics::ColorPair textColor);
    void DrawToolTip();

    void MoveLeft(bool selected);
    void MoveRight(bool selected);
    void MoveUpDown(uint32 times, bool moveUp, bool selected);
    void MoveTo(int newPoz, bool selected);
    void MoveHome(bool selected);
    void MoveEnd(bool selected);
    void MoveToNextWord(bool selected);
    void MoveToPreviousWord(bool selected);
    void MoveToStartOfTheFile(bool selected);
    void MoveToEndOfTheFile(bool selected);
    void AddChar(char16 ch);
    void KeyDelete();
    void KeyBack();
    void CopyToClipboard();
    void PasteFromClipboard();
    bool HasSelection();
    void ToUpper();
    void ToLower();

    void SetReadOnly(bool value);
    bool IsReadOnly();

    void SetToolTip(char* ss);
    void Paint(Graphics::Renderer& renderer);
    bool OnKeyEvent(Input::Key KeyCode, char16 UnicodeChar);
    void OnAfterResize();
    void AnalyzeCurrentText();
    void SetSelection(uint32 start, uint32 end);
    void SetTabCharacter(char tabCharacter);
    void SendMsg(Event eventType);
};

struct TabControlContext : public ControlContext
{
  public:
    uint32 TabTitleSize;
    int HoveredTabIndex;

    int MousePositionToPanel(int x, int y);
    void UpdateMargins();
    void PaintTopBottomPanelTab(Graphics::Renderer& renderer, bool onTop);
    void PaintLeftPanelTab(Graphics::Renderer& renderer);
    void PaintListPanelTab(Graphics::Renderer& renderer);
    void PaintNoTabsPanelTab(Graphics::Renderer& renderer);
};
struct CanvasControlContext : public ControlContext
{
    Graphics::Canvas canvas;
    int CanvasScrollX, CanvasScrollY, mouseDragX, mouseDragY;
    bool dragModeEnabled;
    void MoveScrollTo(int newX, int newY);
};

constexpr uint32 MAX_LISTVIEW_COLUMNS     = 64;
constexpr uint32 MAX_LISTVIEW_HEADER_TEXT = 32;

struct ListViewItem
{
    CharacterBuffer SubItem[MAX_LISTVIEW_COLUMNS];
    ListViewItemType Type;
    uint16 Flags;
    uint32 XOffset;
    uint32 Height;
    ColorPair ItemColor;
    variant<GenericRef, uint64> Data;
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
    uint32 HotKeyOffset;
    uint32 Flags;
    Key HotKeyCode;
    uint16 Width;
    TextAlignament Align;

    void Reset();
    bool SetName(const ConstString& text);
    bool SetAlign(TextAlignament align);
    void SetWidth(uint32 width);
};

class ListViewControlContext : public ControlContext
{
  public:
    struct
    {
        ListViewColumn List[MAX_LISTVIEW_COLUMNS];
        uint32 Count;
        uint32 TotalWidth;
        uint32 ResizeColumnIndex;
        uint32 HoverColumnIndex;
        uint32 HoverSeparatorColumnIndex;
        int XOffset;
        bool ResizeModeEnabled;
    } Columns;

    struct
    {
        uint32 ColumnIndex;
        bool Ascendent;
        Handlers::ListViewItemComparer CompareCallbak;
        void* CompareCallbakContext;
    } SortParams;

    struct
    {
        vector<ListViewItem> List;
        Utils::Array32 Indexes;
        int FirstVisibleIndex, CurentItemIndex;
    } Items;

    struct
    {
        Utils::UnicodeStringBuilder SearchText;
        int LastFoundItem;
        bool FilterModeEnabled;
    } Filter;

    struct
    {
        char Status[20];
        uint32 StatusLength;
    } Selection;
    char clipboardSeparator;

    Controls::Control* Host;

    ListViewItem* GetFilteredItem(uint32 index);

    int SearchItem(uint32 startPoz);
    void UpdateSearch(int startPoz);
    void UpdateSelectionInfo();
    void DrawColumnSeparatorsForResizeMode(Graphics::Renderer& renderer);
    void DrawColumn(Graphics::Renderer& renderer);
    void DrawItem(Graphics::Renderer& renderer, ListViewItem* item, int y, bool currentItem);

    // movement
    void UpdateSelection(int start, int end, bool select);
    void MoveTo(int newItem);
    void ColumnSort(uint32 columnIndex);

    // columns
    void UpdateColumnsWidth();
    bool AddColumn(const ConstString& text, TextAlignament Align, uint32 width = 10);
    bool DeleteColumn(uint32 index);
    void DeleteAllColumns();
    int GetNrColumns();

    // itemuri
    ItemHandle AddItem(const ConstString& text);
    bool SetItemText(ItemHandle item, uint32 subItem, const ConstString& text);
    Graphics::CharacterBuffer* GetItemText(ItemHandle item, uint32 subItem);
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
    uint32 GetCheckedItemsCount();

    void DeleteAllItems();
    bool SetColumnClipboardCopyState(uint32 columnIndex, bool allowCopy);
    bool SetColumnFilterMode(uint32 columnIndex, bool allowFilterForThisColumn);

    bool SetCurrentIndex(ItemHandle item);
    int GetFirstVisibleLine();
    bool SetFirstVisibleLine(ItemHandle item);
    int GetVisibleItemsCount();

    bool SetItemDataAsPointer(ItemHandle item, GenericRef Data);
    GenericRef GetItemDataAsPointer(const ItemHandle item) const;
    bool SetItemDataAsValue(ItemHandle item, uint64 value);
    bool GetItemDataAsValue(const ItemHandle item, uint64& value) const;
    bool SetItemXOffset(ItemHandle item, uint32 XOffset);
    uint32 GetItemXOffset(ItemHandle item);
    bool SetItemHeight(ItemHandle item, uint32 Height);
    uint32 GetItemHeight(ItemHandle item);

    void Paint(Graphics::Renderer& renderer);
    void OnMouseReleased(int x, int y, Input::MouseButton button);
    bool MouseToHeader(int x, int y, uint32& HeaderIndex, uint32& HeaderColumnIndex);
    void OnMousePressed(int x, int y, Input::MouseButton button);
    bool OnMouseDrag(int x, int y, Input::MouseButton button);
    bool OnMouseWheel(int x, int y, Input::MouseWheel direction);
    bool OnMouseOver(int x, int y);
    void SetSortColumn(uint32 colIndex);
    bool OnKeyEvent(Input::Key keyCode, char16 UnicodeChar);
    void SendMsg(Event eventType);
    bool Sort();

    bool FilterItem(ListViewItem& lvi, bool clearColorForAll);
    void FilterItems();
};

struct ComboBoxItem
{
    Graphics::CharacterBuffer Text;
    variant<GenericRef, uint64> Data;
    uint32 Index;
    bool Separator;
    ComboBoxItem();
    ComboBoxItem(
          const ConstString& caption, variant<GenericRef, uint64> userData, uint32 index, bool separator = false);
    ~ComboBoxItem();
    ComboBoxItem(const ComboBoxItem&);
    ComboBoxItem(ComboBoxItem&&) noexcept;
    ComboBoxItem& operator=(const ComboBoxItem&);
    ComboBoxItem& operator=(ComboBoxItem&&) noexcept;
};
class ComboBoxControlContext : public ControlContext
{
  public:
    vector<ComboBoxItem> Items;
    Utils::Array32 Indexes;
    uint32 ExpandedHeight, FirstVisibleItem, VisibleItemsCount, HoveredIndexItem;
    uint32 CurentItemIndex;
};

class NumericSelectorControlContext : public ControlContext
{
  public:
    int64 minValue;
    int64 maxValue;
    int64 value;

    const int32 buttonPadding = 4;
    LocalString<256> stringValue;
    bool intoInsertionMode       = false;
    bool wasMinusPressed         = false;
    int64 insertionModevalue     = 0;
    bool wrongValueInserted      = false;
    int64 sliderPosition         = 0;
    bool isMouseLeftClickPressed = false;

    NumericSelector* instance = nullptr;

    enum class IsMouseOn
    {
        None,
        MinusButton,
        TextField,
        PlusButton
    } isMouseOn{ IsMouseOn::None };

    bool IsValidValue(int64 value) const;
    bool IsValueInsertedWrong() const;
    bool GetRenderColor(Graphics::ColorPair& color) const;
    bool FormatTextField();
    bool IsOnPlusButton(int32 x, int32 y) const;
    bool IsOnMinusButton(int32 x, int32 y) const;
    bool IsOnTextField(int32 x, int32 y) const;
    bool MinValueReached() const;
    bool MaxValueReached() const;
};

struct TreeColumnData
{
    uint32 x      = 0;
    uint32 width  = 0;
    uint32 height = 0;
    CharacterBuffer headerValue;
    TextAlignament headerAlignment  = TextAlignament::Left;
    TextAlignament contentAlignment = TextAlignament::Left;
    bool customWidth                = false;
};

struct TreeItem
{
    ItemHandle parent{ InvalidItemHandle };
    ItemHandle handle{ InvalidItemHandle };
    vector<CharacterBuffer> values;
    variant<GenericRef, uint64> data{ nullptr };
    bool expanded     = false;
    bool isExpandable = false;
    vector<ItemHandle> children;
    Utils::UnicodeStringBuilder metadata;
    uint32 depth                      = 1;
    bool markedAsFound                = false;
    bool hasAChildThatIsMarkedAsFound = false;
};

class TreeControlContext : public ControlContext
{
  public:
    std::map<ItemHandle, TreeItem> items;
    vector<ItemHandle> itemsToDrew;
    vector<ItemHandle> orderedItems;
    ItemHandle nextItemHandle{ 1ULL };
    ItemHandle currentSelectedItemHandle{ InvalidItemHandle };
    uint32 maxItemsToDraw  = 0;
    uint32 offsetTopToDraw = 0;
    uint32 offsetBotToDraw = 0;
    bool notProcessed      = true;
    vector<ItemHandle> roots;
    vector<TreeColumnData> columns;
    uint32 treeFlags              = 0;
    uint32 separatorIndexSelected = 0xFFFFFFFF;
    ItemHandle firstFoundInSearch = InvalidItemHandle;
    bool hidSearchBarOnResize     = false;

    enum class IsMouseOn
    {
        None,
        ToggleSymbol,
        Item,
        Border,
        ColumnSeparator,
        ColumnHeader,
        SearchField
    } isMouseOn{ IsMouseOn::None };

    enum class FilterMode
    {
        None   = 0,
        Search = 1,
        Filter = 2
    };

    struct
    {
        Utils::UnicodeStringBuilder searchText;
        FilterMode mode{ FilterMode::None };
    } filter{};
};

enum class GridCellStatus
{
    Normal   = 0,
    Selected = 1,
    Hovered  = 2
};

struct GridCellData
{
    TextAlignament ta = TextAlignament::Left;
    Grid::CellType ct = Grid::CellType::String;
    variant<bool, std::u16string> content;
};

struct GridHeaderCellData
{
    TextAlignament ta = TextAlignament::Left;
    std::u16string content;
};

class GridControlContext : public ControlContext
{
  public:
    uint32 columnsNo        = 0;
    uint32 rowsNo           = 0;
    GridFlags flags         = GridFlags::None;
    uint32 hoveredCellIndex = 0xFFFFFFFF;
    uint32 anchorCellIndex  = 0xFFFFFFFF;
    std::vector<uint32> selectedCellsIndexes;

    uint32 cWidth  = 0U;
    uint32 cHeight = 0U;
    uint32 offsetX = 0U;
    uint32 offsetY = 0U;

    Menu rightClickMenu;

    std::map<uint32, GridCellData> cells;
    std::u16string separator{ u"," };
    std::vector<GridHeaderCellData> headers;

    uint32 viewportOffsetX = 0;
    uint32 viewportOffsetY = 0;

  public:
    void DrawCellBackground(Graphics::Renderer& renderer, GridCellStatus cellType, uint32 i, uint32 j);
    void DrawCellBackground(Graphics::Renderer& renderer, GridCellStatus cellType, uint32 cellIndex);

  public:
    void DrawBoxes(Graphics::Renderer& renderer);
    void DrawLines(Graphics::Renderer& renderer);
    uint32 ComputeCellNumber(int32 x, int32 y);
    Graphics::SpecialChars ComputeBoxType(
          uint32 colIndex,
          uint32 rowIndex,
          uint32 startColumnsIndex,
          uint32 startRowsIndex,
          uint32 endColumnsIndex,
          uint32 endRowsIndex);
    void DrawCellsBackground(Graphics::Renderer& renderer);
    bool DrawCellContent(Graphics::Renderer& renderer, uint32 cellIndex);
    bool DrawHeader(Graphics::Renderer& renderer);
    void UpdateGridParameters(bool dontRecomputeDimensions = false);
    void UpdateDimensions(int32 offsetX, int32 offsetY);
    bool MoveSelectedCellByKeys(AppCUI::Input::Key keyCode);
    bool SelectCellsByKeys(AppCUI::Input::Key keyCode);
    bool ToggleBooleanCell();
    bool CopySelectedCellsContent() const;
    bool PasteContentToSelectedCells();
};

enum class MenuItemType : uint32
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
    uint32 HotKeyOffset;
    Input::Key HotKey;
    Input::Key ShortcutKey;
    MenuItemType Type;
    int CommandID;
    bool Enabled;
    bool Checked;
    Menu* SubMenu;

    MenuItem(); // line
    MenuItem(
          MenuItemType type,
          const ConstString& text,
          int CommandID,
          bool checked,
          Input::Key shortcutKey);                    // commands
    MenuItem(const ConstString& text, Menu* subMenu); // submenu
    MenuItem(const MenuItem& obj) = delete;
    MenuItem(MenuItem&& obj)      = delete;
    ~MenuItem();
};
enum class MousePressedResult : uint32
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
    uint32 ItemIndex;
    bool IsOnMenu;
    bool IsOnUpButton;
    bool IsOnDownButton;
};

constexpr uint32 MAX_NUMBER_OF_MENU_ITEMS = 256;
struct MenuContext
{
    // vector messes up with inter-items pointers when calling copy/move ctor
    // as a result, opening a sub-menu from another is likely to produce a crash (as the pointers will be invalid)
    // switching to regular pointer of unique_ptr type to avoid this

    unique_ptr<MenuItem> Items[MAX_NUMBER_OF_MENU_ITEMS];
    uint32 ItemsCount;
    Menu* Parent;
    Internal::MenuBar* Owner;
    Graphics::Clip ScreenClip;
    Application::Config* Cfg;
    uint32 FirstVisibleItem;
    uint32 VisibleItemsCount;
    uint32 CurrentItem;
    uint32 Width, TextWidth;
    MenuButtonState ButtonUp, ButtonDown;

    MenuContext();
    ItemHandle AddItem(unique_ptr<MenuItem> itm);

  public:
    // methods
    void Paint(Graphics::Renderer& renderer, bool activ);

    void RunItemAction(uint32 index);
    void CloseMenu();

    // Move
    void UpdateFirstVisibleItem();
    void CreateAvailableItemsList(uint32* indexes, uint32& count);
    void MoveCurrentItemTo(Input::Key keyCode);

    // Check
    bool SetChecked(uint32 index, bool status);

    // mouse events
    void ComputeMousePositionInfo(int x, int y, MenuMousePositionInfo& mpi);
    bool OnMouseMove(int x, int y, bool& repaint);
    MousePressedResult OnMousePressed(int x, int y);
    bool IsOnMenu(int x, int y);
    bool OnMouseWheel(int x, int y, Input::MouseWheel direction);

    // key events
    bool OnKeyEvent(Input::Key keyCode);
    bool ProcessShortCut(Input::Key keyCode);

    // Show
    void Show(
          Controls::Menu* me,
          Reference<Controls::Control> relativeControl,
          int x,
          int y,
          const Graphics::Size& maxSize);
};

struct PropertyCategoryInfo
{
    FixSizeString<61> name;
    uint32 totalItems;
    uint32 filteredItems;
    bool folded;
};
struct PropertyInfo
{
    FixSizeString<61> name;
    std::map<uint64, FixSizeUnicode<48>> listValues;
    uint32 category;
    uint32 id;
    PropertyType type;
};
enum class PropertySeparatorStatus : uint8
{
    None,
    Over,
    Drag
};
struct PropertyListContext : public ControlContext
{
    struct
    {
        decltype(AppCUI::Application::Config::PropertyList.Category) Category;
        decltype(AppCUI::Application::Config::PropertyList.Item) Item;
    } Colors;
    PropertyValue tempPropValue;
    Reference<PropertiesInterface> object;
    vector<PropertyInfo> properties;
    vector<PropertyCategoryInfo> categories;
    Array32 items;
    FixSizeString<61> filterText;
    uint32 startView;
    uint32 currentPos;
    int32 propertyNameWidth;
    float propetyNamePercentage;
    bool showCategories;
    bool hasBorder;
    bool filteredMode;
    PropertySeparatorStatus separatorStatus;

    void ExecuteItemAction();
    void SetPropertyNameWidth(int32 value, bool adjustPercentage);
    void MoveToPropetyIndex(uint32 idx);
    void MoveTo(uint32 newPos);
    void MoveScrollTo(uint32 newPos);
    void DrawCategory(uint32 index, int32 y, Graphics::Renderer& renderer);
    void DrawListProperty(
          WriteTextParams& params,
          PropertyValue& pv,
          const PropertyInfo& pi,
          Graphics::Renderer& renderer,
          bool readOnly);
    void DrawFlagsProperty(
          WriteTextParams& params,
          PropertyValue& pv,
          const PropertyInfo& pi,
          Graphics::Renderer& renderer,
          bool readOnly);
    void DrawCustomProperty(WriteTextParams& params, PropertyValue& pv, Graphics::Renderer& renderer, bool readOnly);
    void DrawProperty(uint32 index, int32 y, Graphics::Renderer& renderer, bool& readOnlyStatus);
    void DrawFilterBar(Graphics::Renderer& renderer);
    void Paint(Graphics::Renderer& renderer);
    bool ProcessFilterKey(Input::Key keyCode, char16 UnicodeChar);
    bool OnKeyEvent(Input::Key keyCode, char16 UnicodeChar);
    void OnMousePressed(int x, int y, Input::MouseButton button);
    bool OnMouseWheel(int x, int y, Input::MouseWheel direction);
    bool OnMouseDrag(int x, int y, Input::MouseButton button);
    bool OnMouseOver(int x, int y);
    bool OnMouseLeave();
    void OnMouseReleased(int x, int y, Input::MouseButton button);
    bool IsItemFiltered(const PropertyInfo& prop);
    void Refilter();
    void EditAndUpdateText(const PropertyInfo& prop);
    void EditAndUpdateBool(const PropertyInfo& prop);
    void EditAndUpdateList(const PropertyInfo& prop);
    void EditAndUpdateFlags(const PropertyInfo& prop);
    void EditAndUpdateKey(const PropertyInfo& prop);

    inline constexpr int32 GetSeparatorXPos() const
    {
        return (this->showCategories ? 3 : 0) + (this->hasBorder ? 1 : 0) + this->propertyNameWidth;
    }
    inline constexpr bool IsPropertyReadOnly(const PropertyInfo& prop) 
    {
        return ((this->Flags && PropertyListFlags::ReadOnly) ? true : this->object->IsPropertyValueReadOnly(prop.id));
    }
};
struct KeySelectorContext : public ControlContext
{
    Input::Key key;
};
struct ColorPickerContext : public ControlContext
{
    Graphics::Color color;
    int32 headerYOffset;
    int32 yOffset;
    uint32 colorObject;

    void PaintColorBox(Graphics::Renderer& renderer);
    void PaintHeader(int x, int y, uint32 width, Graphics::Renderer& renderer);
    void Paint(Graphics::Renderer& renderer);
    uint32 MouseToObject(int x, int y);
    void OnMousePressed(int x, int y, Input::MouseButton button);
    bool OnMouseOver(int x, int y);
    void NextColor(int32 offset, bool isExpanded);
    bool OnKeyEvent(Input::Key keyCode);

    void OnExpandView(Graphics::Clip& expandedClip);
};
#define CREATE_CONTROL_CONTEXT(object, name, retValue)                                                                 \
    ControlContext* name = (ControlContext*) ((object)->Context);                                                      \
    if (name == nullptr)                                                                                               \
        return retValue;
#define GET_CONTROL_HANDLERS(type)                                                                                     \
    if (!this->Context)                                                                                                \
        return nullptr;                                                                                                \
    if (!((ControlContext*) (this->Context))->handlers)                                                                \
        ((ControlContext*) (this->Context))->handlers = std::make_unique<type>();                                      \
    return (type*) ((((ControlContext*) (this->Context))->handlers).get());

#define CREATE_TYPECONTROL_CONTEXT(type, name, retValue)                                                               \
    type* name = (type*) ((this)->Context);                                                                            \
    if (name == nullptr)                                                                                               \
        return retValue;
#define CREATE_TYPE_CONTEXT(type, object, name, retValue)                                                              \
    type* name = (type*) ((object)->Context);                                                                          \
    if (name == nullptr)                                                                                               \
        return retValue;
#define INIT_CONTROL(type, contextType)                                                                                \
    auto me           = std::make_unique<type>();                                                                      \
    auto Members      = new contextType();                                                                             \
    me.get()->Context = Members;

#define DELETE_CONTROL_CONTEXT(type)                                                                                   \
    if (Context != nullptr)                                                                                            \
    {                                                                                                                  \
        type* c = (type*) Context;                                                                                     \
        delete c;                                                                                                      \
        Context = nullptr;                                                                                             \
    }
} // namespace AppCUI
