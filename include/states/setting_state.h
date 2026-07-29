#pragma once

#include "core/state_machine.h"
#include "states/state.h"
#include "utils/shared_context.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

class SettingState : public State
{
private:
	std::unique_ptr<sf::Sprite> background_sprite_;
	std::unique_ptr<sf::Text> title_text_;
	std::vector<std::unique_ptr<sf::Text>> setting_texts_;
	int selected_index_;

	std::vector<std::string> buildDisplayStrings() const;

	void setupUI(const sf::Font &font);
	void updateUI();
	void moveUp();
	void moveDown();
	void increaseSetting();
	void decreaseSetting();

public:
	SettingState(SharedContext* context);
	~SettingState() override = default;

	void init() override;
	void handleEvent(const sf::Event &event) override;
	void update(float delta_time) override;
	void render(sf::RenderWindow &window) override;
};
