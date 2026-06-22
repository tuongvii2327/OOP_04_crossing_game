// src/core/state_machine.cpp
#include "core/state_machine.h"
#include "states/state.h"
#include <stdexcept>

StateMachine::StateMachine() {}

StateMachine::~StateMachine() = default;

void StateMachine::push(std::unique_ptr<State> state)
{
    pending_action = StateAction::Push;
    pending_state = std::move(state);
}

void StateMachine::pop()
{
    pending_action = StateAction::Pop;
}

void StateMachine::switchState(std::unique_ptr<State> state)
{
    pending_action = StateAction::Switch;
    pending_state = std::move(state);
}

void StateMachine::resetTo(std::unique_ptr<State> state)
{
    pending_action = StateAction::Reset;
    pending_state = std::move(state);
}

bool StateMachine::processTransitions()
{
    if (pending_action == StateAction::None)
        return false;

    if (pending_action == StateAction::Pop || pending_action == StateAction::Switch)
    {
        if (!states.empty())
        {
            states.pop_back();
        }
    }

    if (pending_action == StateAction::Reset)
    {
        states.clear();
    }

    if (pending_state)
    {

        if (!states.empty() && pending_action == StateAction::Push)
        {
            states.back()->pause();
        }

        states.push_back(std::move(pending_state));
        states.back()->init();
    }

    else if (!states.empty() && pending_action == StateAction::Pop)
    {
        states.back()->resume();
    }

    pending_action = StateAction::None;
    return true;
}

State &StateMachine::getActiveState()
{
    if (states.empty())
    {
        throw std::runtime_error("No active state in StateMachine!");
    }
    return *states.back();
}

void StateMachine::render(sf::RenderWindow &window)
{
    if (states.empty())
        return;

    int start_idx = states.size() - 1;
    while (start_idx > 0 && states[start_idx]->isOverlay())
    {
        start_idx--;
    }

    for (int i = start_idx; i < states.size(); ++i)
    {
        states[i]->render(window);
    }
}