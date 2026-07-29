#pragma once

#include "states/state.h"
#include "utils/shared_context.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <vector>

class GameOverState : public State
{
private:
    sf::RectangleShape background_overlay_;
    std::unique_ptr<sf::Text> title_text_;
    std::unique_ptr<sf::Text> score_text_;
    std::vector<std::unique_ptr<sf::Text>> menu_texts_;
    int selected_index_;

    void setupUI(const sf::Font& font);
    void moveUp();
    void moveDown();
    void saveHighScore();

public:
    explicit GameOverState(SharedContext* context);
    ~GameOverState() override = default;

    void init() override;
    void handleEvent(const sf::Event& event) override;
    void update(float delta_time) override;
    void render(sf::RenderWindow& window) override;

    bool isOverlay() const override { return true; }
};
