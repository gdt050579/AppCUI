#include "Internal.hpp"

namespace AppCUI
{
using namespace Input;
using namespace Utils;

namespace
{
    // modifier index layout (see KeyUtils::KEY_SHIFT_BITS): bit 0 = Alt, bit 1 = Ctrl, bit 2 = Shift
    constexpr uint32 MODIFIER_BIT_ALT   = 1;
    constexpr uint32 MODIFIER_BIT_CTRL  = 2;
    constexpr uint32 MODIFIER_BIT_SHIFT = 4;

    inline uint32 ModifierIndex(Key k)
    {
        return (static_cast<uint32>(k) >> KeyUtils::KEY_SHIFT_BITS) & 7;
    }
    inline Key WithModifierIndex(Key k, uint32 index)
    {
        return static_cast<Key>((static_cast<uint32>(k) & ~KeyUtils::KEY_SHIFT_MASK) | (index << KeyUtils::KEY_SHIFT_BITS));
    }
    inline Key IndexToModifier(uint32 index)
    {
        return static_cast<Key>(index << KeyUtils::KEY_SHIFT_BITS);
    }

    ModifierMap globalModifierMap;

    // "Ctrl", "Alt", "Ctrl+Shift", ... (no trailing '+'); used for the [AppCUI] Keyboard.* settings
    void ModifiersToText(Key modifiers, LocalString<32>& text)
    {
        const uint32 index = ModifierIndex(modifiers);
        text.Clear();
        if (index & MODIFIER_BIT_CTRL)
            text.Add("Ctrl+");
        if (index & MODIFIER_BIT_ALT)
            text.Add("Alt+");
        if (index & MODIFIER_BIT_SHIFT)
            text.Add("Shift+");
        if (text.Len() > 0)
            text.Truncate(text.Len() - 1);
    }
    bool EqualsIgnoreCase(string_view a, string_view b)
    {
        if (a.size() != b.size())
            return false;
        for (size_t i = 0; i < a.size(); i++)
        {
            auto ca = a[i], cb = b[i];
            if ((ca >= 'A') && (ca <= 'Z'))
                ca |= 0x20;
            if ((cb >= 'A') && (cb <= 'Z'))
                cb |= 0x20;
            if (ca != cb)
                return false;
        }
        return true;
    }
    std::optional<Key> ModifiersFromText(string_view text)
    {
        uint32 result = 0;
        while (!text.empty())
        {
            auto pos  = text.find('+');
            auto part = text.substr(0, pos);
            if (EqualsIgnoreCase(part, "Ctrl"))
                result |= MODIFIER_BIT_CTRL;
            else if (EqualsIgnoreCase(part, "Alt") || EqualsIgnoreCase(part, "Opt") || EqualsIgnoreCase(part, "Option"))
                result |= MODIFIER_BIT_ALT;
            else if (EqualsIgnoreCase(part, "Shift"))
                result |= MODIFIER_BIT_SHIFT;
            else
                return std::nullopt;
            if (pos == string_view::npos)
                break;
            text = text.substr(pos + 1);
        }
        if (result == 0)
            return std::nullopt;
        return IndexToModifier(result);
    }
} // namespace

//====================================================================================== ModifierMap ===
ModifierMap::ModifierMap() : identity(true)
{
    for (uint8 i = 0; i < 8; i++)
    {
        toLogical[i]  = i;
        toPhysical[i] = i;
    }
}
ModifierMap ModifierMap::Identity()
{
    return ModifierMap();
}
std::optional<ModifierMap> ModifierMap::FromAssignments(Key ctrlActsAs, Key altActsAs)
{
    const uint32 ctrlImage = ModifierIndex(ctrlActsAs);
    const uint32 altImage  = ModifierIndex(altActsAs);
    // only modifier bits are allowed (no key code) and both physical modifiers must map to something
    if (((static_cast<uint32>(ctrlActsAs) | static_cast<uint32>(altActsAs)) & KeyUtils::KEY_CODE_MASK) != 0)
        return std::nullopt;
    if ((ctrlImage == 0) || (altImage == 0))
        return std::nullopt;

    ModifierMap result;
    bool used[8] = {};
    for (uint32 physical = 0; physical < 8; physical++)
    {
        uint32 logical = 0;
        if (physical & MODIFIER_BIT_ALT)
            logical |= altImage;
        if (physical & MODIFIER_BIT_CTRL)
            logical |= ctrlImage;
        if (physical & MODIFIER_BIT_SHIFT)
            logical |= MODIFIER_BIT_SHIFT;
        if (used[logical])
            return std::nullopt; // not a bijection -> some shortcuts would become unreachable
        used[logical]                = true;
        result.toLogical[physical]   = static_cast<uint8>(logical);
        result.toPhysical[logical]   = static_cast<uint8>(physical);
    }
    result.identity = true;
    for (uint32 i = 0; i < 8; i++)
        if (result.toLogical[i] != i)
            result.identity = false;
    return result;
}
Key ModifierMap::ToLogical(Key physical) const
{
    if (identity)
        return physical;
    return WithModifierIndex(physical, toLogical[ModifierIndex(physical)]);
}
Key ModifierMap::ToPhysical(Key logical) const
{
    if (identity)
        return logical;
    return WithModifierIndex(logical, toPhysical[ModifierIndex(logical)]);
}
Key ModifierMap::GetCtrlActsAs() const
{
    return IndexToModifier(toLogical[MODIFIER_BIT_CTRL]);
}
Key ModifierMap::GetAltActsAs() const
{
    return IndexToModifier(toLogical[MODIFIER_BIT_ALT]);
}

//====================================================================================== KeyMap ===
KeyMap::KeyMap() : slots(nullptr), capacity(0), count(0), mask(0)
{
}
KeyMap::~KeyMap()
{
    delete[] slots;
}
void KeyMap::Clear()
{
    // keeps the allocated table (bindings are rebuilt with roughly the same number of keys)
    for (uint32 idx = 0; idx < capacity; idx++)
        slots[idx] = Slot{ 0, 0, 0 };
    count = 0;
}
static inline uint32 HashKey(uint32 key)
{
    return key * 2654435761u; // Knuth multiplicative hash
}
const KeyMap::Slot* KeyMap::Find(uint32 key) const
{
    // key 0 (Key::None) marks an empty slot and is never a valid binding
    if ((count == 0) || (key == 0))
        return nullptr;
    for (uint32 idx = HashKey(key) & mask;; idx = (idx + 1) & mask)
    {
        const Slot& s = slots[idx];
        if (s.key == key)
            return &s;
        if (s.key == 0)
            return nullptr; // load factor <= 50% guarantees an empty slot
    }
}
void KeyMap::Rehash(uint32 newCapacity)
{
    // allocate first: if the allocation fails the current table is left untouched
    Slot* fresh            = new Slot[newCapacity]{}; // value-initialized -> key 0 = empty slot
    const uint32 freshMask = newCapacity - 1;
    for (uint32 i = 0; i < capacity; i++)
    {
        const Slot& s = slots[i];
        if (s.key == 0)
            continue;
        uint32 idx = HashKey(s.key) & freshMask;
        while (fresh[idx].key != 0)
            idx = (idx + 1) & freshMask;
        fresh[idx] = s;
    }
    delete[] slots;
    slots    = fresh;
    capacity = newCapacity;
    mask     = freshMask;
}
void KeyMap::Add(const KeyBinding& binding)
{
    const uint32 key = static_cast<uint32>(binding.Key);
    if (key == 0)
        return; // unassigned
    if (capacity == 0)
        Rehash(16);
    else if ((count + 1) * 2 > capacity)
        Rehash(capacity * 2);
    uint32 idx = HashKey(key) & mask;
    while (slots[idx].key != 0)
    {
        if (slots[idx].key == key)
            return; // first binding wins
        idx = (idx + 1) & mask;
    }
    slots[idx] = Slot{ key, binding.CommandId, static_cast<uint8>(binding.Flags) };
    count++;
}
KeyMap::Result KeyMap::Resolve(Key keyCode) const
{
    const uint32 key = static_cast<uint32>(keyCode);
    if (const Slot* s = Find(key))
        return { s->commandId, false };
    const uint32 shift = static_cast<uint32>(Key::Shift);
    if (key & shift)
    {
        const Slot* s = Find(key & ~shift);
        if ((s) && (s->flags & static_cast<uint8>(KeyBindingFlags::ShiftExtendsSelection)))
            return { s->commandId, true };
    }
    return { NO_COMMAND, false };
}

//====================================================================================== Application ===
const ModifierMap& Application::GetModifierMap()
{
    return globalModifierMap;
}
void Application::SetModifierMap(const ModifierMap& map)
{
    globalModifierMap = map;
    auto ini          = Application::GetAppSettings();
    if (ini)
    {
        LocalString<32> tmp;
        auto sect = (*ini)["AppCUI"];
        ModifiersToText(map.GetCtrlActsAs(), tmp);
        sect.UpdateValue("Keyboard.Ctrl", tmp.ToStringView(), false);
        ModifiersToText(map.GetAltActsAs(), tmp);
        sect.UpdateValue("Keyboard.Alt", tmp.ToStringView(), false);
    }
}
void Internal::ResetKeyboardSettings()
{
    globalModifierMap = ModifierMap::Identity();
}
void Internal::LoadKeyboardSettings(Utils::IniSection section)
{
    ResetKeyboardSettings();
    if (!section.Exists())
        return;
    if ((!section.HasValue("Keyboard.Ctrl")) && (!section.HasValue("Keyboard.Alt")))
        return; // default profile
    auto ctrlText = section.HasValue("Keyboard.Ctrl") ? section.GetValue("Keyboard.Ctrl").ToStringView() : string_view();
    auto altText  = section.HasValue("Keyboard.Alt") ? section.GetValue("Keyboard.Alt").ToStringView() : string_view();
    auto ctrlAs   = ModifiersFromText(ctrlText.empty() ? string_view("Ctrl") : ctrlText);
    auto altAs    = ModifiersFromText(altText.empty() ? string_view("Alt") : altText);
    if ((!ctrlAs.has_value()) || (!altAs.has_value()))
    {
        LOG_WARNING("Invalid [AppCUI] Keyboard.Ctrl / Keyboard.Alt values ==> using the default keyboard profile");
        return;
    }
    auto map = ModifierMap::FromAssignments(ctrlAs.value(), altAs.value());
    if (!map.has_value())
    {
        LOG_WARNING("[AppCUI] Keyboard.Ctrl / Keyboard.Alt is not a valid (bijective) mapping ==> using the default keyboard profile");
        return;
    }
    globalModifierMap = map.value();
}
} // namespace AppCUI
