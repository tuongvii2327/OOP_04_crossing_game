#include "world/lane_safe.h"
#include "entities/rock.h"
#include "utils/serialization.h"

LaneSafe::LaneSafe(const sf::Vector2f &init_position, const int &init_level, const int &init_direction, bool spawn_obstacles)
    : Lane(init_position, init_level, init_direction, Constants::LaneType::Safe)
{
    if (spawn_obstacles)
    {
        seedObjects();
    }
}

LaneSafe::~LaneSafe() = default;

Constants::LaneType LaneSafe::getType() const
{
    return Constants::LaneType::Safe;
}

void LaneSafe::seedObjects()
{
    int min_rocks = 0;
    int max_rocks = 1;
 
    if (level >= 10) {min_rocks = 3; max_rocks = Constants::MAX_OBJECT_PER_LANE;}
    else if (level >= 6) {min_rocks = 2; max_rocks = 3;}
    else if (level >= 3) {min_rocks = 1; max_rocks = 2; }
    else {min_rocks = 0; max_rocks = 1;}

    int count = randomRange(min_rocks, max_rocks);
    if (count == 0) return;

    int total_cols = static_cast<int>(Constants::LANE_WIDTH / Constants::CELL_SIZE);

    for (int i = 0; i < count; ++i)
    {
        for (int tryy = 0; tryy < 10; ++tryy)
        {
            int col = randomRange(1, std::max(1, total_cols - 2));
            float cell_x = position.x + (col * Constants::CELL_SIZE);
            float cell_y = position.y;

            bool overlap = false;
            for (const auto &existing : list_static)
            {
                if (std::abs(cell_x - existing->getPosition().x) < Constants::CELL_SIZE * 0.9f)
                {
                    overlap = true;
                    break;
                }
            }

            if (!overlap)
            {
                auto rock = std::make_unique<Rock>(this, sf::Vector2f{cell_x, cell_y});
                list_static.push_back(std::move(rock));
                break;
            }
        }
    }
}

void LaneSafe::spawnObject()
{
    // no-op: safe lanes never spawn dynamic objects
}

void LaneSafe::updateState(float delta_time)
{
    Lane::updateState(delta_time);
}

void LaneSafe::render(sf::RenderWindow &window) const
{
    Lane::render(window);
}

void LaneSafe::spawnLane(const sf::Vector2f &new_position, const int &new_level, const int &new_direction)
{
    Lane::spawnLane(new_position, new_level, new_direction);
}

void LaneSafe::serialize(std::ostream &os) const
{
    Lane::serialize(os);
}

void LaneSafe::deserialize(std::istream &is)
{
    Lane::deserialize(is);
}
