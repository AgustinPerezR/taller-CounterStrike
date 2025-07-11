#include "../include/player.h"

#include <cmath>
#include <iostream>
#include <string>

#include "../../server/include/id_generator.h"

//-------------
// Inicializo las variables estáticas (para poder compilar)
bool Player::initialized = false;
float Player::PLAYER_SPEED = 0;
float Player::INITIAL_MONEY = 0;
int Player::INITIAL_HEALTH = 0;


void Player::init(float player_speed, float initial_money, int initial_health) {
    if (initialized == false) {
        PLAYER_SPEED = player_speed;
        INITIAL_MONEY = initial_money;
        INITIAL_HEALTH = initial_health;
        initialized = true;
    }
}
//---------

Player::Player(const std::string& name, const Team team, const Vec2D& position):
        serverId(IdGenerator::getNextId()),
        name(name),
        team(team),
        posX(position.getX()),
        posY(position.getY()),
        health(INITIAL_HEALTH),
        state(PlayerState::Idle),
        speed(PLAYER_SPEED),
        knife(WeaponFactory::create(Weapon::Knife)),
        primaryWeapon(WeaponFactory::create(
                Weapon::Ak47)),  // esto hay que quitarlo cuando se pueda comprar armas.
        secondaryWeapon(WeaponFactory::create(Weapon::Glock)),
        equippedWeapon(TypeWeapon::Knife),
        id_weapon(knife->getServerId()),
        money(INITIAL_MONEY),
        skinT(PlayerSkin::Terrorist3),
        skinCT(PlayerSkin::CounterTerrorist3) {}


void Player::setPrimaryWeapon(std::unique_ptr<Weapon_> weapon) {
    primaryWeapon = std::move(weapon);
    if (primaryWeapon && equippedWeapon == TypeWeapon::Primary) {
        id_weapon = primaryWeapon->getServerId();
    }
}
void Player::setBomb(Bomb* _bomb) {
    bomb = _bomb;
    if (_bomb == nullptr) {
        setEquippedWeapon(TypeWeapon::Secondary);
    }
}

void Player::setEquippedWeapon(TypeWeapon type) {

    switch (type) {
        case TypeWeapon::Primary:
            if (primaryWeapon == nullptr) {
                return;
            }
            id_weapon = primaryWeapon->getServerId();
            if (bomb != nullptr)
                bomb->hide();
            break;
        case TypeWeapon::Secondary:
            id_weapon = secondaryWeapon->getServerId();
            if (bomb != nullptr)
                bomb->hide();
            break;
        case TypeWeapon::Bomb:
            if (bomb == nullptr)  // si no tiene una, no cambio
                return;
            bomb->equip();
            id_weapon = bomb->getServerId();
            break;
        case TypeWeapon::Knife:
            id_weapon = knife->getServerId();
            if (bomb != nullptr)
                bomb->hide();
            break;
        default:
            break;
    }
    equippedWeapon = type;
}

float Player::getX() const { return posX; }
void Player::setX(const float x) { posX = x; }

float Player::getY() const { return posY; }
void Player::setY(const float y) { posY = y; }

void Player::setPostion(const Vec2D& pos) {
    posX = pos.getX();
    posY = pos.getY();
}

float Player::getAngle() const { return angle; }
void Player::setAngle(float _angle) { angle = _angle; }

std::string Player::getId() const { return name; }

Team Player::getTeam() const { return team; }
int Player::getHealth() const { return health; }
float Player::getMoney() const { return money; }
float Player::getSpeed() const { return speed; }

TypeWeapon Player::getEquippedWeapon() const { return equippedWeapon; }

Weapon Player::getSpecificEquippedWeapon() const {
    switch (equippedWeapon) {
        case TypeWeapon::Knife:
            return knife ? knife->getWeaponType() : Weapon::None;
        case TypeWeapon::Primary:
            return primaryWeapon ? primaryWeapon->getWeaponType() : Weapon::None;
        case TypeWeapon::Secondary:
            return secondaryWeapon ? secondaryWeapon->getWeaponType() : Weapon::None;
        default:
            return Weapon::None;
    }
}

Weapon_* Player::getEquippedWeaponInstance() const {
    switch (equippedWeapon) {
        case TypeWeapon::Knife:
            return knife.get();
        case TypeWeapon::Primary:
            return primaryWeapon.get();
        case TypeWeapon::Secondary:
            return secondaryWeapon.get();
        default:
            return nullptr;
    }
}

Weapon_* Player::getPrimaryWeapon() const { return primaryWeapon.get(); }

Weapon_* Player::getSecondaryWeapon() const { return secondaryWeapon.get(); }

bool Player::isAlive() const { return state != PlayerState::Dead; }

bool Player::canShoot(double currentTime) const {
    switch (equippedWeapon) {
        case TypeWeapon::Knife:
            return knife && knife->canShoot(currentTime);
        case TypeWeapon::Primary:
            return primaryWeapon && primaryWeapon->canShoot(currentTime);
        case TypeWeapon::Secondary:
            return secondaryWeapon && secondaryWeapon->canShoot(currentTime);
        default:
            return false;
    }
}

void Player::takeDamage(int dmg) {
    if (!isAlive())
        return;

    health -= dmg;
    if (health <= 0) {
        health = 0;
        state = PlayerState::Dead;
        stats.registerDeath();
    }
}

std::vector<Projectile> Player::shoot(float dirX, float dirY, double currentTime) {
    Weapon_* weapon = getEquippedWeaponInstance();
    if (!weapon)  // || !weapon->canShoot(currentTime)) //se chequea en weapon->shoot()
        return {};
    if (weapon->canShoot(currentTime))
        state = PlayerState::Attacking;
    return weapon->shoot(posX, posY, dirX, dirY, name, currentTime);
}

std::unique_ptr<Weapon_> Player::dropPrimaryWeapon() {
    if (!primaryWeapon)
        return nullptr;
    if (!isAlive() && equippedWeapon == TypeWeapon::Primary) {
        setEquippedWeapon(TypeWeapon::Knife);
    }
    return std::move(primaryWeapon);
}

uint32_t Player::getServerId() const { return serverId; }

void Player::revive() {
    health = INITIAL_HEALTH;
    state = PlayerState::Idle;
}

void Player::setTeam(Team newTeam) { team = newTeam; }

void Player::setState(PlayerState newState) { state = newState; }

bool Player::spendMoney(int amount) {
    if (money >= amount) {
        money -= amount;
        return true;
    }
    return false;
}

void Player::addMoney(int amount) {
    money += amount;
    stats.addMoney(amount);
}

const Statistics& Player::getStats() const { return stats; }

LocalPlayerInfo Player::generateLocalPlayerInfo() const {
    PlayerSkin currentSkin = (team == Team::CounterTerrorist) ? skinCT : skinT;
    int ammo = 0;
    if (equippedWeapon == TypeWeapon::Primary) {
        ammo = primaryWeapon->getBullets();
    } else if (equippedWeapon == TypeWeapon::Secondary) {
        ammo = secondaryWeapon->getBullets();
    }

    return LocalPlayerInfo(serverId, team, currentSkin, state, Vec2D(posX, posY), angle,
                           equippedWeapon, getSpecificEquippedWeapon(), health, money, ammo,
                           id_weapon);
}

PlayerInfo Player::generatePlayerInfo() const {
    PlayerSkin currentSkin = (team == Team::CounterTerrorist) ? skinCT : skinT;

    return PlayerInfo(serverId, name, team, currentSkin, state, Vec2D(posX, posY), angle,
                      equippedWeapon, getSpecificEquippedWeapon(), id_weapon);
}
