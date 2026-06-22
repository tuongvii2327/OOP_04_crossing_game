#pragma once

#include <memory>
#include <stack>
#include "../states/state.h"

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
	~StateMachine() = default;

	void addState(std::unique_ptr<State> new_state, bool is_replacing = true);
	void removeState();

	void processStateChanges();

	State& getActiveState();
};
