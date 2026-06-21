// src/core/state_machine.cpp
#include "core/state_machine.h"
#include "states/state.h"
#include <stdexcept>

StateMachine::StateMachine()
    : is_removing(false),
      is_adding(false),
      is_replacing(false)
{
}

StateMachine::~StateMachine() = default;

// Queue a new state with move semantics
void StateMachine::addState(std::unique_ptr<State> new_state, bool is_replacing)
{
    is_adding = true;
    this->is_replacing = is_replacing;
    this->new_state = std::move(new_state);
}

// Queue removal of the top state
void StateMachine::removeState()
{
    is_removing = true;
}

// Execute all pending state transitions
void StateMachine::processStateChanges()
{
    if (is_removing && !states.empty())
    {
        states.pop();
        if (!states.empty())
        {
            states.top()->resume();
        }
        is_removing = false;
    }

    if (is_adding)
    {
        if (!states.empty())
        {
            if (is_replacing)
            {
                states.pop();
            }
            else
            {
                states.top()->pause();
            }
        }

        states.push(std::move(new_state));
        states.top()->init();
        is_adding = false;
    }
}

// Get reference to active state, throw if none exists
State &StateMachine::getActiveState()
{
    if (states.empty())
    {
        throw std::runtime_error("No active state in StateMachine!");
    }
    return *states.top();
}

void StateMachine::replaceRootState(std::unique_ptr<State> new_state)
{
    while (!states.empty())
    {
        states.pop();
    }
    states.push(std::move(new_state));
    states.top()->init();
}