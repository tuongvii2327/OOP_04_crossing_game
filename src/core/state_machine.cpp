// src/core/state_machine.cpp
#include "core/state_machine.h"
#include "states/state.h"
#include <stdexcept>

StateMachine::StateMachine()
    : is_removing(false),
      is_adding(false),
      is_replacing(false),
      is_replacing_root(false)
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
bool StateMachine::processStateChanges()
{
    bool state_changed = false;
    if (is_removing && !states.empty())
    {
        states.pop_back();
        if (!states.empty())
        {
            states.back()->resume();
        }
        is_removing = false;
        state_changed = true;
    }

    if (is_adding)
    {
        if (is_replacing_root)
        {
            states.clear(); 
            is_replacing_root = false;
        }
        else if (!states.empty())
        {
            if (is_replacing)
            {
                states.pop_back();
            }
            else
            {
                states.back()->pause();
            }
        }

        states.push_back(std::move(new_state));
        states.back()->init();
        is_adding = false;
        state_changed = true;
    }
    return state_changed;
}

// Get reference to active state, throw if none exists
State &StateMachine::getActiveState()
{
    if (states.empty())
    {
        throw std::runtime_error("No active state in StateMachine!");
    }
    return *states.back();
}

void StateMachine::replaceRootState(std::unique_ptr<State> new_state)
{
    is_replacing_root = true;
    is_adding = true;
    this->new_state = std::move(new_state);
}

void StateMachine::render(sf::RenderWindow &window)
{
    if (states.empty())
        return;

    // find first non-overlay state from the top
    int start_idx = states.size() - 1;
    while (start_idx > 0 && states[start_idx]->isOverlay())
    {
        start_idx--;
    }

    // Render from the first non-overlay state to the top
    for (int i = start_idx; i < states.size(); ++i)
    {
        states[i]->render(window);
    }
}