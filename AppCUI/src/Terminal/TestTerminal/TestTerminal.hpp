#pragma once

#include "../../Internal.hpp"
#include <queue>

namespace AppCUI
{
namespace Internal
{
    class TestTerminal : public AbstractTerminal
    {
        std::queue<SystemEvent> eventsQueue;
      public:
        TestTerminal();

        bool CreateEventsQueue(std::string_view commandsScript);

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
