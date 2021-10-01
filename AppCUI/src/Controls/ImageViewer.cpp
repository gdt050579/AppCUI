#include <AppCUI.hpp>

using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;

std::unique_ptr<ImageViewer> ImageViewer::Create(
      const AppCUI::Utils::ConstString& caption, const std::string_view& layout, ViewerFlags flags)
{
    auto obj = std::make_unique<ImageViewer>();
    CHECK(obj->Init(caption, layout, 16U, 16U, flags), nullptr, "");
    return obj;
}
std::unique_ptr<ImageViewer> ImageViewer::Create(
      const std::string_view& layout, ViewerFlags flags)
{
    return ImageViewer::Create("", layout, flags);
}
ImageViewer* ImageViewer::Create(
      Control& parent, const AppCUI::Utils::ConstString& caption, const std::string_view& layout, ViewerFlags flags)
{
    auto me = ImageViewer::Create(caption, layout, flags);
    CHECK(me, nullptr, "Fail to create a ImageViewer control !");
    return parent.AddControl<ImageViewer>(std::move(me));
}
ImageViewer* ImageViewer::Create(
      Control& parent, const std::string_view& layout, ViewerFlags flags)
{
    auto me = ImageViewer::Create(layout, flags);
    CHECK(me, nullptr, "Fail to create a ImageViewer control !");
    return parent.AddControl<ImageViewer>(std::move(me));
}
bool ImageViewer::SetImage(
      const AppCUI::Graphics::Image& img,
      AppCUI::Graphics::ImageRenderingMethod method,
      AppCUI::Graphics::ImageScaleMethod scale)
{
    auto* c = GetCanvas();
    CHECK(c, false, "");
    auto sz = c->ComputeRenderingSize(img, method, scale);
    CHECK((sz.Width > 0) && (sz.Height > 0), false, "Invalid image size (0x0) --> nothing to render");
    CHECK(c->Resize(sz.Width, sz.Height), false, "Fail to set canvas size to %ux%u", sz.Width, sz.Height);
    return c->DrawImage(img, 0, 0, method, scale);
}