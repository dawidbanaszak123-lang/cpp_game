#include "Player.hpp"
#include <cmath>
#include <string_view>

namespace dungeon {

namespace {

// Zwraca wektor o długości 1 dla podanego kierunku.
[[nodiscard]] sf::Vector2f normalized(sf::Vector2f vector)
{
    const float length = std::sqrt(vector.x * vector.x + vector.y * vector.y);
    if (length <= 0.0f) {
        return {};
    }

    return {vector.x / length, vector.y / length};
}

}

// Tworzy gracza i ustawia jego wygląd.
Player::Player()
{
    body_.setSize({28.0f, 28.0f});
    body_.setOrigin(body_.getSize() / 2.0f);
    body_.setFillColor(sf::Color(60, 160, 255));
}

// Aktualizuje liczniki gracza, przeładowanie i broń.
void Player::update(float deltaSeconds)
{
    if (dodgeTimer_ > 0.0f) {
        dodgeTimer_ -= deltaSeconds;
    }
    if (dodgeCooldownTimer_ > 0.0f) {
        dodgeCooldownTimer_ -= deltaSeconds;
    }
    if (invincibilityTimer_ > 0.0f) {
        invincibilityTimer_ -= deltaSeconds;
    }
    if (reloadTimer_ > 0.0f) {
        reloadTimer_ -= deltaSeconds;
        // Kończy przeładowanie, gdy minie wymagany czas.
        if (reloadTimer_ <= 0.0f && reloadingWeaponIndex_ < rangedWeapons_.size()) {
            reloadTimer_ = 0.0f;
            rangedWeapons_[reloadingWeaponIndex_]->reload();
        }
    }
    for (const auto& weapon : rangedWeapons_) {
        weapon->update(deltaSeconds);
    }

    body_.setFillColor(sf::Color(60, 160, 255));
}

// Rysuje postać gracza.
void Player::render(sf::RenderTarget& target)
{
    target.draw(body_);
}

// Zwraca aktualną pozycję gracza.
sf::Vector2f Player::position() const
{
    return body_.getPosition();
}

// Ustawia pozycję gracza na mapie.
void Player::setPosition(sf::Vector2f position)
{
    body_.setPosition(position);
}

// Odejmuje życie graczowi po otrzymaniu obrażeń.
void Player::applyDamage(const Damage& damage)
{
    if (hasInvincibilityFrames()) {
        return;
    }

    health_ -= damage.amount;
    if (health_ < 0.0f) {
        health_ = 0.0f;
    }
    invincibilityTimer_ = invincibilityDurationSeconds_;
}

// Sprawdza, czy gracz nadal żyje.
bool Player::isAlive() const
{
    return health_ > 0.0f;
}

// Zwraca aktualne życie gracza.
float Player::health() const
{
    return health_;
}

// Zwraca maksymalne życie gracza.
float Player::maxHealth() const
{
    return maxHealth_;
}

// Przesuwa gracza o podaną wartość.
void Player::move(sf::Vector2f displacement)
{
    if (displacement.x != 0.0f || displacement.y != 0.0f) {
        lastMoveDirection_ = displacement;
    }
    body_.move(displacement);
}

// Wykonuje unik w wybranym kierunku.
void Player::dodgeRoll(sf::Vector2f direction)
{
    if (!canDodge()) {
        return;
    }

    if (direction.x == 0.0f && direction.y == 0.0f) {
        direction = lastMoveDirection_;
    }

    lastMoveDirection_ = direction;
    dodgeTimer_ = dodgeDurationSeconds_;
    dodgeCooldownTimer_ = dodgeCooldownSeconds_;
    invincibilityTimer_ = invincibilityDurationSeconds_;
}

// Sprawdza, czy gracz jest chwilowo nietykalny.
bool Player::hasInvincibilityFrames() const
{
    return invincibilityTimer_ > 0.0f;
}

// Miejsce na atak wręcz gracza.
void Player::meleeAttack(sf::Vector2f) {}

// Próbuje wystrzelić z aktywnej broni dystansowej.
void Player::fireRangedWeapon(sf::Vector2f target)
{
    (void)tryFireRangedWeapon(target);
}

// Zwraca pocisk, jeśli broń może teraz strzelić.
std::optional<Projectile> Player::tryFireRangedWeapon(sf::Vector2f target)
{
    if (rangedWeapons_.empty() || isReloading()) {
        return std::nullopt;
    }

    // Kierunek strzału prowadzi od gracza do kursora.
    const sf::Vector2f direction = normalized(target - position());
    if (direction.x == 0.0f && direction.y == 0.0f) {
        return std::nullopt;
    }

    return rangedWeapons_[activeRangedWeaponIndex_]->tryShoot(position(), direction);
}

// Zakłada broń do walki wręcz.
void Player::equipMeleeWeapon(std::unique_ptr<IMeleeWeapon> weapon)
{
    meleeWeapon_ = std::move(weapon);
}

// Dodaje nową broń dystansową do ekwipunku.
void Player::addRangedWeapon(std::unique_ptr<IRangedWeapon> weapon)
{
    rangedWeapons_.push_back(std::move(weapon));
    if (activeRangedWeaponIndex_ >= rangedWeapons_.size()) {
        activeRangedWeaponIndex_ = 0;
    }
}

// Zamienia aktywną broń dystansową na nową.
void Player::replaceActiveRangedWeapon(std::unique_ptr<IRangedWeapon> weapon)
{
    if (!weapon) {
        return;
    }

    reloadTimer_ = 0.0f;

    if (rangedWeapons_.empty()) {
        rangedWeapons_.push_back(std::move(weapon));
        activeRangedWeaponIndex_ = 0;
        return;
    }

    rangedWeapons_[activeRangedWeaponIndex_] = std::move(weapon);
}

// Wybiera broń dystansową po numerze.
void Player::selectRangedWeapon(std::size_t index)
{
    if (index < rangedWeapons_.size()) {
        activeRangedWeaponIndex_ = index;
    }
}

// Rozpoczyna przeładowanie aktywnej broni.
void Player::startReload()
{
    if (rangedWeapons_.empty() || isReloading()) {
        return;
    }

    if (activeAmmo() >= activeMagazineSize()) {
        return;
    }

    reloadingWeaponIndex_ = activeRangedWeaponIndex_;
    reloadTimer_ = reloadDurationSeconds_;
}

// Zwraca obszar kolizji gracza.
sf::FloatRect Player::bounds() const
{
    return body_.getGlobalBounds();
}

// Zwraca prędkość ruchu gracza.
float Player::movementSpeed() const
{
    return isDodging() ? baseSpeed_ * dodgeSpeedMultiplier_ : baseSpeed_;
}

// Sprawdza, czy gracz wykonuje unik.
bool Player::isDodging() const
{
    return dodgeTimer_ > 0.0f;
}

// Sprawdza, czy unik jest gotowy.
bool Player::canDodge() const
{
    return dodgeCooldownTimer_ <= 0.0f;
}

// Zwraca liczbę nabojów w aktywnej broni.
int Player::activeAmmo() const
{
    if (rangedWeapons_.empty()) {
        return 0;
    }

    return rangedWeapons_[activeRangedWeaponIndex_]->ammo();
}

// Zwraca rozmiar magazynka aktywnej broni.
int Player::activeMagazineSize() const
{
    if (rangedWeapons_.empty()) {
        return 0;
    }

    return rangedWeapons_[activeRangedWeaponIndex_]->magazineSize();
}

// Sprawdza, czy gracz przeładowuje broń.
bool Player::isReloading() const
{
    return reloadTimer_ > 0.0f;
}

// Zwraca typ aktywnej broni.
WeaponType Player::activeWeaponType() const
{
    if (rangedWeapons_.empty()) {
        return WeaponType::Melee;
    }

    return rangedWeapons_[activeRangedWeaponIndex_]->type();
}

// Zwraca nazwę aktywnej broni.
std::string_view Player::activeWeaponName() const
{
    if (rangedWeapons_.empty()) {
        return "None";
    }

    return rangedWeapons_[activeRangedWeaponIndex_]->displayName();
}

}
