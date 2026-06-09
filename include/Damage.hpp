#pragma once

namespace dungeon {

enum class DamageType {
    Melee,
    Bullet,
    Laser,
    Explosion
};

struct Damage {
    float amount{};
    DamageType type{DamageType::Bullet};
    bool ignoresArmor{};
};

}
