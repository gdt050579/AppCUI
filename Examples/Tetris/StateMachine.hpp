#pragma once

#include <memory>
#include <stack>

#include "State.hpp"

namespace Tetris
{
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
    void PushState(const std::shared_ptr<struct GameData>& data, bool isReplacing = true)
    {
        newState.reset(new T(data));
        this->isAdding    = true;
        this->isReplacing = isReplacing;
    }
    void PopState();

    bool ProcessStackAction();

    const std::shared_ptr<State>& PeekState();

    const bool GetStatesCount() const;

  private:
    std::stack<std::shared_ptr<State>> states;
    std::shared_ptr<State> newState;

    bool isRemoving  = false;
    bool isAdding    = false;
    bool isReplacing = false;
};
} // namespace Tetris
