#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;

enum class MyControlProperty : uint32
{
    X,
    Y,
    Size,
    ForeColor,
    BackColor,
    Character,
    Name,
    Version,
    Border,
    BorderType,
    AnimationStarted,
    AnimationSpeed,
    Flags,
    Custom
};

class MyUserControl : public UserControl, public PropertiesInterface
{
    int32 x, y, addX, addY;
    Size sz;
    char16 ch;
    ColorPair c;
    bool hasBorder, animationStarted;
    uint32 frameDelay;
    uint32 counter;

  public:
    MyUserControl() : UserControl("d:c")
    {
        counter          = 0;
        frameDelay       = 3;
        x                = 2;
        y                = 3;
        addX             = 1;
        addY             = -1;
        sz               = { 15, 5 };
        ch               = 'X';
        c                = ColorPair{ Color::Red, Color::Black };
        hasBorder        = false;
        animationStarted = true;
    }
    bool OnFrameUpdate() override
    {
        counter++;
        if (counter < frameDelay)
            return false;
        counter = 0;
        x += addX;
        y += addY;
        if (x < 0)
        {
            x    = 0;
            addX = 1;
        }
        if (x > this->GetWidth())
        {
            x    = this->GetWidth();
            addX = -1;
        }
        if (y < 0)
        {
            y    = 0;
            addY = 1;
        }
        if (y > this->GetHeight())
        {
            y    = this->GetHeight();
            addY = -1;
        }
        return true;
    }
    void Paint(Graphics::Renderer& renderer) override
    {
        renderer.Clear(' ', ColorPair{ Color::White, Color::Black });
        renderer.FillRect(x, y, x + sz.Width - 1, y + sz.Height - 1, ch, c);
    }
    // PropertiesInterface
    bool GetPropertyValue(uint32 id, PropertyValue& value) override
    {
        switch (static_cast<MyControlProperty>(id))
        {
        case MyControlProperty::X:
            value = this->x;
            return true;
        case MyControlProperty::Y:
            value = this->y;
            return true;
        case MyControlProperty::Size:
            value = this->sz;
            return true;
        case MyControlProperty::ForeColor:
            value = this->c.Foreground;
            return true;
        case MyControlProperty::BackColor:
            value = this->c.Background;
            return true;
        case MyControlProperty::Name:
            value = u"Dragoș";
            return true;
        case MyControlProperty::Version:
            value = "1.2.3";
            return true;
        case MyControlProperty::Character:
            value = ch;
            return true;
        case MyControlProperty::Border:
            value = this->hasBorder;
            return true;
        case MyControlProperty::BorderType:
            value = 2;
            return true;
        case MyControlProperty::AnimationStarted:
            value = animationStarted;
            return true;
        case MyControlProperty::AnimationSpeed:
            value = frameDelay;
            return true;
        case MyControlProperty::Flags:
            value = 59;
            return true;
        case MyControlProperty::Custom:
            value = "Custom string representation";
            return true;
        }
        return false;
    };
    void SetPropertyValue(uint32 id, const PropertyValue& value, String& error) override
    {
        Size tmpSz;
        switch (static_cast<MyControlProperty>(id))
        {
        case MyControlProperty::X:
            this->x = std::get<int32>(value);
            break;
        case MyControlProperty::Y:
            this->y = std::get<int32>(value);
            break;
        case MyControlProperty::Size:
            tmpSz = std::get<Size>(value);
            if ((tmpSz.Width == 0) || (tmpSz.Height == 0))
                error.Format(
                      "Invalid size (%u x %u) --> Width/Height must be bigger than 0", tmpSz.Width, tmpSz.Height);
            else
                this->sz = tmpSz;
            break;
        case MyControlProperty::ForeColor:
            this->c.Foreground = std::get<Color>(value);
            break;
        case MyControlProperty::BackColor:
            this->c.Background = std::get<Color>(value);
            break;
        case MyControlProperty::Name:
            // do nothing ==> read-only
            break;
        case MyControlProperty::Version:
            // do nothing ==> read-only
            break;
        case MyControlProperty::Border:
            this->hasBorder = std::get<bool>(value);
            break;
        case MyControlProperty::BorderType:
            // value = 2;
            break;

        }
    };
    void SetCustomPropetyValue(uint32 propertyID) override
    {
    }
    bool IsPropertyValueReadOnly(uint32 propertyID) override
    {
        return (propertyID == (uint32) MyControlProperty::Name) || (propertyID == (uint32) MyControlProperty::Version);
    }
    vector<Property> GetPropertiesList() override
    {
        return vector<Property>({
              { (uint32) MyControlProperty::X, "Layout", "X", PropertyType::Int32 },
              { (uint32) MyControlProperty::Y, "Layout", "Y", PropertyType::Int32 },
              { (uint32) MyControlProperty::Size, "Layout", "Size", PropertyType::Size },
              { (uint32) MyControlProperty::ForeColor, "Look & Feel", "Fore color", PropertyType::Color },
              { (uint32) MyControlProperty::BackColor, "Look & Feel", "Back color", PropertyType::Color },
              { (uint32) MyControlProperty::Character, "Look & Feel", "Character", PropertyType::Char16 },
              { (uint32) MyControlProperty::Border, "Look & Feel", "Draw border", PropertyType::Boolean },
              { (uint32) MyControlProperty::BorderType,
                "Look & Feel",
                "Border Type",
                PropertyType::List,
                "Single=1,Double=2,Thick=3" },
              { (uint32) MyControlProperty::Name, "General", "Name", PropertyType::Unicode },
              { (uint32) MyControlProperty::Version, "General", "Version", PropertyType::Ascii },
              { (uint32) MyControlProperty::AnimationStarted, "Animation", "Started", PropertyType::Boolean },
              { (uint32) MyControlProperty::AnimationSpeed,
                "Animation",
                "Speed",
                PropertyType::List,
                "  Very Slow   = 5,Slow=4,Normal=3,Fast=2, Super Fast = 1" },
              { (uint32) MyControlProperty::Flags, "General", "File flags", PropertyType::Flags, "Read=1,Write=2,Execute=4,Shared=8" },
              { (uint32) MyControlProperty::Custom, "General", "Custom prop", PropertyType::Custom },
        });
    };
};
class PropertyWindowExmaple : public Window
{
    Reference<MyUserControl> ct;

  public:
    PropertyWindowExmaple() : Window("Example", "d:c,w:60,h:20", WindowFlags::Sizeable)
    {
        auto sp = Factory::Splitter::Create(this, "d:c", true);
        ct      = sp->CreateChildControl<MyUserControl>();
        auto pl =
              sp->CreateChildControl<PropertyList>("d:c", ct.UpCast<PropertiesInterface>(), PropertyListFlags::Border);
        sp->SetSecondPanelSize(30);
    }
};
int main()
{
    if (!Application::Init(InitializationFlags::EnableFPSMode))
        return 1;
    Application::AddWindow(std::make_unique<PropertyWindowExmaple>());
    Application::Run();
    return 0;
}
