#include <AppCUI.hpp>

namespace AppCUI::Graphics
{
    using namespace AppCUI::Utils;
optional<Size> Size::FromString(string_view text)
{
    auto* start = text.data();
    auto* end   = start + text.length();
    CHECK(start, std::nullopt, "Expecting a non-null value for size");
    CHECK(text.length() >= 3, std::nullopt, "Value (%s) is too small (expecting at least 3 chars <width>x<height>", start);
    uint32 sz    = 0;
    auto p_width = Number::ToUInt16(
          string_view(start, end - start), NumberParseFlags::Base10 | NumberParseFlags::TrimSpaces, &sz);
    CHECK(p_width.has_value(),
          std::nullopt,
          "Expecting a valid format for size - eithed 'width x height' or 'width , height'");
    uint32 width = p_width.value();
    start += sz;
    CHECK(start < end,
          std::nullopt,
          "Expecting a valid format for size - eithed 'width x height' or 'width , height' --> Missing height value !");
    CHECK((*start == 'x') || (*start == 'X') || (*start == ','),
          std::nullopt,
          "Invalid format for size --> expcting either a 'x' or ',' after the width");
    start++;
    CHECK(start < end,
          std::nullopt,
          "Expecting a valid format for size - eithed 'width x height' or 'width , height' --> Missing height value !");
    auto p_height =
          Number::ToUInt16(string_view(start, end - start), NumberParseFlags::Base10 | NumberParseFlags::TrimSpaces);
    CHECK(p_height.has_value(),
          std::nullopt,
          "Expecting a valid format for size - eithed 'width x height' or 'width , height' -> height value is invalid");
    uint32 height = p_height.value();
    CHECK(width > 0, std::nullopt, "Width must be bigger than 0");
    CHECK(height > 0, std::nullopt, "Height must be bigger than 0");
    return Graphics::Size(width, height);
}
} // namespace AppCUI::Utils