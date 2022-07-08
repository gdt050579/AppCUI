#pragma once

#include "../../Internal.hpp"

namespace AppCUI
{
namespace Internal
{
    class TestTerminal : public AbstractTerminal
    {
      public:
        TestTerminal();
        virtual bool OnInit(const Application::InitializationData& initData) override;
        virtual void RestoreOriginalConsoleSettings() override;
        virtual void OnUninit() override;
        virtual void OnFlushToScreen() override;
        virtual void OnFlushToScreen(const Graphics::Rect& r) override;
        virtual bool OnUpdateCursor() override;
        virtual void GetSystemEvent(Internal::SystemEvent& evnt) override;
        virtual bool IsEventAvailable() override;
        virtual bool HasSupportFor(Application::SpecialCharacterSetType type) override;
        virtual ~TestTerminal();
    };
} // namespace Internal
} // namespace AppCUI
