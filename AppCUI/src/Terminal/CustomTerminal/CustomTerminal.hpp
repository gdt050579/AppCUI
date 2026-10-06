#pragma once

#include "../../Internal.hpp"
#include <chrono>

namespace AppCUI
{
namespace Internal
{
    // Adapter between AppCUI and a frontend implemented by the application (Application::CustomFrontendInterface)
    class CustomTerminal : public AbstractTerminal
    {
        Application::CustomFrontendInterface* frontend;
        std::chrono::steady_clock::time_point lastFramesUpdate;
        bool fpsMode;

      public:
        CustomTerminal();

        virtual bool OnInit(const Application::InitializationData& initData) override;
        virtual void RestoreOriginalConsoleSettings() override;
        virtual void OnUnInit() override;
        virtual void OnFlushToScreen() override;
        virtual void OnFlushToScreen(const Graphics::Rect& r) override;
        virtual bool OnUpdateCursor() override;
        virtual void GetSystemEvent(Internal::SystemEvent& evnt) override;
        virtual bool IsEventAvailable() override;
        virtual bool HasSupportFor(Application::SpecialCharacterSetType type) override;
        virtual ~CustomTerminal();
    };
} // namespace Internal
} // namespace AppCUI
