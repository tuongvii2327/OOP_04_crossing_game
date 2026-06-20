// include/core/state_machine.h
#pragma once

#include <memory>
#include <stack>

class State;

class StateMachine
{
private:
    std::stack<std::unique_ptr<State>> states;
    std::unique_ptr<State> new_state;

    bool is_removing;
    bool is_adding;
    bool is_replacing;

public:
    StateMachine();
    ~StateMachine();

    void addState(std::unique_ptr<State> new_state, bool is_replacing = true);
    void removeState();

    void processStateChanges();

    // Get a reference to the active (top) state
    State &getActiveState();
    bool isEmpty() const { return states.empty(); }
};