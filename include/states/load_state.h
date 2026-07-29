#pragma once

#include "core/state_machine.h"
#include "states/state.h"
#include "utils/shared_context.h"
#include <SFML/Graphics.hpp>
#include <vector>

class LoadState : public State
{
private:
    std::unique_ptr<sf::Sprite> background_sprite_;
    std::unique_ptr<sf::Text> title_text_;
    std::vector<sf::Text> save_slot_texts_;
    int selected_index_;

    // Setup save slot UI
    void setupUI(const sf::Font& font);
    // Move selection up
    void moveUp();
    // Move selection down
    void moveDown();
    // Load game data from specified slot
    void loadGameData(int slot_index);

public:
    LoadState(SharedContext* context);
    ~LoadState() override = default;

    // Initialize load screen
    void init() override;
    // Handle slot selection and navigation
    void handleEvent(const sf::Event& event) override;
    // Update load state (no-op)
    void update(float delta_time) override;
    // Render load menu
    void render(sf::RenderWindow& window) override;
};
