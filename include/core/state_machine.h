#pragma once

#include <memory>
#include <stack>
#include "../states/state.h"

class StateMachine 
{
private:
    std::stack<std::unique_ptr<State>> mStates;
    std::unique_ptr<State> mNewState;

    bool mIsRemoving;
    bool mIsAdding;
    bool mIsReplacing;

public:
    StateMachine();
    ~StateMachine() = default;

    void addState(std::unique_ptr<State> newState, bool isReplacing = true);
    void removeState();
    
    void processStateChanges();

    State& getActiveState();
};