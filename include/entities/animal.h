#pragma once

#include "dynamic_object.h"

class Animal : public DynamicObject
{
protected:
    // sf::Sound animal_sound;

    float speed;
    float timer;      // time of the current state
    float duration;   // max time of the current state
    bool is_move = 0; // current state: 1 = moving, 0 = stop

    float randomDuration() const; // pick a random duration for new state between MIN_DURATION_ANIMAL & MAX_DURATION_ANIMAL

    float sound_timer_ = 0.f;
    float next_sound_time_ = 0.f;
    float randomSoundInterval() const;

public:
    // contructor, destructor
    Animal(const Constants::DynamicObjectType &init_type,
        const Constants::ObjectColor &init_color, 
        Lane *init_lane = nullptr); // speed, velocity, is_move = 1, timer = 0, duration are calculated & set automactically

    // get, set
    float getSpeed() const;
    void setSpeed(const float &new_speed);

    float getTimer() const;
    void setTimer(const float &new_timer);

    float getDuration() const;
    void setDuration(const float &new_duration);

    bool isMove() const;
    void setMoveState(const bool &state); // set new duration and timer = 0 if new state != current state

    // Other
    void playAnimalSound();
    Constants::ObjectEffect getEffect() const override { return Constants::ObjectEffect::Lethal; }
    void updateState(float delta_time) override;
};
