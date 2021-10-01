#include "ControlContext.hpp"

using namespace AppCUI::Controls;

std::unique_ptr<UserControl> UserControl::Create(
      const AppCUI::Utils::ConstString& caption, const std::string_view& layout)
{
    INIT_CONTROL(UserControl, ControlContext);
    CHECK(me->Init(caption, layout, false), nullptr, "Failed to create user control !");
    Members->Flags = GATTR_VISIBLE | GATTR_ENABLE | GATTR_TABSTOP;
    return me;
}
std::unique_ptr<UserControl> UserControl::Create(const std::string_view& layout)
{
    return UserControl::Create("", layout);
}
UserControl* UserControl::Create(
      Control& parent, const AppCUI::Utils::ConstString& caption, const std::string_view& layout)
{
    auto me = UserControl::Create(caption, layout);
    CHECK(me, nullptr, "Fail to create a UserControl control !");
    return parent.AddControl<UserControl>(std::move(me));
}
UserControl* UserControl::Create(Control& parent, const std::string_view& layout)
{
    auto me = UserControl::Create(layout);
    CHECK(me, nullptr, "Fail to create a UserControl control !");
    return parent.AddControl<UserControl>(std::move(me));
}