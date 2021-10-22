#include "StateMachine.hpp"

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

		states.top()->Init();
		isAdding = false;
	}

    return actionTaken;
}

const std::shared_ptr<State>& StateMachine::PeekState()
{
	return states.top();
}
