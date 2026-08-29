#pragma once

#include "core/state_machine.h"
#include "states/state.h"
#include "utils/shared_context.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class AboutState : public State
{
private:
    std::unique_ptr<sf::Text> title_text_;
    std::vector<std::unique_ptr<sf::Text>> lines_text_;
    std::unique_ptr<sf::Text> back_btn_;
    bool is_back_hovered_ = false;
    void setupUI(const sf::Font& font);

public:
    AboutState(SharedContext* context);
    ~AboutState() override = default;

    void init() override;
    void handleEvent(const sf::Event& event) override;
    void update(float delta_time) override;
    void render(sf::RenderWindow& window) override;
};