#pragma once

#include <memory>
#include <stack>

#include "State.hpp"

class StateMachine
{
  public:
    StateMachine() = default;

    StateMachine(const StateMachine& other)     = default;
    StateMachine(StateMachine&& other) noexcept = default;
    explicit StateMachine(StateMachine* other);
    StateMachine& operator=(const StateMachine& other) = default;
    StateMachine& operator=(StateMachine&& other) noexcept = default;

    ~StateMachine() = default;

    template <typename T>
    void PushState(
          const std::shared_ptr<struct GameData>& data, const AppCUI::Controls::SingleApp* app, bool isReplacing = true)
    {
        newState.reset(new T(data, app));
        isAdding    = true;
        isReplacing = isReplacing;
    }
    void PopState();

    bool ProcessStackAction();

    const std::shared_ptr<State>& PeekState();

  private:
    std::stack<std::shared_ptr<State>> states;
    std::shared_ptr<State> newState;

    bool isRemoving  = false;
    bool isAdding    = false;
    bool isReplacing = false;
};
