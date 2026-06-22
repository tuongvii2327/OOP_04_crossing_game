// include/core/state_machine.h
#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <stack>
#include <vector>

class State;

class StateMachine
{
private:
    std::vector<std::unique_ptr<State>> states;
    std::unique_ptr<State> new_state;

    bool is_removing;
    bool is_adding;
    bool is_replacing;
    bool is_replacing_root;

public:
    StateMachine();
    ~StateMachine();

	void addState(std::unique_ptr<State> new_state, bool is_replacing = true);
	void removeState();

    bool processStateChanges();

    // Get a reference to the active (top) state
    State &getActiveState();
    bool isEmpty() const { return states.empty(); }
    void replaceRootState(std::unique_ptr<State> new_state);
    void render(sf::RenderWindow &window);
};
