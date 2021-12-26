#include "StateMachine.hpp"

namespace Tetris
{
StateMachine::StateMachine(StateMachine* other)
{
}

void StateMachine::PopState()
{
    isRemoving = true;
}

bool StateMachine::ProcessStackAction()
{
    bool actionTaken = false;

    if (isRemoving && !states.empty())
    {
        states.pop();
        actionTaken = true;

        if (!states.empty())
        {
            states.top()->Resume();
        }

        isRemoving = false;
    }

    if (isAdding)
    {
        if (!states.empty())
        {
            if (isReplacing)
            {
                states.pop();
            }
            else
            {
                states.top()->Pause();
            }
        }

        states.push(newState);
        actionTaken = true;

        isAdding = false;
        states.top()->Init();
    }

    return actionTaken;
}

const std::shared_ptr<State>& StateMachine::PeekState()
{
    return states.top();
}

const bool StateMachine::GetStatesCount() const
{
    return states.size();
}
} // namespace Tetris
