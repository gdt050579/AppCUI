#pragma once

#include "AppCUI.hpp"

class State
{
  public:
    State() = default;

    State(const State& other)     = default;
    State(State&& other) noexcept = default;
    State& operator=(const State& other) = default;
    State& operator=(State&& other) noexcept = default;

    virtual ~State() = default;

    virtual bool HandleEvent(AppCUI::Controls::Control* ctrl, AppCUI::Controls::Event eventType, int controlID) = 0;
    virtual bool Update()                                                                                       = 0;
    virtual void Draw(AppCUI::Graphics::Renderer& renderer)                                                     = 0;

    virtual void Init()   = 0;
    virtual void Pause()  = 0;
    virtual void Resume() = 0;
};
