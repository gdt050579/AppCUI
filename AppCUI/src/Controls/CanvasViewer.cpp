#include "ControlContext.hpp"

namespace AppCUI
{
void CanvasControlContext::MoveScrollTo(int newX, int newY)
{
    const int imgWidth  = canvas.GetWidth();
    const int imgHeight = canvas.GetHeight();
    int viewWidth       = Layout.Width;
    int viewHeight      = Layout.Height;

    if (this->Flags && ViewerFlags::Border)
    {
        viewWidth++;
        viewHeight++;
    }

    if ((newX + imgWidth) < viewWidth)
        newX = viewWidth - imgWidth;
    if ((newY + imgHeight) < viewHeight)
        newY = viewHeight - imgHeight;

    newX = std::min<>(newX, 0);
    newY = std::min<>(newY, 0);

    this->CanvasScrollX = newX;
    this->CanvasScrollY = newY;
}

CanvasViewer::CanvasViewer(
      const ConstString& caption, string_view layout, uint32 canvasWidth, uint32 canvasHeight, ViewerFlags flags)
    : Control(new CanvasControlContext(), caption, layout, true)
{
    auto Members = reinterpret_cast<CanvasControlContext*>(this->Context);

    Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | ((uint32) flags);
    if (!(Members->Flags && ViewerFlags::HideScrollBar))
    {
        Members->Flags |= (GATTR_VSCROLL | GATTR_HSCROLL);
    }
    Members->CanvasScrollX             = 0;
    Members->CanvasScrollY             = 0;
    Members->mouseDragX                = 0;
    Members->mouseDragY                = 0;
    Members->dragModeEnabled           = false;
    Members->ScrollBars.OutsideControl = !(Members->Flags && ViewerFlags::Border);
    ASSERT(Members->canvas.Create(canvasWidth, canvasHeight), "Fail to create a canvas of size object !");
}

CanvasViewer::~CanvasViewer()
{
    DELETE_CONTROL_CONTEXT(CanvasControlContext);
}

void CanvasViewer::Paint(Graphics::Renderer& renderer)
{
    CREATE_TYPECONTROL_CONTEXT(CanvasControlContext, Members, );
    auto left = 0;
    auto top  = 0;

    if (Members->Flags && ViewerFlags::Border)
    {
        renderer.DrawRectSize(
              0,
              0,
              Members->Layout.Width,
              Members->Layout.Height,
              Members->Cfg->Border.GetColor(Members->GetControlState(ControlStateFlags::ProcessHoverStatus)),
              LineType::Single);
        if (Members->Layout.Width > 6)
        {
            WriteTextParams params(
                  WriteTextFlags::SingleLine | WriteTextFlags::HighlightHotKey | WriteTextFlags::ClipToWidth |
                  WriteTextFlags::OverwriteColors | WriteTextFlags::LeftMargin | WriteTextFlags::RightMargin);
            const auto enabled    = (Members->Flags & GATTR_ENABLE) != 0;
            params.X              = 3;
            params.Y              = 0;
            params.Color          = enabled ? Members->Cfg->Text.Normal : Members->Cfg->Text.Inactive;
            params.HotKeyColor    = enabled ? Members->Cfg->Text.Normal : Members->Cfg->Text.Inactive;
            params.HotKeyPosition = Members->HotKeyOffset;
            params.Width          = Members->Layout.Width - 6;
            params.Align          = TextAlignament::Left;
            renderer.WriteText(Members->Text, params);
        }
        if (!renderer.SetClipMargins(1, 1, 1, 1))
            return; // clipping is not visible --> no need to try to draw the rest
        left = top = 1;
    }
    if (!this->IsEnabled())
        renderer.DrawCanvas(
              Members->CanvasScrollX + left,
              Members->CanvasScrollY + top,
              Members->canvas,
              Members->Cfg->Text.Inactive);
    else
        renderer.DrawCanvas(Members->CanvasScrollX + left, Members->CanvasScrollY + top, Members->canvas);
}

void CanvasViewer::OnUpdateScrollBars()
{
    CREATE_TYPECONTROL_CONTEXT(CanvasControlContext, Members, );

    // horizontal
    if (Members->canvas.GetHeight() > (uint32) Members->Layout.Height)
        UpdateVScrollBar(-Members->CanvasScrollY, Members->canvas.GetHeight() - (uint32) Members->Layout.Height);
    else
        UpdateVScrollBar(-Members->CanvasScrollY, 0);

    // vertical
    if (Members->canvas.GetWidth() > (uint32) Members->Layout.Width)
        UpdateHScrollBar(-Members->CanvasScrollX, Members->canvas.GetWidth() - (uint32) Members->Layout.Width);
    else
        UpdateHScrollBar(-Members->CanvasScrollX, 0);
}

bool CanvasViewer::OnKeyEvent(Input::Key KeyCode, char16)
{
    CREATE_TYPECONTROL_CONTEXT(CanvasControlContext, Members, false);

    switch (KeyCode)
    {
    case Key::Down:
        Members->MoveScrollTo(Members->CanvasScrollX, Members->CanvasScrollY - 1);
        return true;
    case Key::Up:
        Members->MoveScrollTo(Members->CanvasScrollX, Members->CanvasScrollY + 1);
        return true;
    case Key::Right:
        Members->MoveScrollTo(Members->CanvasScrollX - 1, Members->CanvasScrollY);
        return true;
    case Key::Left:
        Members->MoveScrollTo(Members->CanvasScrollX + 1, Members->CanvasScrollY);
        return true;

    case Key::Ctrl | Key::Left:
        Members->MoveScrollTo(0, Members->CanvasScrollY);
        return true;
    case Key::Ctrl | Key::Right:
        Members->MoveScrollTo(-((int) Members->canvas.GetWidth()), Members->CanvasScrollY);
        return true;
    case Key::Ctrl | Key::Up:
        Members->MoveScrollTo(Members->CanvasScrollX, 0);
        return true;
    case Key::Ctrl | Key::Down:
        Members->MoveScrollTo(Members->CanvasScrollX, -((int) Members->canvas.GetHeight()));
        return true;

    case Key::Shift | Key::Left:
        Members->MoveScrollTo(Members->CanvasScrollX + 20, Members->CanvasScrollY);
        return true;
    case Key::Shift | Key::Right:
        Members->MoveScrollTo(Members->CanvasScrollX - 20, Members->CanvasScrollY);
        return true;
    case Key::Shift | Key::Up:
        Members->MoveScrollTo(Members->CanvasScrollX, Members->CanvasScrollY + 10);
        return true;
    case Key::Shift | Key::Down:
        Members->MoveScrollTo(Members->CanvasScrollX, Members->CanvasScrollY - 10);
        return true;

    case Key::Home:
        Members->MoveScrollTo(0, 0);
        return true;
    case Key::End:
        Members->MoveScrollTo(-((int) Members->canvas.GetWidth()), -((int) Members->canvas.GetHeight()));
        return true;
    default:
        break;
    }
    return false;
}
bool Controls::CanvasViewer::OnMouseWheel(int, int, Input::MouseWheel direction)
{
    switch (direction)
    {
    case Input::MouseWheel::Up:
        return OnKeyEvent(Key::Up, 0);
    case Input::MouseWheel::Down:
        return OnKeyEvent(Key::Down, 0);
    case Input::MouseWheel::Left:
        return OnKeyEvent(Key::Left, 0);
    case Input::MouseWheel::Right:
        return OnKeyEvent(Key::Right, 0);
    }
    return false;
}
bool Controls::CanvasViewer::OnMouseEnter()
{
    return true;
}
bool Controls::CanvasViewer::OnMouseLeave()
{
    return true;
}
void Controls::CanvasViewer::OnMousePressed(int x, int y, Input::MouseButton)
{
    CREATE_TYPECONTROL_CONTEXT(CanvasControlContext, Members, );
    Members->mouseDragX      = Members->CanvasScrollX - x;
    Members->mouseDragY      = Members->CanvasScrollY - y;
    Members->dragModeEnabled = true;
}
bool Controls::CanvasViewer::OnMouseDrag(int x, int y, Input::MouseButton)
{
    CREATE_TYPECONTROL_CONTEXT(CanvasControlContext, Members, false);
    if (!Members->dragModeEnabled)
        return false; // don't process
    Members->MoveScrollTo(Members->mouseDragX + x, Members->mouseDragY + y);
    return true;
}
void Controls::CanvasViewer::OnMouseReleased(int x, int y, Input::MouseButton button)
{
    // last update
    OnMouseDrag(x, y, button);
    CREATE_TYPECONTROL_CONTEXT(CanvasControlContext, Members, );
    Members->dragModeEnabled = false;
}
Reference<Canvas> CanvasViewer::GetCanvas()
{
    CREATE_TYPECONTROL_CONTEXT(CanvasControlContext, Members, nullptr);
    return Reference<Canvas>(&(Members->canvas));
}
} // namespace AppCUI
