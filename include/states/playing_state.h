#pragma once

#include "core/state_machine.h"
#include "entities/people.h"
#include "states/state.h"
#include "utils/shared_context.h"
#include "world/map.h"

#include <SFML/Graphics.hpp>
#include <memory>
#include <fstream>

class PlayingState : public State
{
private:
    std::unique_ptr<sf::Text> score_text_;
    std::unique_ptr<sf::Text> coin_text_;

    // Integrate your original entity system here
    std::unique_ptr<People> player_;
    Map map_;

    // Camera handling for infinite scrolling as the player advances
    sf::View view_;
    void setupWorld();

    static PlayingState *active_instance_; // Con trỏ tĩnh toàn cục cục bộ
    int load_slot_ = 0;                    // Slot cần nạp khi khởi tạo
    float death_timer_ = 0.0f;
    float shake_timer_ = 0.0f;
    sf::RectangleShape flash_overlay_;

    int current_top_score_ = 0;
    bool has_beaten_highscore_ = false;

public:
    PlayingState(SharedContext *context, int load_slot = 0);
    ~PlayingState() override;

    static PlayingState *getActiveInstance() { return active_instance_; }

    // Initialize UI resources, player position, and generate the initial map
    void init() override;

    // Handle movement key presses via People's tryToMove()
    void handleEvent(const sf::Event &event) override;

    // Update camera position and handle Map's lane spawn/delete logic
    void update(float dt) override;

    // Render the game world alongside the static score HUD overlay
    void render(sf::RenderWindow &window) override;

    void pause() override;
    void resume() override;
    void restart();

    void serialize(std::ostream &os, const std::string &slot_name) const;
    void deserialize(std::istream &is);

    void setLoadSlot(int slot) { load_slot_ = slot; }
};
