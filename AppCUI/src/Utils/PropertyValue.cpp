#include "AppCUI.hpp"

#define COMPILE_ERROR_MSG "Internal compiler error --> Increase PropertyValue::buffer size !"

namespace AppCUI::Utils
{

static_assert(sizeof(PropertyValue) > sizeof(uint64) + 1, COMPILE_ERROR_MSG);
static_assert(sizeof(PropertyValue) > sizeof(string_view) + 1, COMPILE_ERROR_MSG);
static_assert(sizeof(PropertyValue) > sizeof(u16string_view) + 1, COMPILE_ERROR_MSG);

constexpr uint8 PROPERTY_VALUE_NONE             = 0;
constexpr uint8 PROPERTY_VALUE_BOOLEAN          = 1;
constexpr uint8 PROPERTY_VALUE_UNSIGNED_INTEGER = 2;
constexpr uint8 PROPERTY_VALUE_SIGNED_INTEGER   = 3;
constexpr uint8 PROPERTY_VALUE_STRING_ASCII     = 4;
constexpr uint8 PROPERTY_VALUE_STRING_UNICODE   = 5;

void CopyPropertyValue(uint8* buffer, void* source, size_t sourceSize, uint8 type)
{
    if ((sourceSize + 1) >= sizeof(PropertyValue))
    {
        *buffer = PROPERTY_VALUE_NONE;
        return;
    }
    memcpy(buffer + 1, source, sourceSize);
    *buffer = type;
}

PropertyValue::PropertyValue()
{
    this->buffer[0] = PROPERTY_VALUE_NONE;
}
void PropertyValue::operator=(uint64 value)
{
    CopyPropertyValue(this->buffer, &value, sizeof(uint64), PROPERTY_VALUE_UNSIGNED_INTEGER);
}
void PropertyValue::operator=(uint32 value32)
{
    uint64 value = value32;
    CopyPropertyValue(this->buffer, &value, sizeof(uint64), PROPERTY_VALUE_UNSIGNED_INTEGER);
}
void PropertyValue::operator=(uint16 value16)
{
    uint64 value = value16;
    CopyPropertyValue(this->buffer, &value, sizeof(uint64), PROPERTY_VALUE_UNSIGNED_INTEGER);
}
void PropertyValue::operator=(uint8 value8)
{
    uint64 value = value8;
    CopyPropertyValue(this->buffer, &value, sizeof(uint64), PROPERTY_VALUE_UNSIGNED_INTEGER);
}
void PropertyValue::operator=(int64 value)
{
    CopyPropertyValue(this->buffer, &value, sizeof(int64), PROPERTY_VALUE_SIGNED_INTEGER);
}
void PropertyValue::operator=(int32 value32)
{
    int64 value = value32;
    CopyPropertyValue(this->buffer, &value, sizeof(int64), PROPERTY_VALUE_SIGNED_INTEGER);
}
void PropertyValue::operator=(int16 value16)
{
    int64 value = value16;
    CopyPropertyValue(this->buffer, &value, sizeof(int64), PROPERTY_VALUE_SIGNED_INTEGER);
}
void PropertyValue::operator=(int8 value8)
{
    int64 value = value8;
    CopyPropertyValue(this->buffer, &value, sizeof(int64), PROPERTY_VALUE_SIGNED_INTEGER);
}
void PropertyValue::operator=(bool value)
{
    this->buffer[0] = PROPERTY_VALUE_BOOLEAN;
    this->buffer[1] = value ? 1 : 0;
}
void PropertyValue::operator=(string_view value)
{
    CopyPropertyValue(this->buffer, &value, sizeof(string_view), PROPERTY_VALUE_STRING_ASCII);
}
void PropertyValue::operator=(u16string_view value)
{
    CopyPropertyValue(this->buffer, &value, sizeof(u16string_view), PROPERTY_VALUE_STRING_UNICODE);
}
} // namespace AppCUI::Utils

#undef COMPILE_ERROR_MSG