#include "ControlContext.hpp"

using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;
using namespace AppCUI::Input;

bool CanvasViewer::Create(
      Control* parent, const std::string_view& layout, unsigned int canvasWidth, unsigned int canvasHeight, ViewerFlags flags)
{
    return this->Create(parent, "", layout, canvasWidth, canvasHeight, flags);
}
bool CanvasViewer::Create(
      Control* parent,
      const AppCUI::Utils::ConstString& caption,
      const std::string_view& layout,
      unsigned int canvasWidth,
      unsigned int canvasHeight,
      ViewerFlags flags)
{
    CHECK(canvasWidth > 0, false, "Canvas Width must be greater than 0.");
    CHECK(canvasHeight > 0, false, "Canvas Height must be greater than 0.");
    CONTROL_INIT_CONTEXT(CanvasControlContext);
    CHECK(Init(parent, caption, layout, true), false, "Failed to create Canvas viewer object");
    CREATE_TYPECONTROL_CONTEXT(CanvasControlContext, Members, false);
    Members->Flags =
          GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | GATTR_VSCROLL | GATTR_HSCROLL | ((unsigned int) flags);
    Members->CanvasScrollX             = 0;
    Members->CanvasScrollY             = 0;
    Members->ScrollBars.OutsideControl = ((((unsigned int) flags) & ((unsigned int) ViewerFlags::Border)) == 0);
    CHECK(Members->canvas.Create(canvasWidth, canvasHeight),
          false,
          "Fail to create a canvas of size %d x %d",
          canvasWidth,
          canvasHeight);
    return true;
}

CanvasViewer::~CanvasViewer()
{
    DELETE_CONTROL_CONTEXT(CanvasControlContext);
}

void CanvasViewer::Paint(Graphics::Renderer& renderer)
{
    CREATE_TYPECONTROL_CONTEXT(CanvasControlContext, Members, );
    auto* col = &Members->Cfg->View.Normal;
    if (!this->IsEnabled())
        col = &Members->Cfg->View.Inactive;
    else if (Members->Focused)
        col = &Members->Cfg->View.Focused;
    else if (Members->MouseIsOver)
        col = &Members->Cfg->View.Hover;

    if (Members->Flags & ((unsigned int) ViewerFlags::Border))
    {
        renderer.DrawRectSize(0, 0, Members->Layout.Width, Members->Layout.Height, col->Border, false);
        if (Members->Layout.Width > 6)
        {
            WriteTextParams params(
                  WriteTextFlags::SingleLine | WriteTextFlags::HighlightHotKey | WriteTextFlags::ClipToWidth |
                  WriteTextFlags::OverwriteColors | WriteTextFlags::LeftMargin | WriteTextFlags::RightMargin);
            params.X              = 3;
            params.Y              = 0;
            params.Color          = col->Text;
            params.HotKeyColor    = col->Hotkey;
            params.HotKeyPosition = Members->HotKeyOffset;
            params.Width          = Members->Layout.Width - 6;
            params.Align          = TextAlignament::Left;
            renderer.WriteText(Members->Text, params);
        }
        if (!renderer.SetClipMargins(1, 1, 1, 1))
            return; // clipping is not visible --> no need to try to draw the rest
    }
    if (!this->IsEnabled())
        renderer.DrawCanvas(
              Members->CanvasScrollX, Members->CanvasScrollY, Members->canvas, Members->Cfg->View.InactiveCanvasColor);
    else
        renderer.DrawCanvas(Members->CanvasScrollX, Members->CanvasScrollY, Members->canvas);
}

void CanvasViewer::OnUpdateScrollBars()
{
    CREATE_TYPECONTROL_CONTEXT(CanvasControlContext, Members, );
    UpdateVScrollBar(-Members->CanvasScrollY, Members->canvas.GetHeight());
    UpdateHScrollBar(-Members->CanvasScrollX, Members->canvas.GetWidth());
}

bool CanvasViewer::OnKeyEvent(AppCUI::Input::Key KeyCode, char AsciiCode)
{
    CREATE_TYPECONTROL_CONTEXT(CanvasControlContext, Members, false);

    switch (KeyCode)
    {
    case Key::Down:
        Members->CanvasScrollY--;
        if (Members->CanvasScrollY < -((int) (Members->canvas.GetHeight())))
            Members->CanvasScrollY = -((int) (Members->canvas.GetHeight()));
        return true;
    case Key::Up:
        Members->CanvasScrollY = MINVALUE(Members->CanvasScrollY + 1, 0);
        return true;
    case Key::Right:
        Members->CanvasScrollX--;
        if (Members->CanvasScrollX < -((int) (Members->canvas.GetWidth())))
            Members->CanvasScrollX = -((int) (Members->canvas.GetWidth()));
        return true;
    case Key::Left:
        Members->CanvasScrollX = MINVALUE(Members->CanvasScrollX + 1, 0);
        return true;
    default:
        break;
    }
    return false;
}
bool AppCUI::Controls::CanvasViewer::OnMouseWheel(int x, int y, AppCUI::Input::MouseWheel direction)
{
    switch (direction)
    {
        case AppCUI::Input::MouseWheel::Up:
            return OnKeyEvent(Key::Up, 0);
        case AppCUI::Input::MouseWheel::Down:
            return OnKeyEvent(Key::Down, 0);
        case AppCUI::Input::MouseWheel::Left:
            return OnKeyEvent(Key::Left, 0);
        case AppCUI::Input::MouseWheel::Right:
            return OnKeyEvent(Key::Right, 0);
    }
    return false;
}
bool AppCUI::Controls::CanvasViewer::OnMouseEnter()
{
    return true;
}
bool AppCUI::Controls::CanvasViewer::OnMouseLeave()
{
    return true;
}

Canvas* CanvasViewer::GetCanvas()
{
    CREATE_TYPECONTROL_CONTEXT(CanvasControlContext, Members, nullptr);
    return &(Members->canvas);
}
