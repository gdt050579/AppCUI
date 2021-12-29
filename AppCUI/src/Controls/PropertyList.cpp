#include "ControlContext.hpp"
#include <string.h>
#include <set>

#ifdef MessageBox
#    undef MessageBox
#endif

namespace AppCUI
{
constexpr uint32 CATEGORY_FLAG         = 0x80000000;
constexpr uint32 CATEGORY_INDEX_MASK   = 0x7FFFFFFF;
constexpr int32 FILTER_PREFERED_WIDTH  = 17;
constexpr int32 BUTTON_COMMAND_REFRESH = 1;
constexpr int32 BUTTON_COMMAND_OK      = 2;
constexpr int32 BUTTON_COMMAND_CANCEL  = 3;
constexpr uint64 INVALID_LISTVIEW_ITEM = 0xFFFFFFFFFFFFFFFFULL;
constexpr uint32 INVALID_ITEM          = 0xFFFFFFFFU;

bool PropertyValueToUInt64(const PropertyValue& value, uint64& result)
{
    if (std::holds_alternative<uint64>(value))
    {
        result = std::get<uint64>(value);
        return true;
    }
    if (std::holds_alternative<uint32>(value))
    {
        result = std::get<uint32>(value);
        return true;
    }
    if (std::holds_alternative<uint16>(value))
    {
        result = std::get<uint16>(value);
        return true;
    }
    if (std::holds_alternative<uint8>(value))
    {
        result = std::get<uint8>(value);
        return true;
    }
    // check for ints --> but only if positive
    if (std::holds_alternative<int64>(value))
    {
        auto intResult = std::get<int64>(value);
        if (intResult >= 0)
        {
            result = (uint64) intResult;
            return true;
        }
        RETURNERROR(false, "Invalid (non-positive) value for list/flag members");
    }
    if (std::holds_alternative<int32>(value))
    {
        auto intResult = std::get<int32>(value);
        if (intResult >= 0)
        {
            result = (uint64) intResult;
            return true;
        }
        RETURNERROR(false, "Invalid (non-positive) value for list/flag members");
    }
    if (std::holds_alternative<int16>(value))
    {
        auto intResult = std::get<int16>(value);
        if (intResult >= 0)
        {
            result = (uint64) intResult;
            return true;
        }
        RETURNERROR(false, "Invalid (non-positive) value for list/flag members");
    }
    if (std::holds_alternative<int8>(value))
    {
        auto intResult = std::get<int8>(value);
        if (intResult >= 0)
        {
            result = (uint64) intResult;
            return true;
        }
        RETURNERROR(false, "Invalid (non-positive) value for list/flag members");
    }
    RETURNERROR(false, "Unknwon (non-convertable) variant type for list/flag members");
}
class PropertyEditDialog : public Window
{
  protected:
    const PropertyInfo& prop;
    Reference<PropertiesInterface> object;
    bool isReadOnly;

    PropertyEditDialog(
          string_view layout, const PropertyInfo& _prop, Reference<PropertiesInterface> _object, bool readOnly)
        : Window("Edit", layout, WindowFlags::NoCloseButton), prop(_prop), object(_object), isReadOnly(readOnly)
    {
        if (readOnly)
        {
            Factory::Button::Create(this, "&Refresh", "l:7,b:0,w:12", BUTTON_COMMAND_REFRESH);
            Factory::Button::Create(this, "&Close", "l:21,b:0,w:12", BUTTON_COMMAND_CANCEL);
            this->SetText("View");
        }
        else
        {
            Factory::Button::Create(this, "&Refresh", "l:2,b:0,w:11", BUTTON_COMMAND_REFRESH);
            Factory::Button::Create(this, "&Ok", "l:14,b:0,w:11", BUTTON_COMMAND_OK);
            Factory::Button::Create(this, "&Cancel", "l:26,b:0,w:11", BUTTON_COMMAND_CANCEL);
        }
    }

  public:
    void ShowDialog()
    {
        this->OnInitPropertyDialog();
        this->Refresh();
        this->Show();
    }
    bool OnEvent(Reference<Control> /*sender*/, Event eventType, int id) override
    {
        switch (eventType)
        {
        case Event::WindowClose:
            this->Exit(0);
            return true;
        case Event::WindowAccept:
            if (this->isReadOnly)
                this->Exit(1);
            else
                Validate();
            return true;
        case Event::ListViewItemClicked:
            if (!this->isReadOnly)
                Validate();
            return true;
        case Event::ButtonClicked:
            if (id == BUTTON_COMMAND_REFRESH)
                this->Refresh();
            else if (id == BUTTON_COMMAND_OK)
            {
                if (this->isReadOnly)
                    this->Exit(1);
                else
                    Validate();
            }
            else if (id == BUTTON_COMMAND_CANCEL)
                this->Exit(0);
            return true;
        }
        return false;
    }
    virtual void OnInitPropertyDialog() = 0;
    virtual void Refresh()              = 0;
    virtual void Validate()             = 0;
};

class PropertyCharEditDialog : public PropertyEditDialog
{
    Reference<CharacterTable> chTable;
    bool isChar8;

  public:
    PropertyCharEditDialog(
          const PropertyInfo& _prop, Reference<PropertiesInterface> _object, bool readOnly, bool char8value)
        : PropertyEditDialog("d:c,w:40,h:20", _prop, _object, readOnly), isChar8(char8value)
    {
    }
    void OnInitPropertyDialog() override
    {
        chTable = Factory::CharacterTable::Create(this, "t:2,l:1,r:1,b:2");
        chTable->SetEnabled(!isReadOnly);
        if (!isReadOnly)
            chTable->SetFocus();
    }
    void Refresh() override
    {
        PropertyValue tempPropValue;
        LocalString<256> tmpString;

        if (this->object->GetPropertyValue(prop.id, tempPropValue))
        {
            if (std::holds_alternative<char8>(tempPropValue))
                chTable->SetCharacter(std::get<char8>(tempPropValue));
            if (std::holds_alternative<char16>(tempPropValue))
                chTable->SetCharacter(std::get<char16>(tempPropValue));
        }
        else
        {
            Dialogs::MessageBox::ShowError(
                  "Error", tmpString.Format("Unable to read property value for %s", prop.name.GetText()));
        }
        if (!isReadOnly)
            chTable->SetFocus();
    }
    void Validate() override
    {
        LocalString<256> error;
        char16 ch = chTable->GetCharacter();
        if (this->isChar8)
        {
            if (object->SetPropertyValue(prop.id, (char8) ch, error))
            {
                this->Exit(0);
                return;
            }
        }
        else
        {
            if (object->SetPropertyValue(prop.id, (char16) ch, error))
            {
                this->Exit(0);
                return;
            }
        }
        // error
        Dialogs::MessageBox::ShowError("Error", error);
        chTable->SetFocus();
    }
};
class PropertyColorEditDialog : public PropertyEditDialog
{
    Reference<ColorPicker> col;

  public:
    PropertyColorEditDialog(const PropertyInfo& _prop, Reference<PropertiesInterface> _object, bool readOnly)
        : PropertyEditDialog("d:c,w:40,h:8", _prop, _object, readOnly)
    {
    }
    void OnInitPropertyDialog() override
    {
        Factory::Label::Create(this, prop.name, "x:2,y:1,w:36");
        col = Factory::ColorPicker::Create(this, "t:2,l:1,r:1", Color::Black);
        col->SetEnabled(!isReadOnly);
        if (!isReadOnly)
            col->SetFocus();
    }
    void Refresh() override
    {
        PropertyValue tempPropValue;
        LocalString<256> tmpString;

        if (this->object->GetPropertyValue(prop.id, tempPropValue))
        {
            col->SetColor(std::get<Color>(tempPropValue));
        }
        else
        {
            Dialogs::MessageBox::ShowError(
                  "Error", tmpString.Format("Unable to read property value for %s", prop.name.GetText()));
        }
        if (!isReadOnly)
            col->SetFocus();
    }
    void Validate() override
    {
        LocalString<256> error;

        if (object->SetPropertyValue(prop.id, col->GetColor(), error))
        {
            this->Exit(0);
            return;
        }
        // error
        Dialogs::MessageBox::ShowError("Error", error);
        col->SetFocus();
    }
};
class PropertyKeyEditDialog : public PropertyEditDialog
{
    Reference<KeySelector> key;

  public:
    PropertyKeyEditDialog(const PropertyInfo& _prop, Reference<PropertiesInterface> _object, bool readOnly)
        : PropertyEditDialog("d:c,w:40,h:8", _prop, _object, readOnly)
    {
    }
    void OnInitPropertyDialog() override
    {
        Factory::Label::Create(this, prop.name, "x:2,y:1,w:36");

        if (isReadOnly)
            key = Factory::KeySelector::Create(this, "t:2,l:1,r:1", Key::None, KeySelectorFlags::ReadOnly);
        else
            key = Factory::KeySelector::Create(this, "t:2,l:1,r:1", Key::None, KeySelectorFlags::None);
        key->SetFocus();
    }
    void Refresh() override
    {
        PropertyValue tempPropValue;
        LocalString<256> tmpString;

        if (this->object->GetPropertyValue(prop.id, tempPropValue))
        {
            key->SetSelectedKey(std::get<Input::Key>(tempPropValue));
        }
        else
        {
            Dialogs::MessageBox::ShowError(
                  "Error", tmpString.Format("Unable to read property value for %s", prop.name.GetText()));
        }
        key->SetFocus();
    }
    void Validate() override
    {
        LocalString<256> error;

        if (object->SetPropertyValue(prop.id, key->GetSelectedKey(), error))
        {
            this->Exit(0);
            return;
        }
        // error
        Dialogs::MessageBox::ShowError("Error", error);
        key->SetFocus();
    }
};
class PropertyFlagsEditDialog : public PropertyEditDialog
{
    Reference<ListView> lv;
    ItemHandle* items;

  public:
    PropertyFlagsEditDialog(const PropertyInfo& _prop, Reference<PropertiesInterface> _object, bool readOnly)
        : PropertyEditDialog("d:c,w:40,h:20", _prop, _object, readOnly), items(nullptr)
    {
    }
    ~PropertyFlagsEditDialog()
    {
        if (items)
            delete[] items;
        items = nullptr;
    }
    void OnInitPropertyDialog() override
    {
        lv = Factory::ListView::Create(
              this,
              "l:1,t:1,r:1,b:3",
              ListViewFlags::HideColumns | ListViewFlags::CheckBoxes | ListViewFlags::SearchMode);
        lv->AddColumn("", TextAlignament::Left, 100);
        if (prop.listValues.size() > 0)
        {
            items   = new ItemHandle[prop.listValues.size()];
            auto* i = items;
            for (auto& entry : prop.listValues)
            {
                *i = lv->AddItem(entry.second);
                lv->SetItemData(*i, entry.first);
                i++;
            }
        }
        lv->SetFocus();
    }
    void Refresh() override
    {
        PropertyValue tempPropValue;
        LocalString<256> tmpString;

        if (this->object->GetPropertyValue(prop.id, tempPropValue))
        {
            uint64 result;
            if (PropertyValueToUInt64(tempPropValue, result))
            {
                auto* i = items;
                auto* e = items + prop.listValues.size();
                while (i < e)
                {
                    auto flagValue = lv->GetItemData(*i, 0);
                    lv->SetItemCheck(*i, (flagValue & result) == flagValue);
                    i++;
                }
            }
            else
            {
                Dialogs::MessageBox::ShowError(
                      "Error",
                      tmpString.Format(
                            "Item value should be an index (uint8/16/32/64 or int8/16/32/64) for %s",
                            prop.name.GetText()));
            }
        }
        else
        {
            Dialogs::MessageBox::ShowError(
                  "Error", tmpString.Format("Unable to read property value for %s", prop.name.GetText()));
        }
        lv->SetFocus();
    }
    void Validate() override
    {
        if (lv->GetItemsCount() == 0)
            return;
        LocalString<256> error;
        uint64 value = 0;
        auto* i      = items;
        auto* e      = items + prop.listValues.size();
        while (i < e)
        {
            if (lv->IsItemChecked(*i))
                value |= lv->GetItemData(*i, 0);
            i++;
        }
        if (object->SetPropertyValue(prop.id, value, error))
        {
            this->Exit(0);
            return;
        }
        // error
        Dialogs::MessageBox::ShowError("Error", error);
        lv->SetFocus();
    }
};
class PropertyListEditDialog : public PropertyEditDialog
{
    Reference<ListView> lv;
    ItemHandle* items;

  public:
    PropertyListEditDialog(const PropertyInfo& _prop, Reference<PropertiesInterface> _object, bool readOnly)
        : PropertyEditDialog("d:c,w:40,h:20", _prop, _object, readOnly), items(nullptr)
    {
    }
    ~PropertyListEditDialog()
    {
        if (items)
            delete[] items;
        items = nullptr;
    }
    void OnInitPropertyDialog() override
    {
        lv = Factory::ListView::Create(this, "l:1,t:1,r:1,b:3", ListViewFlags::HideColumns | ListViewFlags::SearchMode);
        lv->AddColumn("", TextAlignament::Left, 100);
        if (prop.listValues.size() > 0)
        {
            items   = new ItemHandle[prop.listValues.size()];
            auto* i = items;
            for (auto& entry : prop.listValues)
            {
                *i = lv->AddItem(entry.second);
                lv->SetItemData(*i, entry.first);
                i++;
            }
        }
        lv->SetFocus();
    }
    void Refresh() override
    {
        PropertyValue tempPropValue;
        LocalString<256> tmpString;
        auto* i = items;
        auto* e = items + prop.listValues.size();
        while (i < e)
        {
            lv->SetItemType(*i, ListViewItemType::Normal);
            i++;
        }
        if (this->object->GetPropertyValue(prop.id, tempPropValue))
        {
            uint64 result;
            if (PropertyValueToUInt64(tempPropValue, result))
            {
                i = items;
                while (i < e)
                {
                    if (lv->GetItemData(*i, INVALID_LISTVIEW_ITEM) == result)
                        lv->SetItemType(*i, ListViewItemType::Emphasized_1);
                    i++;
                }
            }
            else
            {
                Dialogs::MessageBox::ShowError(
                      "Error",
                      tmpString.Format(
                            "Item value should be an index (uint8/16/32/64 or int8/16/32/64) for %s",
                            prop.name.GetText()));
            }
        }
        else
        {
            Dialogs::MessageBox::ShowError(
                  "Error", tmpString.Format("Unable to read property value for %s", prop.name.GetText()));
        }
        lv->SetFocus();
    }
    void Validate() override
    {
        if (lv->GetItemsCount() == 0)
            return;
        LocalString<256> error;
        auto result = lv->GetItemData(lv->GetCurrentItem(), INVALID_LISTVIEW_ITEM);
        if (result != INVALID_LISTVIEW_ITEM)
        {
            if (object->SetPropertyValue(prop.id, result, error))
            {
                this->Exit(0);
                return;
            }
            // error
            Dialogs::MessageBox::ShowError("Error", error);
            lv->SetFocus();
        }
        else
        {
            Dialogs::MessageBox::ShowError("Error", "Internal error - invalid item !");
            lv->SetFocus();
        }
    }
};
class PropertyTextEditDialog : public PropertyEditDialog
{
    Reference<TextField> txt;

  public:
    PropertyTextEditDialog(const PropertyInfo& _prop, Reference<PropertiesInterface> _object, bool readOnly)
        : PropertyEditDialog("d:c,w:40,h:8", _prop, _object, readOnly)
    {
    }
    void OnInitPropertyDialog() override
    {
        Factory::Label::Create(this, prop.name, "x:2,y:1,w:36");

        if (isReadOnly)
            txt = Factory::TextField::Create(this, "", "t:2,l:1,r:1", TextFieldFlags::Readonly);
        else
            txt = Factory::TextField::Create(this, "", "t:2,l:1,r:1");
    }
    void Refresh() override
    {
        PropertyValue tempPropValue;
        LocalString<256> tmpString;
        NumericFormatter n;
        if (this->object->GetPropertyValue(prop.id, tempPropValue))
        {
            switch (prop.type)
            {
            case PropertyType::UInt8:
                txt->SetText(n.ToDec(std::get<uint8>(tempPropValue)));
                break;
            case PropertyType::UInt16:
                txt->SetText(n.ToDec(std::get<uint16>(tempPropValue)));
                break;
            case PropertyType::UInt32:
                txt->SetText(n.ToDec(std::get<uint32>(tempPropValue)));
                break;
            case PropertyType::UInt64:
                txt->SetText(n.ToDec(std::get<uint64>(tempPropValue)));
                break;
            case PropertyType::Int8:
                txt->SetText(n.ToDec(std::get<int8>(tempPropValue)));
                break;
            case PropertyType::Int16:
                txt->SetText(n.ToDec(std::get<int16>(tempPropValue)));
                break;
            case PropertyType::Int32:
                txt->SetText(n.ToDec(std::get<int32>(tempPropValue)));
                break;
            case PropertyType::Int64:
                txt->SetText(n.ToDec(std::get<int64>(tempPropValue)));
                break;
            case PropertyType::Float:
                txt->SetText(n.ToDec(std::get<float>(tempPropValue)));
                break;
            case PropertyType::Double:
                txt->SetText(n.ToDec(std::get<double>(tempPropValue)));
                break;
            case PropertyType::Ascii:
                txt->SetText(std::get<string_view>(tempPropValue));
                break;
            case PropertyType::Unicode:
                txt->SetText(std::get<u16string_view>(tempPropValue));
                break;
            case PropertyType::UTF8:
                txt->SetText(std::get<u8string_view>(tempPropValue));
                break;
            case PropertyType::CharacterView:
                txt->SetText(std::get<CharacterView>(tempPropValue));
                break;
            case PropertyType::Size:
                txt->SetText(tmpString.Format(
                      "%u x %u", std::get<Size>(tempPropValue).Width, std::get<Size>(tempPropValue).Height));
                break;
            default:
                txt->SetText("");
                Dialogs::MessageBox::ShowError(
                      "Error",
                      tmpString.Format("Unknown property type (%u) for %s", (uint32) prop.type, prop.name.GetText()));
                break;
            }
        }
        else
        {
            txt->SetText("");
            Dialogs::MessageBox::ShowError(
                  "Error", tmpString.Format("Unable to read property value for %s", prop.name.GetText()));
        }
        txt->SetFocus();
    }

    template <typename T>
    void UpdateNumericPropertValue(std::optional<T> value, String& asciiValue, string_view typeName)
    {
        LocalString<512> error;
        if (value.has_value())
        {
            if (object->SetPropertyValue(prop.id, value.value(), error))
            {
                // all good
                this->Exit(1);
                return;
            }
            else
            {
                Dialogs::MessageBox::ShowError("Error", error);
            }
        }
        else
        {
            Dialogs::MessageBox::ShowError(
                  "Error",
                  error.Format("Fail to convert '%s' to a valid '%s' value", asciiValue.GetText(), typeName.data()));
            txt->SetFocus();
        }
    }
    template <typename T>
    void UpdateStringPropertyValue(const T& value)
    {
        LocalString<512> error;

        if (object->SetPropertyValue(prop.id, value, error))
        {
            // all good
            this->Exit(1);
            return;
        }
        else
        {
            Dialogs::MessageBox::ShowError("Error", error);
        }
    }
    void Validate() override
    {
        LocalString<256> asciiValue;
        LocalUnicodeStringBuilder<256> unicodeValue;

        switch (prop.type)
        {
        case PropertyType::UInt8:
        case PropertyType::UInt16:
        case PropertyType::UInt32:
        case PropertyType::UInt64:
        case PropertyType::Int8:
        case PropertyType::Int16:
        case PropertyType::Int32:
        case PropertyType::Int64:
        case PropertyType::Float:
        case PropertyType::Double:
        case PropertyType::Ascii:
        case PropertyType::Size:
            // an ascii text is required
            if (!asciiValue.Set((CharacterView) txt->GetText()))
            {
                Dialogs::MessageBox::ShowError(
                      "Error",
                      "Fail to convert value to ascii (make sure that you did not introduced any unicode characters or "
                      "control characters)");
                txt->SetFocus();
                return; // value was not validated
            }
            break;
        case PropertyType::Unicode:
        case PropertyType::UTF8:
            unicodeValue.Set(txt->GetText());
            break;
        }
        // update the value
        switch (prop.type)
        {
        case PropertyType::UInt8:
            UpdateNumericPropertValue<uint8>(Number::ToUInt8(asciiValue), asciiValue, "UInt8");
            break;
        case PropertyType::UInt16:
            UpdateNumericPropertValue<uint16>(Number::ToUInt16(asciiValue), asciiValue, "UInt16");
            break;
        case PropertyType::UInt32:
            UpdateNumericPropertValue<uint32>(Number::ToUInt32(asciiValue), asciiValue, "UInt32");
            break;
        case PropertyType::UInt64:
            UpdateNumericPropertValue<uint64>(Number::ToUInt64(asciiValue), asciiValue, "UInt64");
            break;
        case PropertyType::Int8:
            UpdateNumericPropertValue<int8>(Number::ToInt8(asciiValue), asciiValue, "Int8");
            break;
        case PropertyType::Int16:
            UpdateNumericPropertValue<int16>(Number::ToInt16(asciiValue), asciiValue, "Int16");
            break;
        case PropertyType::Int32:
            UpdateNumericPropertValue<int32>(Number::ToInt32(asciiValue), asciiValue, "Int32");
            break;
        case PropertyType::Int64:
            UpdateNumericPropertValue<int64>(Number::ToInt64(asciiValue), asciiValue, "Int64");
            break;
        case PropertyType::Float:
            UpdateNumericPropertValue<float>(Number::ToFloat(asciiValue), asciiValue, "Float");
            break;
        case PropertyType::Double:
            UpdateNumericPropertValue<double>(Number::ToDouble(asciiValue), asciiValue, "Double");
            break;
        case PropertyType::Size:
            UpdateNumericPropertValue<Size>(Size::FromString(asciiValue), asciiValue, "Size");
            break;
        case PropertyType::Ascii:
            UpdateStringPropertyValue<string_view>(asciiValue);
            break;
        case PropertyType::CharacterView:
            UpdateStringPropertyValue<CharacterView>(txt->GetText());
            break;
        case PropertyType::Unicode:
            UpdateStringPropertyValue<u16string_view>(unicodeValue);
            break;
        case PropertyType::UTF8:
            UpdateStringPropertyValue<u8string_view>((std::u8string) unicodeValue);
            break;
        default:
            Dialogs::MessageBox::ShowError("Error", "Update for this type is not implemented yet");
            break;
        }
    };
};

class ListItemsParser
{
    const char16* start;
    const char16* end;

    inline const char16* SkipSpaces(const char16* pos) const
    {
        while ((pos < end) && (((*pos) == ' ') || ((*pos) == '\t') || ((*pos) == '\n') || ((*pos) == '\r')))
            pos++;
        return pos;
    }
    inline const char16* SkipWord(const char16* pos, char c1, char c2) const
    {
        auto st = pos;
        while ((pos < end) && ((*pos) != c1) && ((*pos) != c2))
            pos++;
        if (pos < end) // I've reached either a `c1` or `c2` character
        {
            // go back and skip spaces/tabs or new lines
            pos--;
            while ((pos > st) && (((*pos) == ' ') || ((*pos) == '\t') || ((*pos) == '\n') || ((*pos) == '\r')))
                pos--;
            pos++; // next char
        }

        return pos;
    }
    inline bool CheckNextChar(const char16* pos, char c1, char c2, bool failIfEndOfBuffer) const
    {
        if ((pos < end) && (((*pos) == c1) || ((*pos) == c2)))
            return true;
        if ((pos >= end) && (!failIfEndOfBuffer))
            return true;
        return false;
    }

  public:
    ListItemsParser(const char16* _start, const char16* _end) : start(_start), end(_end)
    {
    }
    bool CreateDictionary(std::map<uint64, FixSizeUnicode<PROPERTY_VALUE_MAXCHARS>>& result)
    {
        const char16* k_start;
        const char16* k_end;
        const char16* v_start;
        const char16* v_end;
        const char16* p = start;
        char asciiValue[128];
        char* v_ascii;

        CHECK(p, false, "Expecting a valid NON-NULL buffer for ListItemParser !");

        while (p < end)
        {
            k_start = SkipSpaces(p);
            k_end   = SkipWord(k_start, '=', '=');
            p       = SkipSpaces(k_end);
            CHECK(CheckNextChar(p, '=', '=', true), false, "");
            v_start = SkipSpaces(p + 1);
            v_end   = SkipWord(v_start, ',', ';');
            p       = SkipSpaces(v_end);
            CHECK(CheckNextChar(p, ',', ';', false), false, "");
            p = SkipSpaces(p + 1);
            CHECK(v_start < v_end, false, "Expecting a value in list");
            CHECK(k_start < k_end, false, "Expecting a key in list");
            CHECK((v_end - v_start) < 127, false, "Numbers in list must not exceed 126 characters");

            v_ascii = asciiValue;
            while (v_start < v_end)
            {
                CHECK((((*v_start) > 32) && ((*v_start) < 127)),
                      false,
                      "Invalid character representation for a number");
                *v_ascii = (char) (*v_start);
                v_ascii++;
                v_start++;
            }
            string_view value((const char*) asciiValue, (size_t) (v_ascii - asciiValue));
            auto res = Number::ToUInt64(value);
            CHECK(res.has_value(), false, "Invalid value (or invalid uint64 value) in list");
            result[res.value()] = u16string_view{ k_start, (size_t) (k_end - k_start) };
        }
        return true;
    }
    bool CreateBooleanValues(std::map<uint64, FixSizeUnicode<PROPERTY_VALUE_MAXCHARS>>& result)
    {
        const char16* k_start;
        const char16* k_end;
        const char16* p = start;
        uint64 val      = 0; // false;

        CHECK(p, false, "Expecting a valid NON-NULL buffer for ListItemParser !");

        while ((p < end) && (val < 2))
        {
            k_start = SkipSpaces(p);
            k_end   = SkipWord(k_start, ',', ';');
            p       = SkipSpaces(k_end);
            CHECK(CheckNextChar(p, ',', ';', false), false, "");
            p             = SkipSpaces(p + 1);
            result[val++] = u16string_view{ k_start, (size_t) (k_end - k_start) };
        }
        return ((val == 2) && (p >= end));
    }
};

int32 SortWithCategories(int32 i1, int32 i2, void* context)
{
    auto* PC = reinterpret_cast<PropertyListContext*>(context);
    int32 result;
    const auto& p1 = PC->properties[i1];
    const auto& p2 = PC->properties[i2];
    const auto* c1 = PC->categories[p1.category].name.GetText();
    const auto* c2 = PC->categories[p2.category].name.GetText();
    result         = String::Compare(c1, c2, true);
    if (result != 0)
        return result;
    return String::Compare(p1.name.GetText(), p2.name.GetText(), true);
}
int32 SortWithoutCategories(int32 i1, int32 i2, void* context)
{
    auto* PC       = reinterpret_cast<PropertyListContext*>(context);
    const auto& p1 = PC->properties[i1];
    const auto& p2 = PC->properties[i2];

    return String::Compare(p1.name.GetText(), p2.name.GetText(), true);
}
void PropertyListContext::DrawCategory(uint32 index, int32 y, Graphics::Renderer& renderer)
{
    if (index >= this->categories.size())
        return;
    const auto& cat = this->categories[index];
    int32 x         = this->hasBorder ? 1 : 0;
    int32 w         = this->hasBorder ? this->Layout.Width - 2 : this->Layout.Width;

    renderer.FillHorizontalLine(x, y, w, ' ', Colors.Category.Text);

    WriteTextParams params(
          WriteTextFlags::OverwriteColors | WriteTextFlags::SingleLine | WriteTextFlags::FitTextToWidth);
    params.Y = y;
    if (w >= 5)
    {
        params.X     = x + 2;
        params.Color = Colors.Category.Text;
        params.Width = w - 5;
        renderer.WriteText(cat.name, params);
    }

    LocalString<32> tmp;
    if (cat.filteredItems < cat.totalItems)
    {
        tmp.SetFormat("[%u/%u]", cat.filteredItems, cat.totalItems);
    }
    else
    {
        tmp.SetFormat("[%u]", cat.totalItems);
    }
    if ((int32) tmp.Len() + 5 <= w)
    {
        params.Align = TextAlignament::Right;
        params.Width = tmp.Len();
        params.X     = w - 1;
        params.Color = Colors.Category.Stats;
        renderer.WriteText(tmp, params);
    }
    if (w > 0)
    {
        renderer.WriteSpecialCharacter(
              x, y, cat.folded ? SpecialChars::TriangleRight : SpecialChars::TriangleDown, Colors.Category.Arrow);
    }
}
void PropertyListContext::DrawListProperty(
      WriteTextParams& params, PropertyValue& pv, const PropertyInfo& pi, Graphics::Renderer& renderer, bool readOnly)
{
    uint64 result;
    LocalString<128> errText;
    if (PropertyValueToUInt64(pv, result))
    {
        auto it = pi.listValues.find(result);
        if (it != pi.listValues.cend())
        {
            renderer.WriteText(it->second, params);
        }
        else
        {
            params.Color = Colors.Item.Error;
            renderer.WriteText(errText.Format("Value %llu is not among list accepted values", result), params);
        }
        if (!readOnly)
        {
            // draw the expand arrow
            auto X = params.X + params.Width - 2;
            renderer.WriteCharacter(X++, params.Y, ' ', Colors.Item.Text);
            renderer.WriteSpecialCharacter(X, params.Y, SpecialChars::TriangleDown, Colors.Item.Checked);
        }
    }
    else
    {
        params.Color = Colors.Item.Error;
        renderer.WriteText(
              "Invalid value type for list (expected a pozitive int value - uint8/16/32/64 or int8/16/32/64>0)",
              params);
    }
}
void PropertyListContext::DrawFlagsProperty(
      WriteTextParams& params, PropertyValue& pv, const PropertyInfo& pi, Graphics::Renderer& renderer, bool readOnly)
{
    uint64 result;
    LocalString<64> errText;
    LocalUnicodeStringBuilder<1024> flagList;
    if (PropertyValueToUInt64(pv, result))
    {
        bool first = true;
        for (auto& e : pi.listValues)
        {
            if ((result & e.first) == (e.first))
            {
                if (!first)
                    flagList.Add(u",");
                flagList.Add(e.second.GetText());
                first = false;
                result -= e.first; // remove flag
            }
        }
        if (result != 0)
        {
            flagList.Add(u" +[Unk bits: 0x");
            NumericFormatter n;
            flagList.Add(n.ToHex(result));
            flagList.Add(u"]");
        }
        renderer.WriteText(flagList, params);
        if (!readOnly)
        {
            // draw the expand arrow
            auto X = params.X + params.Width - 2;
            renderer.WriteCharacter(X++, params.Y, ' ', Colors.Item.Text);
            renderer.WriteSpecialCharacter(X, params.Y, SpecialChars::ThreePointsHorizontal, Colors.Item.Checked);
        }
    }
    else
    {
        params.Color = Colors.Item.Error;
        renderer.WriteText(
              "Invalid value type for flags (expected a pozitive int value - uint8/16/32/64 or int8/16/32/64>0)",
              params);
    }
}
void PropertyListContext::DrawCustomProperty(
      WriteTextParams& params, PropertyValue& pv, Graphics::Renderer& renderer, bool readOnly)
{
    bool found = false;
    LocalString<64> errText;
    ConstString stringRepr;
    if (std::holds_alternative<string_view>(pv))
    {
        stringRepr = std::get<string_view>(pv);
        found      = true;
    }
    else if (std::holds_alternative<u16string_view>(pv))
    {
        stringRepr = std::get<u16string_view>(pv);
        found      = true;
    }
    else if (std::holds_alternative<u8string_view>(pv))
    {
        stringRepr = std::get<u8string_view>(pv);
        found      = true;
    }

    if (found)
    {
        renderer.WriteText(stringRepr, params);
        if (!readOnly)
        {
            // draw the expand arrow
            auto X = params.X + params.Width - 2;
            renderer.WriteCharacter(X++, params.Y, ' ', Colors.Item.Text);
            renderer.WriteSpecialCharacter(X, params.Y, SpecialChars::ThreePointsHorizontal, Colors.Item.Checked);
        }
    }
    else
    {
        params.Color = Colors.Item.Error;
        renderer.WriteText("Invalid value type for list (expecting a string (ascii/unicode/UTF-8)", params);
    }
}
void PropertyListContext::DrawProperty(uint32 index, int32 y, Graphics::Renderer& renderer, bool& readOnlyStatus)
{
    if (index >= this->properties.size())
        return;
    const auto& prop       = this->properties[index];
    const int32 extraSpace = this->showCategories ? 3 : 0;
    int32 x                = this->hasBorder ? 1 : 0;
    NumericFormatter n;
    WriteTextParams params(WriteTextFlags::OverwriteColors | WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth);
    if ((x + extraSpace + this->propertyNameWidth) < (int32) this->Layout.Width)
    {
        params.X     = x + extraSpace;
        params.Y     = y;
        params.Color = Colors.Item.Text;
        params.Width = this->propertyNameWidth;
        renderer.WriteText(prop.name, params);
        auto col = (this->separatorStatus == PropertySeparatorStatus::None) ? Colors.Item.LineSeparator
                                                                            : Colors.Item.Checked;
        renderer.WriteSpecialCharacter(
              x + extraSpace + this->propertyNameWidth, y, SpecialChars::BoxVerticalSingleLine, col);
    }
    bool readOnly  = IsPropertyReadOnly(prop);
    auto w         = this->hasBorder ? ((int32) this->Layout.Width - (x + extraSpace + 2 + this->propertyNameWidth))
                                     : ((int32) this->Layout.Width - (x + extraSpace + 1 + this->propertyNameWidth));
    readOnlyStatus = readOnly;
    params.X       = x + extraSpace + 1 + this->propertyNameWidth;
    params.Y       = y;
    params.Color   = readOnly ? Colors.Item.ReadOnly : Colors.Item.Value;
    params.Flags   = WriteTextFlags::OverwriteColors | WriteTextFlags::SingleLine | WriteTextFlags::FitTextToWidth;

    if (this->object->GetPropertyValue(prop.id, tempPropValue))
    {
        string_view tmpAscii;
        u16string_view tmpUnicode;
        u8string_view tmpUTF8;
        CharacterView tmpCharView;
        NumericFormatter n;
        LocalString<32> tmpString;
        Size tmpSize;
        char16 tmpCh16;
        switch (prop.type)
        {
        case PropertyType::Boolean:
            if (prop.listValues.size() == 2)
            {
                tmpUnicode = prop.listValues.find(std::get<bool>(tempPropValue) ? 1 : 0)->second;
            }
            else
            {
                tmpAscii = std::get<bool>(tempPropValue) ? "(Yes)" : "(No)";
            }
            break;
        case PropertyType::Char8:
            tmpCh16  = std::get<char8>(tempPropValue);
            tmpAscii = tmpString.Format(
                  "| 0x%04X | Dec:%u |", (uint32) (*(uint16*) &tmpCh16), (uint32) (*(uint16*) &tmpCh16));
            break;
        case PropertyType::Char16:
            tmpCh16  = std::get<char16>(tempPropValue);
            tmpAscii = tmpString.Format(
                  "| 0x%04X | Dec:%u |", (uint32) (*(uint16*) &tmpCh16), (uint32) (*(uint16*) &tmpCh16));
            break;
        case PropertyType::UInt8:
            tmpAscii = n.ToDec(std::get<uint8>(tempPropValue));
            break;
        case PropertyType::UInt16:
            tmpAscii = n.ToDec(std::get<uint16>(tempPropValue));
            break;
        case PropertyType::UInt32:
            tmpAscii = n.ToDec(std::get<uint32>(tempPropValue));
            break;
        case PropertyType::UInt64:
            tmpAscii = n.ToDec(std::get<uint64>(tempPropValue));
            break;
        case PropertyType::Int8:
            tmpAscii = n.ToDec(std::get<int8>(tempPropValue));
            break;
        case PropertyType::Int16:
            tmpAscii = n.ToDec(std::get<int16>(tempPropValue));
            break;
        case PropertyType::Int32:
            tmpAscii = n.ToDec(std::get<int32>(tempPropValue));
            break;
        case PropertyType::Int64:
            tmpAscii = n.ToDec(std::get<int64>(tempPropValue));
            break;
        case PropertyType::Float:
            tmpAscii = n.ToDec(std::get<float>(tempPropValue));
            break;
        case PropertyType::Double:
            tmpAscii = n.ToDec(std::get<double>(tempPropValue));
            break;
        case PropertyType::Ascii:
            tmpAscii = std::get<string_view>(tempPropValue);
            break;
        case PropertyType::Unicode:
            tmpUnicode = std::get<u16string_view>(tempPropValue);
            break;
        case PropertyType::UTF8:
            tmpUTF8 = std::get<u8string_view>(tempPropValue);
            break;
        case PropertyType::CharacterView:
            tmpCharView = std::get<CharacterView>(tempPropValue);
            break;
        case PropertyType::Color:
            tmpAscii = ColorUtils::GetColorName(std::get<Graphics::Color>(tempPropValue));
            break;
        case PropertyType::Key:
            if (KeyUtils::ToString(std::get<Input::Key>(tempPropValue), tmpString))
                tmpAscii = tmpString.ToStringView();
            else
                tmpAscii = "?";
            break;
        case PropertyType::Size:
            tmpSize  = std::get<Graphics::Size>(tempPropValue);
            tmpAscii = tmpString.Format("%u x %u", tmpSize.Width, tmpSize.Height);
            break;
        case PropertyType::List:
        case PropertyType::Flags:
        case PropertyType::Custom:
            break; // have their own drawing method
        }

        if (w > 0)
        {
            params.Width = (uint32) w;

            switch (prop.type)
            {
            case PropertyType::Boolean:
                if (prop.listValues.size() == 2)
                {
                    renderer.WriteText(tmpUnicode, params);
                }
                else
                {
                    if (std::get<bool>(tempPropValue))
                        renderer.WriteSpecialCharacter(params.X, y, SpecialChars::CheckMark, Colors.Item.Checked);
                    else
                        renderer.WriteCharacter(params.X, y, 'x', Colors.Item.Unchecked);
                    params.X += 2;
                    if (w > 2)
                    {
                        params.Width -= 2;
                        renderer.WriteText(tmpAscii, params);
                    }
                }
                break;
            case PropertyType::UInt8:
            case PropertyType::UInt16:
            case PropertyType::UInt32:
            case PropertyType::UInt64:
            case PropertyType::Int8:
            case PropertyType::Int16:
            case PropertyType::Int32:
            case PropertyType::Int64:
            case PropertyType::Float:
            case PropertyType::Double:
            case PropertyType::Ascii:
            case PropertyType::Key:
            case PropertyType::Size:
                // value is already converted to ascii string --> printed
                renderer.WriteText(tmpAscii, params);
                break;
            case PropertyType::Unicode:
                renderer.WriteText(tmpUnicode, params);
                break;
            case PropertyType::UTF8:
                renderer.WriteText(tmpUTF8, params);
                break;
            case PropertyType::CharacterView:
                renderer.WriteText(tmpCharView, params);
                break;
            case PropertyType::Color:
                renderer.WriteSpecialCharacter(
                      params.X,
                      y,
                      SpecialChars::BlockCentered,
                      ColorPair{ std::get<Graphics::Color>(tempPropValue), Color::Transparent });
                params.X += 2;
                if (w > 2)
                {
                    params.Width -= 2;
                    renderer.WriteText(tmpAscii, params);
                }
                break;
            case PropertyType::Char8:
            case PropertyType::Char16:
                renderer.WriteCharacter(params.X, y, tmpCh16, params.Color);
                params.X += 2;
                if (w > 2)
                {
                    params.Width -= 2;
                    renderer.WriteText(tmpAscii, params);
                }
                break;
            case PropertyType::List:
                DrawListProperty(params, tempPropValue, prop, renderer, readOnly);
                break;
            case PropertyType::Flags:
                DrawFlagsProperty(params, tempPropValue, prop, renderer, readOnly);
                break;
            case PropertyType::Custom:
                DrawCustomProperty(params, tempPropValue, renderer, readOnly);
                break;

            default:
                renderer.WriteText("<Internal error - fail to process item type>", params);
                break;
            }
        }
    }
    else
    {
        if (w > 0)
        {
            params.Width = (uint32) w;
            params.Color = Colors.Item.Error;
            renderer.WriteText("<Unable to read item value>", params);
        }
    }
}
void PropertyListContext::DrawFilterBar(Graphics::Renderer& renderer)
{
    auto filterWidth = std::min<>(FILTER_PREFERED_WIDTH, this->Layout.Width - 7);
    renderer.FillHorizontalLine(2, this->Layout.Height - 1, 2 + filterWidth + 1, ' ', Cfg->PropertyList.Filter.Text);
    auto col = this->filteredMode ? Cfg->PropertyList.Filter.Focused : Cfg->PropertyList.Filter.Text;

    if (this->filterText.Len() <= filterWidth)
    {
        renderer.WriteSingleLineText(3, this->Layout.Height - 1, this->filterText, col);
        if (this->filteredMode)
            renderer.SetCursor(3 + this->filterText.Len(), this->Layout.Height - 1);
    }
    else
    {
        renderer.WriteSingleLineText(
              3,
              this->Layout.Height - 1,
              string_view{ this->filterText.GetText() + this->filterText.Len() - filterWidth, (size_t) filterWidth },
              col);
        if (this->filteredMode)
            renderer.SetCursor(3 + filterWidth, this->Layout.Height - 1);
    }
}
void PropertyListContext::Paint(Graphics::Renderer& renderer)
{
    uint32 value;
    int32 y             = 0;
    int32 max_y         = this->Layout.Height;
    auto c              = this->Cfg->PropertyList.Border;
    bool readOnlyStatus = false;

    if (this->propertyNameWidth == 0)
        this->SetPropertyNameWidth(this->Layout.Width * 4 / 10, false);

    if ((this->Flags & GATTR_ENABLE) == 0)
    {
        c                               = this->Cfg->PropertyList.Inactive;
        this->Colors.Category.Arrow     = c;
        this->Colors.Category.Stats     = c;
        this->Colors.Category.Text      = c;
        this->Colors.Item.LineSeparator = c;
        this->Colors.Item.ReadOnly      = c;
        this->Colors.Item.Text          = c;
        this->Colors.Item.Value         = c;
        this->Colors.Item.Checked       = c;
        this->Colors.Item.Unchecked     = c;
        this->Colors.Item.Error         = c;
    }
    else
    {
        this->Colors.Category = this->Cfg->PropertyList.Category;
        this->Colors.Item     = this->Cfg->PropertyList.Item;
    }
    renderer.Clear(' ', c);
    if (this->hasBorder)
    {
        renderer.DrawRectSize(0, 0, this->Layout.Width, this->Layout.Height, c, LineType::Single);
        if ((this->Focused) && (this->Layout.Width > 9))
            DrawFilterBar(renderer);
        y++;
        max_y--;
    }
    for (auto idx = this->startView; (idx < this->items.Len()) && (y < max_y); idx++, y++)
    {
        if (this->items.Get(idx, value) == false)
            break;
        if (value & CATEGORY_FLAG)
        {
            DrawCategory(value & CATEGORY_INDEX_MASK, y, renderer);
            readOnlyStatus = false;
        }
        else
            DrawProperty(value, y, renderer, readOnlyStatus);
        if ((this->Focused) && (idx == this->currentPos))
        {
            const auto col = readOnlyStatus ? Cfg->PropertyList.CursorReadOnly : Cfg->PropertyList.Cursor;
            if (this->hasBorder)
                renderer.FillHorizontalLine(1, y, this->Layout.Width - 2, -1, col);
            else
                renderer.FillHorizontalLine(0, y, this->Layout.Width, -1, col);
        }
    }
}
void PropertyListContext::SetPropertyNameWidth(int32 value, bool adjustPercentage)
{
    if (this->hasBorder)
        value = std::min<>(this->Layout.Width - 8, value);
    else
        value = std::min<>(this->Layout.Width - 6, value);
    this->propertyNameWidth = std::max<>(2, value);
    if (adjustPercentage)
        this->propetyNamePercentage = (float) this->propertyNameWidth / (float) (this->Layout.Width);
}
void PropertyListContext::MoveTo(uint32 newPos)
{
    if (this->items.Len() == 0)
    {
        this->currentPos = 0;
        this->startView  = 0;
        return;
    }
    if (newPos >= this->items.Len())
        newPos = this->items.Len() - 1;
    auto h = this->hasBorder ? this->Layout.Height - 2 : this->Layout.Height;
    if (h < 1)
        return; // sanity check
    uint32 height = (uint32) h;
    if ((startView <= newPos) && ((startView + height) > newPos))
    {
        this->currentPos = newPos;
        return;
    }
    // adjust start view --> if before scroll up
    if (newPos < startView)
    {
        this->currentPos = newPos;
        this->startView  = newPos;
        return;
    }
    // otherwise scroll down
    this->currentPos = newPos;
    if (newPos >= startView + height - 1)
        this->startView = newPos - (height - 1);
    else
        this->startView = 0;
}
void PropertyListContext::MoveScrollTo(uint32 newPos)
{
    auto h = this->hasBorder ? this->Layout.Height - 2 : this->Layout.Height;
    if (h < 1)
        return; // sanity check
    if (newPos + h <= items.Len())
        startView = newPos;
    else
    {
        if ((uint32) h > items.Len())
            startView = 0;
        else
            startView = items.Len() - h;
    }
    if (this->currentPos < startView)
        MoveTo(startView);
    else
    {
        if (items.Len() > 0)
        {
            auto last = startView + h - 1;
            if (last >= items.Len())
                last = items.Len() - 1;
            if (this->currentPos > last)
                MoveTo(last);
        }
    }
}
void PropertyListContext::MoveToPropetyIndex(uint32 idx)
{
    auto ptr = this->items.GetUInt32Array();
    for (auto pos = 0U; pos < this->items.Len(); pos++, ptr++)
    {
        if ((*ptr) == idx)
        {
            MoveTo(pos);
            return;
        }
    }
    // nothing found ==> move to first item
    MoveTo(0);
}
void PropertyListContext::EditAndUpdateText(const PropertyInfo& prop)
{
    PropertyTextEditDialog dlg(prop, object, IsPropertyReadOnly(prop));
    dlg.ShowDialog();
}
void PropertyListContext::EditAndUpdateList(const PropertyInfo& prop)
{
    PropertyListEditDialog dlg(prop, object, IsPropertyReadOnly(prop));
    dlg.ShowDialog();
}
void PropertyListContext::EditAndUpdateFlags(const PropertyInfo& prop)
{
    PropertyFlagsEditDialog dlg(prop, object, IsPropertyReadOnly(prop));
    dlg.ShowDialog();
}
void PropertyListContext::EditAndUpdateKey(const PropertyInfo& prop)
{
    PropertyKeyEditDialog dlg(prop, object, IsPropertyReadOnly(prop));
    dlg.ShowDialog();
}
void PropertyListContext::EditAndUpdateColor(const PropertyInfo& prop)
{
    PropertyColorEditDialog dlg(prop, object, IsPropertyReadOnly(prop));
    dlg.ShowDialog();
}
void PropertyListContext::EditAndUpdateChar(const PropertyInfo& prop, bool isChar8)
{
    PropertyCharEditDialog dlg(prop, object, IsPropertyReadOnly(prop), isChar8);
    dlg.ShowDialog();
}
void PropertyListContext::EditAndUpdateBool(const PropertyInfo& prop)
{
    if (IsPropertyReadOnly(prop))
        return;
    if (this->object->GetPropertyValue(prop.id, tempPropValue))
    {
        LocalString<256> error;
        if (this->object->SetPropertyValue(prop.id, !(std::get<bool>(tempPropValue)), error) == false)
        {
            Dialogs::MessageBox::ShowError("Error", error);
        }
    }
    else
    {
        Dialogs::MessageBox::ShowError("Error", "Fail to retrive object value");
    }
}
void PropertyListContext::ExecuteItemAction()
{
    uint32 idx;
    if (this->items.Get(this->currentPos, idx))
    {
        if (idx & CATEGORY_FLAG)
        {
            idx &= CATEGORY_INDEX_MASK;
            this->categories[idx].folded = !this->categories[idx].folded;
            Refilter();
        }
        else
        {
            switch (this->properties[idx].type)
            {
            case PropertyType::Boolean:
                EditAndUpdateBool(this->properties[idx]);
                break;
            case PropertyType::Char8:
            case PropertyType::Char16:
                EditAndUpdateChar(this->properties[idx], this->properties[idx].type == PropertyType::Char8);
                break;
            case PropertyType::UInt8:
            case PropertyType::UInt16:
            case PropertyType::UInt32:
            case PropertyType::UInt64:
            case PropertyType::Int8:
            case PropertyType::Int16:
            case PropertyType::Int32:
            case PropertyType::Int64:
            case PropertyType::Float:
            case PropertyType::Double:
            case PropertyType::Ascii:
            case PropertyType::Unicode:
            case PropertyType::UTF8:
            case PropertyType::CharacterView:
            case PropertyType::Size:
                EditAndUpdateText(this->properties[idx]);
                break;
            case PropertyType::Color:
                EditAndUpdateColor(this->properties[idx]);
                break;
            case PropertyType::Key:
                EditAndUpdateKey(this->properties[idx]);
                break;

            case PropertyType::List:
                EditAndUpdateList(this->properties[idx]);
                break;
            case PropertyType::Flags:
                EditAndUpdateFlags(this->properties[idx]);
                break;
            case PropertyType::Custom:
                object->SetCustomPropetyValue(this->properties[idx].id);
                break;
            }
        }
    }
}
bool PropertyListContext::ProcessFilterKey(Input::Key keyCode, char16 UnicodeChar)
{
    uint32 idx;
    auto hasIndex = this->items.Get(this->currentPos, idx);
    if ((UnicodeChar >= 32) && (UnicodeChar < 127))
    {
        this->filterText.AddChar((char) UnicodeChar);
        Refilter();
        if (hasIndex)
            MoveToPropetyIndex(idx);
        else
            MoveTo(0); // first index
        this->filteredMode = true;
        return true;
    }
    if (keyCode == Key::Backspace)
    {
        if (this->filterText.Len())
        {
            this->filterText.Truncate(this->filterText.Len() - 1);
            Refilter();
            if (hasIndex)
                MoveToPropetyIndex(idx);
            else
                MoveTo(0); // first index
        }
        this->filteredMode = true;
        return true;
    }
    if ((keyCode == Key::Escape) && (this->filteredMode))
    {
        this->filteredMode = false;
        return true;
    }
    return false;
}
bool PropertyListContext::OnKeyEvent(Input::Key keyCode, char16 UnicodeChar)
{
    auto h = this->hasBorder ? this->Layout.Height - 3 : this->Layout.Height - 1;

    switch (keyCode)
    {
    case Key::Up:
        if (this->currentPos > 0)
            MoveTo(this->currentPos - 1);
        return true;
    case Key::Down:
        if (this->currentPos + 1 < this->items.Len())
            MoveTo(this->currentPos + 1);
        return true;
    case Key::Home:
        MoveTo(0);
        return true;
    case Key::End:
        MoveTo(this->items.Len());
        return true;
    case Key::PageUp:
        if ((h >= 1) && (((uint32) h) > this->currentPos))
            MoveTo(this->currentPos - (uint32) h);
        else
            MoveTo(0);
        return true;
    case Key::PageDown:
        MoveTo(this->currentPos + (uint32) std::max<int>(1, h));
        return true;
    case Key::Left:
        SetPropertyNameWidth(this->propertyNameWidth - 1, true);
        return true;
    case Key::Right:
        SetPropertyNameWidth(this->propertyNameWidth + 1, true);
        return true;
    case Key::Space:
    case Key::Enter:
        if (!this->filteredMode)
        {
            ExecuteItemAction();
            return true;
        }
        break; // else allow it to be process by ProcessFilterKey
    }

    if (ProcessFilterKey(keyCode, UnicodeChar))
        return true;
    return false;
}
bool PropertyListContext::OnMouseWheel(int /*x*/, int /*y*/, Input::MouseWheel direction)
{
    auto currentStartView = this->startView;
    switch (direction)
    {
    case MouseWheel::Up:
        if (this->startView > 0)
            MoveScrollTo(this->startView - 1);
        // if scroll did not changed, move the cursor
        if ((currentStartView == this->startView) && (this->currentPos > 0))
            MoveTo(this->currentPos - 1);
        return true;
    case MouseWheel::Down:
        MoveScrollTo(this->startView + 1);
        // if scroll did not changed, move the cursor
        if (currentStartView == this->startView)
            MoveTo(this->currentPos + 1);
        return true;
    }
    return false;
}
bool PropertyListContext::MouseToItem(int x, int y, uint32& itemIndex, PropertyItemLocation& loc)
{
    // check for item click
    auto y_poz = this->hasBorder ? y - 1 : y;
    auto x_poz = this->hasBorder ? x - 1 : x;
    if ((y_poz >= 0) && (((uint32) y_poz) + startView < this->items.Len()))
    {
        loc       = PropertyItemLocation::None;
        itemIndex = ((uint32) y_poz) + startView;
        uint32 value;
        if (this->items.Get(itemIndex, value))
        {
            if (value & CATEGORY_FLAG)
            {
                // category
                value = value & CATEGORY_INDEX_MASK;
                if (x_poz == 0)
                    loc = PropertyItemLocation::CollapseExpandButton;
            }
            else
            {
                // normal item
            }
        }
        return true;
    }
    return false;
}
void PropertyListContext::OnMousePressed(int x, int y, Input::MouseButton button)
{
    // check for filter mode
    if ((this->hasBorder) && (y == this->Layout.Height - 1))
    {
        this->filteredMode = true;
        return;
    }
    // check separator
    if (x == GetSeparatorXPos())
    {
        this->separatorStatus = PropertySeparatorStatus::Drag;
        return;
    }
    // check for item click
    uint32 idx;
    PropertyItemLocation loc;
    if (MouseToItem(x, y, idx, loc))
    {
        // click on one item
        this->filteredMode = false;
        MoveTo(idx);
        if (((button & MouseButton::DoubleClicked) != MouseButton::None) || (loc != PropertyItemLocation::None))
            ExecuteItemAction();
        return;
    }
}

bool PropertyListContext::OnMouseOver(int x, int y, PropertyItemLocation& loc)
{
    if (GetSeparatorXPos() == x)
    {
        if (separatorStatus == PropertySeparatorStatus::None)
        {
            separatorStatus = PropertySeparatorStatus::Over;
            loc             = PropertyItemLocation::None;
            return true;
        }
    }
    else
    {
        auto result = false;
        if (separatorStatus == PropertySeparatorStatus::Over)
        {
            separatorStatus = PropertySeparatorStatus::None;
            loc             = PropertyItemLocation::None;
            result          = true;
        }
        uint32 idx;
        if (MouseToItem(x, y, idx, loc))
        {
            if (idx == this->hoveredItemIDX)
            {
                if (this->hoveredItemStatus != loc)
                {
                    this->hoveredItemStatus = loc;
                    result                  = true;
                }
            }
            else
            {
                this->hoveredItemIDX    = idx;
                this->hoveredItemStatus = loc;
                result                  = true;
            }
        }
        else
        {
            if (this->hoveredItemStatus != PropertyItemLocation::None)
            {
                this->hoveredItemIDX    = INVALID_ITEM;
                this->hoveredItemStatus = PropertyItemLocation::None;
                result                  = true;
            }
            loc = PropertyItemLocation::None;
        }
        return result;
    }
    return false;
}

bool PropertyListContext::OnMouseDrag(int x, int /*y*/, Input::MouseButton /*button*/)
{
    if (separatorStatus == PropertySeparatorStatus::Drag)
    {
        this->SetPropertyNameWidth(this->propertyNameWidth + (x - this->GetSeparatorXPos()), true);
        return true;
    }
    return false;
}
bool PropertyListContext::OnMouseLeave()
{
    if ((this->separatorStatus != PropertySeparatorStatus::None) ||
        (this->hoveredItemStatus != PropertyItemLocation::None))
    {
        this->separatorStatus   = PropertySeparatorStatus::None;
        this->hoveredItemStatus = PropertyItemLocation::None;
        this->hoveredItemIDX    = INVALID_ITEM;
        return true;
    }
    return false;
}
void PropertyListContext::OnMouseReleased(int /*x*/, int /*y*/, Input::MouseButton /*button*/)
{
    this->separatorStatus = PropertySeparatorStatus::None;
}
bool PropertyListContext::IsItemFiltered(const PropertyInfo& p)
{
    if (this->filterText.Empty())
        return true;
    return String::Contains(p.name.GetText(), this->filterText.GetText(), true);
}
void PropertyListContext::Refilter()
{
    this->items.Clear();
    for (uint32 i = 0U; i < this->properties.size(); i++)
    {
        if (IsItemFiltered(this->properties[i]))
            this->items.Push(i);
    }
    // sort the data
    if (this->showCategories)
    {
        this->items.Sort(SortWithCategories, true, this);
        // clear categories filtered items;
        for (auto& cat : this->categories)
            cat.filteredItems = 0;
        // insert categories
        uint32 idx      = 0;
        uint32 value    = 0;
        uint32 last_cat = 0xFFFFFFFF;
        while (idx < this->items.Len())
        {
            if (this->items.Get(idx, value))
            {
                if (this->properties[value].category != last_cat)
                {
                    last_cat = this->properties[value].category;
                    this->items.Insert(idx, last_cat | CATEGORY_FLAG);
                }
                else
                {
                    this->categories[this->properties[value].category].filteredItems++;
                }
            }
            idx++;
        }
        // fold categories if case
        idx = 0;
        while (idx < this->items.Len())
        {
            if ((this->items.Get(idx, value)) && (value & CATEGORY_FLAG) &&
                (this->categories[value & CATEGORY_INDEX_MASK].folded))
            {
                // search for the next category
                uint32 next = idx + 1;
                while ((next < this->items.Len()) && (this->items.Get(next, value)) && ((value & CATEGORY_FLAG) == 0))
                    next++;
                if (next > (idx + 1))
                {
                    this->items.Delete(idx + 1, (next - (idx + 1)));
                }
            }
            idx++;
        }
    }
    else
    {
        this->items.Sort(SortWithoutCategories, true, this);
    }
}

PropertyList::PropertyList(string_view layout, Reference<PropertiesInterface> obj, PropertyListFlags flags)
    : Control(new PropertyListContext(), "", layout, false)
{
    auto* Members = (PropertyListContext*) this->Context;
    Members->properties.reserve(64);
    Members->categories.reserve(8);
    Members->showCategories            = (flags & PropertyListFlags::HideCategories) == PropertyListFlags::None;
    Members->hasBorder                 = (flags & PropertyListFlags::Border) != PropertyListFlags::None;
    Members->propertyNameWidth         = 0;
    Members->startView                 = 0;
    Members->currentPos                = 0;
    Members->Flags                     = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | GATTR_VSCROLL | (uint32) flags;
    Members->Layout.MinWidth           = 10; // 3 spaces+2chars(name)+1char(bar)+2chars(value)+2chars(border)
    Members->Layout.MinHeight          = 4;
    Members->propetyNamePercentage     = 0.4f; // 40% of width is the property name
    Members->filteredMode              = false;
    Members->separatorStatus           = PropertySeparatorStatus::None;
    Members->hoveredItemStatus         = PropertyItemLocation::None;
    Members->hoveredItemIDX            = INVALID_ITEM;
    Members->ScrollBars.OutsideControl = !Members->hasBorder;

    SetObject(obj);
}
void PropertyList::SetObject(Reference<PropertiesInterface> obj)
{
    auto* Members   = (PropertyListContext*) this->Context;
    Members->object = obj;
    Members->categories.clear();
    Members->properties.clear();

    if (obj.IsValid())
    {
        std::map<string_view, uint32> s;
        // items
        for (auto& e : obj->GetPropertiesList())
        {
            auto& pi = Members->properties.emplace_back();
            pi.name  = e.name;
            pi.type  = e.type;
            pi.id    = e.id;
            if ((pi.type == PropertyType::Flags) || (pi.type == PropertyType::List) ||
                (pi.type == PropertyType::Boolean))
            {
                LocalUnicodeStringBuilder<1024> tempValues(e.values);
                // we need to process list of flags/values
                ListItemsParser parser(tempValues.GetString(), tempValues.GetString() + tempValues.Len());
                auto result = false;
                if (pi.type == PropertyType::Boolean)
                    result = parser.CreateBooleanValues(pi.listValues);
                else
                    result = parser.CreateDictionary(pi.listValues);
                if (!result)
                {
                    // clear all data
                    pi.listValues.clear();
                }
            }
            auto it = s.find(e.category);
            if (it != s.cend())
            {
                pi.category = it->second;
                Members->categories[pi.category].totalItems++;
            }
            else
            {
                pi.category   = (uint32) Members->categories.size();
                s[e.category] = pi.category;
                // add categoy
                auto& cat         = Members->categories.emplace_back();
                cat.filteredItems = 0; // it will be recomputed on Members->Refilter()
                cat.totalItems    = 1;
                cat.folded        = false;
                cat.name          = e.category;
            }
        }
    }

    Members->items.Resize((uint32) Members->properties.size() * 2); // assume that each item has its own category
    Members->Refilter();
}
PropertyList::~PropertyList()
{
}
void PropertyList::Paint(Graphics::Renderer& renderer)
{
    reinterpret_cast<PropertyListContext*>(this->Context)->Paint(renderer);
}

void PropertyList::OnAfterResize(int newWidth, int)
{
    auto* Members = (PropertyListContext*) this->Context;
    if (Members->propertyNameWidth == 0)
        Members->SetPropertyNameWidth((int32) (newWidth * Members->propetyNamePercentage), false);
    else
        Members->SetPropertyNameWidth((int32) (newWidth * Members->propetyNamePercentage), false);
}
bool PropertyList::OnKeyEvent(Input::Key keyCode, char16 UnicodeChar)
{
    return reinterpret_cast<PropertyListContext*>(this->Context)->OnKeyEvent(keyCode, UnicodeChar);
}
void PropertyList::OnMouseReleased(int x, int y, Input::MouseButton button)
{
    reinterpret_cast<PropertyListContext*>(this->Context)->OnMouseReleased(x, y, button);
}
void PropertyList::OnMousePressed(int x, int y, Input::MouseButton button)
{
    reinterpret_cast<PropertyListContext*>(this->Context)->OnMousePressed(x, y, button);
}
bool PropertyList::OnMouseDrag(int x, int y, Input::MouseButton button)
{
    return reinterpret_cast<PropertyListContext*>(this->Context)->OnMouseDrag(x, y, button);
}
bool PropertyList::OnMouseWheel(int x, int y, Input::MouseWheel direction)
{
    return reinterpret_cast<PropertyListContext*>(this->Context)->OnMouseWheel(x, y, direction);
}
bool PropertyList::OnMouseOver(int x, int y)
{
    PropertyItemLocation loc;
    if (reinterpret_cast<PropertyListContext*>(this->Context)->OnMouseOver(x, y, loc) == false)
        return false;
    switch (loc)
    {
    case PropertyItemLocation::None:
        this->HideToolTip();
        break;
    case PropertyItemLocation::CollapseExpandButton:
        this->ShowToolTip("Click to expand/collapse category", x, y);
        break;
    default:
        this->HideToolTip();
        break;
    }
    return true;
}
bool PropertyList::OnMouseLeave()
{
    return reinterpret_cast<PropertyListContext*>(this->Context)->OnMouseLeave();
}

void PropertyList::OnUpdateScrollBars()
{
    auto* Members = (PropertyListContext*) this->Context;

    uint32 count = Members->items.Len();
    if (count > 0)
        count--;
    UpdateVScrollBar(Members->currentPos, count);
}

} // namespace AppCUI