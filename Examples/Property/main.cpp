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
    AnimationKey,
    Flags,
    Custom
};
enum class BorderType : uint8
{
    Single = 1,
    Double = 2,
    Thick  = 3
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
    uint32 flags;
    BorderType borderType;
    std::u16string name;
    Input::Key keyToStopAnimation;

  public:
    MyUserControl() : UserControl("d:c")
    {
        counter            = 0;
        frameDelay         = 3;
        x                  = 2;
        y                  = 3;
        addX               = 1;
        addY               = -1;
        sz                 = { 15, 5 };
        ch                 = 'X';
        c                  = ColorPair{ Color::Red, Color::Black };
        hasBorder          = false;
        animationStarted   = true;
        borderType         = BorderType::Single;
        flags              = 59;
        name               = u"Dragoș";
        keyToStopAnimation = Input::Key::F1;
    }
    bool OnFrameUpdate() override
    {
        if (!animationStarted)
            return false;
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

        if (hasBorder)
        {
            switch (borderType)
            {
            case BorderType::Single:
                renderer.DrawRect(x - 1, y - 1, x + sz.Width, y + sz.Height, c, LineType::Single);
                break;
            case BorderType::Double:
                renderer.DrawRect(x - 1, y - 1, x + sz.Width, y + sz.Height, c, LineType::Double);
                break;
            case BorderType::Thick:
                renderer.FillHorizontalLineWithSpecialChar(x - 1, y - 1, x + sz.Width, SpecialChars::BlockLowerHalf, c);
                renderer.FillHorizontalLineWithSpecialChar(
                      x - 1, y + sz.Height, x + sz.Width, SpecialChars::BlockUpperHalf, c);
                renderer.FillVerticalLineWithSpecialChar(x - 1, y, y + sz.Height - 1, SpecialChars::BlockLeftHalf, c);
                renderer.FillVerticalLineWithSpecialChar(
                      x + sz.Width, y, y + sz.Height - 1, SpecialChars::BlockRightHalf, c);
                break;
            }
        }
    }
    bool OnKeyEvent(Input::Key code, char16) override
    {
        if (code == keyToStopAnimation)
        {
            animationStarted = !animationStarted;
            return true;
        }
        return false;
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
            value = name;
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
            value = (uint8) borderType;
            return true;
        case MyControlProperty::AnimationStarted:
            value = animationStarted;
            return true;
        case MyControlProperty::AnimationSpeed:
            value = frameDelay;
            return true;
        case MyControlProperty::Flags:
            value = flags;
            return true;
        case MyControlProperty::Custom:
            value = "Custom string representation";
            return true;
        case MyControlProperty::AnimationKey:
            value = keyToStopAnimation;
            return true;
        }
        return false;
    };
    bool SetPropertyValue(uint32 id, const PropertyValue& value, String& error) override
    {
        Size tmpSz;
        int32 tmpValue;
        switch (static_cast<MyControlProperty>(id))
        {
        case MyControlProperty::X:
            tmpValue = std::get<int32>(value);
            if (tmpValue < 0)
            {
                error.Format("X coordonate must be positive (%d)", tmpValue);
                return false;
            }
            this->x = tmpValue;
            return true;
        case MyControlProperty::Y:
            tmpValue = std::get<int32>(value);
            if (tmpValue < 0)
            {
                error.Format("Y coordonate must be positive (%d)", tmpValue);
                return false;
            }
            this->y = tmpValue;
            return true;
        case MyControlProperty::Size:
            tmpSz = std::get<Size>(value);
            if ((tmpSz.Width == 0) || (tmpSz.Height == 0))
            {
                error.Format(
                      "Invalid size (%u x %u) --> Width/Height must be bigger than 0", tmpSz.Width, tmpSz.Height);
                return false;
            }
            else
                this->sz = tmpSz;
            return true;
        case MyControlProperty::ForeColor:
            this->c.Foreground = std::get<Color>(value);
            return true;
        case MyControlProperty::BackColor:
            this->c.Background = std::get<Color>(value);
            return true;
        case MyControlProperty::Name:
            name = std::get<u16string_view>(value);
            return true;
        case MyControlProperty::Version:
            // do nothing ==> read-only
            return true;
        case MyControlProperty::Border:
            this->hasBorder = std::get<bool>(value);
            return true;
        case MyControlProperty::BorderType:
            this->borderType = static_cast<BorderType>(std::get<uint64>(value));
            return true;
        case MyControlProperty::AnimationStarted:
            this->animationStarted = std::get<bool>(value);
            return true;
        case MyControlProperty::AnimationSpeed:
            this->frameDelay = (uint32) std::get<uint64>(value);
            return true;
        case MyControlProperty::Flags:
            this->flags = (uint32) std::get<uint64>(value);
            return true;
        case MyControlProperty::AnimationKey:
            this->keyToStopAnimation = std::get<Input::Key>(value);
            return true;
        case MyControlProperty::Character:
            this->ch = std::get<char16>(value);
            return true;
        }
        error.SetFormat("Unknwon property ID: %u", (uint32) id);
        return false;
    };
    void SetCustomPropetyValue(uint32 /*propertyID*/) override
    {
        AppCUI::Dialogs::MessageBox::ShowNotification("Info", "Custom window for propert chage");
    }
    bool IsPropertyValueReadOnly(uint32 propertyID) override
    {
        // if hasBorder is false, than BorderType is readOnly and can not be changed
        if ((hasBorder == false) && (propertyID == (uint32) MyControlProperty::BorderType))
            return true;
        if (propertyID == (uint32) MyControlProperty::Version)
            return true;
        return false;
    }
    const vector<Property> GetPropertiesList() override
    {
        return {
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
              { (uint32) MyControlProperty::AnimationKey, "Animation", "Key for start/stop", PropertyType::Key },
              { (uint32) MyControlProperty::AnimationSpeed,
                "Animation",
                "Speed",
                PropertyType::List,
                "  Very Slow   = 5,Slow=4,Normal=3,Fast=2, Super Fast = 1" },
              { (uint32) MyControlProperty::Flags,
                "General",
                "File flags",
                PropertyType::Flags,
                "Read=1,Write=2,Execute=4,Shared=8" },
              { (uint32) MyControlProperty::Custom, "General", "Custom prop", PropertyType::Custom },
        };
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
    //void Paint(Graphics::Renderer & renderer) override
    //{
    //    renderer.Clear(' ', ColorPair{ Color::White, Color::Blue });
    //}
};
int main()
{
    if (!Application::Init(InitializationFlags::EnableFPSMode))
        return 1;
    Application::AddWindow(std::make_unique<PropertyWindowExmaple>());
    Application::Run();
    return 0;
}
