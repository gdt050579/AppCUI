#include "AppCUI.hpp"

namespace AppCUI
{
using namespace OS;
using namespace Utils;

Buffer File::ReadContent(const std::filesystem::path& path)
{
    File f;
    CHECK(f.OpenRead(path), Buffer(), "Fail to open: %s", path.string().c_str());
    CHECK(f.SetCurrentPos(0),
          Buffer(),
          "Fail to position the current pointer to the start of the file: %s",
          path.string().c_str());
    auto file_size = f.GetSize();
    CHECK(file_size > 0, Buffer(), "Empty file (%s)!", path.string().c_str());
    CHECK(file_size < 0xFFFFFFF, Buffer(), "File size exceed 0xFFFFF bytes (%s)", path.string().c_str());
    Buffer buf;
    buf.Resize(file_size);
    CHECK(f.Read(buf.GetData(), (uint32) file_size),
          Buffer(),
          "Fail to read %u bytes from the file %s",
          (uint32) file_size,
          path.string().c_str());
    f.Close();
    return buf;
}
bool File::WriteContent(const std::filesystem::path& path, BufferView buf)
{
    File f;
    CHECK(buf.GetLength() < 0xFFFFFFF,
          false,
          "Buffer size exceed 0xFFFFF bytes --> cannot create (%s)",
          path.string().c_str());
    CHECK(f.Create(path, true), false, "Fail to create: %s", path.string().c_str());
    if (buf.Empty())
    {
        f.Close(); // empty file
        return true;
    }
    CHECK(f.Write(buf.GetData(), (uint32) buf.GetLength()),
          false,
          "Fail to write %u bytes into %s",
          (uint32) buf.GetLength(),
          path.string().c_str());
    f.Close();
    return true;
}
bool File::WriteContent(const std::filesystem::path& path, string_view text)
{
    auto buf_ptr = static_cast<const void*>(text.data());
    auto buf_sz  = static_cast<size_t>(text.length());
    return WriteContent(path, BufferView(buf_ptr, buf_sz));
}
} // namespace AppCUI