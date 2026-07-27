#include "entities/people.h"
#include "utils/audio_system.h"
#include "utils/constants.h"
#include "utils/serialization.h"

People::People(Constants::ObjectColor color)
    : DynamicObject(Constants::DynamicObjectType::People, color, nullptr), // Truyền color cho lớp cha
      is_dead(false),
      step_size(static_cast<float>(Constants::LANE_HEIGHT)),
      people_color(color) // Lưu lại màu
{
    resetPosition();
}

People::People(float x, float y, Constants::ObjectColor color)
    : People(color)
{
    setPosition(sf::Vector2f(x, y));
}

Constants::DynamicObjectType People::getType() const
{
    return Constants::DynamicObjectType::People;
}

Constants::ObjectColor People::getColor() const
{
    return people_color;
}

void People::updateState(float delta_time)
{
    if (is_dead || !is_active)
        return;

    // process any animations or state changes for the People object here
    // add later: add animation for people
}

bool People::isColliding(const sf::FloatRect &other_hitbox) const
{
    if (is_dead || !is_active)
        return false;
    return hitbox.findIntersection(other_hitbox).has_value();
}

bool People::tryToMove(float dx, float dy, float camera_top_y)
{
    if (is_dead)
        return false;

    float future_hitbox_left = position.x + hitbox_offset.x + dx;
    float future_hitbox_right = future_hitbox_left + hitbox.size.x;

    float future_hitbox_top = position.y + hitbox_offset.y + dy;
    float future_hitbox_bottom = future_hitbox_top + hitbox.size.y;

    float left_bound = Constants::GAME_PLAY_BAR_WIDTH; // left (140)
    float right_bound = Constants::WINDOW_WIDTH;       // right (1400)

    // check if hit box out out bounds
    if (future_hitbox_left < left_bound ||
        future_hitbox_right > right_bound ||
        future_hitbox_top < camera_top_y ||
        future_hitbox_bottom > (camera_top_y + Constants::WINDOW_HEIGHT))
    {
        return false;
    }

    // if valid, move the People object
    moveBy(sf::Vector2f(dx, dy));
    Audio().playSFX(AudioSystem::SFX::Jump);
    return true;
}

void People::die()
{
    is_dead = true;
}

bool People::isDead() const
{
    return is_dead;
}
sf::Vector2f People::getStartPosition(int init_cell, int init_lane) const // init_cell là ô people sẽ xuất hiện trong lane
{
    float start_x = Constants::WINDOW_WIDTH - (Constants::NUMBER_OF_CELLS - init_cell + 1) * Constants::CELL_SIZE + (Constants::CELL_SIZE - getWidthSprite()) / 2.f;
    float start_y = (Constants::WINDOW_HEIGHT - (Constants::NUMBER_OF_LANES - init_lane + 1) * Constants::LANE_HEIGHT) + (Constants::LANE_HEIGHT - getHeightSprite()) / 2.f; // luon xuat hien o lane cuoi cung
    return sf::Vector2f(start_x, start_y);
}

void People::resetPosition() 
{
    setPosition(getStartPosition());
    is_dead = false;
    is_active = true;
}

sf::FloatRect People::getBounds() const
{
    return sprite.getGlobalBounds();
}

bool People::handleEvent(const sf::Event &event, float camera_top_y)
{
    if (is_dead || !is_active)
        return false;

    const auto *key = event.getIf<sf::Event::KeyPressed>();
    if (!key)
        return false;

    if (key->code == sf::Keyboard::Key::W || key->code == sf::Keyboard::Key::Up)
    {
        if (tryToMove(0.f, -step_size, camera_top_y))
        {
            return true;
        }
    }
    else if (key->code == sf::Keyboard::Key::S || key->code == sf::Keyboard::Key::Down)
    {
        return tryToMove(0.f, step_size, camera_top_y);
    }
    else if (key->code == sf::Keyboard::Key::A || key->code == sf::Keyboard::Key::Left)
    {
        return tryToMove(-step_size, 0.f, camera_top_y);
    }
    else if (key->code == sf::Keyboard::Key::D || key->code == sf::Keyboard::Key::Right)
    {
        return tryToMove(step_size, 0.f, camera_top_y);
    }

    return false;
}

void People::serialize(std::ostream &os) const
{
    // Save living status
    Serialization::write(os, is_dead);

    // Save player's coordinate
    Serialization::write(os, position.x);
    Serialization::write(os, position.y);

    // Save player's step size
    Serialization::write(os, step_size);

    // Save active status inherit from DynamicObject
    Serialization::write(os, is_active);
}

void People::deserialize(std::istream &is)
{
    Serialization::read(is, is_dead);
    Serialization::read(is, position.x);
    Serialization::read(is, position.y);
    Serialization::read(is, step_size);
    Serialization::read(is, is_active);

    is_dead = false;  // Ép sống lại khi load game đề phòng file lưu bị lỗi chết
    is_active = true; // Ép người chơi hoạt động để nhận sự kiện phím bấm

    // Log kiểm tra tọa độ nạp thực tế từ file xuống màn hình console
    std::cout << "[DEBUG LOAD] Player Loaded Position: (" << position.x << ", " << position.y << ")\n";

    // Update sprite & hitbox
    setPosition(position);
}