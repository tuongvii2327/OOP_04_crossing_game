// include/core/state_machine.h
#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <stack>
#include <vector>

class State;

enum class StateAction
{
    None,
    Push,
    Pop,
    Switch,
    Reset
};

class StateMachine
{
private:
    std::vector<std::unique_ptr<State>> states;
    std::unique_ptr<State> pending_state;
    StateAction pending_action = StateAction::None;

public:
    StateMachine();
    ~StateMachine();
    void push(std::unique_ptr<State> state);

    void pop();

    void switchState(std::unique_ptr<State> state);

    void resetTo(std::unique_ptr<State> state);

    bool processTransitions();

    State &getActiveState();
    bool isEmpty() const { return states.empty(); }
    void render(sf::RenderWindow &window);
};