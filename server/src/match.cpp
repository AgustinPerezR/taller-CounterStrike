#include "../include/match.h"

#include <algorithm>
#include <iostream>
#include <vector>

Match::Match(): phase(GamePhase::Preparation), roundsPlayed(0) {}

void Match::addPlayer(Player&& player) { players.emplace_back(std::move(player)); }

bool Match::addPlayer(const std::string& username) {
    Player newPlayer(username, Team::Terrorist);
    players.push_back(std::move(newPlayer));
    return true;
}

void Match::removePlayer(const std::string& username) {
    for (auto& p: players) {
        if (p.getId() == username) {
            // players.erase(std::find(players.begin(), players.end(), p));
        }
    }
}


// std::vector<std::string> Match::getPlayers() { return players; }


Player* Match::getPlayer(const std::string& playerName) {
    for (auto& p: players) {
        if (p.getId() == playerName)
            return &p;
    }
    return nullptr;
}

bool Match::movePlayer(const std::string& playerId, const int dx, const int dy) {
    Player* p = getPlayer(playerId);
    if (!p || !p->isAlive())
        return false;

    const int newX = p->getX() + dx;
    const int newY = p->getY() + dy;

    if (map.isWalkable(newX, newY)) {
        p->setPosition(newX, newY);
        return true;
    }

    return false;
}
void Match::processAction(const PlayerAction& action) {
    Player* player = getPlayer(action.player_username);
    if (!player || !player->isAlive())
        return;

    GameAction gameAction = action.gameAction;
    switch (gameAction.type) {
        case GameActionType::Walk: {
            const int dx = gameAction.direction.getX();
            const int dy = gameAction.direction.getY();
            movePlayer(action.player_username, dx, dy);
            break;
        }
        case GameActionType::ChangeWeapon: {
            player->setEquippedWeapon(gameAction.typeWeapon);
            break;
        }
        case GameActionType::Attack: {
            if (player->getEquippedWeapon() == TypeWeapon::Bomb) {
                processPlant(action.player_username);
            }
            const int dmg =
                    player->attack(gameAction.direction.getX(), gameAction.direction.getY());
            std::cout << "Player " << action.player_username << " shoot with " << dmg
                      << " damage\n";
            break;
        }
        // case ActionType::DEFUSE:
        //   processDefuse(playerId);
        //   break;
        default:
            std::cout << "Accion no implementada\n";
            break;
    }
}

void Match::updateState(double elapsedTime) {
    if (roundOver)
        return;

    roundTimer -= elapsedTime;
    if (bombPlanted) {
        bombTimer -= elapsedTime;
        if (bombTimer <= 0) {
            std::cout << "La bomba exploto!\n";
            roundOver = true;
            roundWinner = Team::Terrorist;
        }
    }

    checkRoundEnd();
}


void Match::processPlant(const std::string& playerName) {
    Player* player = getPlayer(playerName);
    if (!player || !player->isAlive() || player->getTeam() != Team::Terrorist)
        return;
    if (bombPlanted)
        return;

    int x = player->getX();
    int y = player->getY();
    if (!map.isBombSite(x, y))
        return;

    bombPlanted = true;
    bombPosX = x;
    bombPosY = y;
    bombTimer = TIME_TO_EXPLODE;

    std::cout << "Player " << playerName << " planted the bomb at (" << x << ", " << y << ")\n";
}

void Match::processDefuse(const std::string& playerName) {
    Player* player = getPlayer(playerName);
    if (!player || !player->isAlive() || player->getTeam() != Team::CounterTerrorist)
        return;
    if (!bombPlanted)
        return;

    int x = player->getX();
    int y = player->getY();
    if (x != bombPosX || y != bombPosY)
        return;

    bombPlanted = false;
    bombTimer = 0;
    roundOver = true;
    roundWinner = Team::CounterTerrorist;

    std::cout << "Player " << playerName << " defused the bomb!\n";
}

void Match::checkRoundEnd() {
    if (roundOver)
        return;

    bool terroristsLeft = false;
    bool antiterroristsLeft = false;

    for (const auto& p: players) {
        if (!p.isAlive())
            continue;
        if (p.getTeam() == Team::Terrorist)
            terroristsLeft = true;
        if (p.getTeam() == Team::CounterTerrorist)
            antiterroristsLeft = true;
    }

    if (!terroristsLeft) {
        roundOver = true;
        roundWinner = Team::CounterTerrorist;
        std::cout << "Todos los terroristas murieron. Ganan los antiterroristas. \n";
    } else if (!antiterroristsLeft) {
        roundOver = true;
        roundWinner = Team::Terrorist;
        std::cout << "Todos los antiterroristas murieron. Ganan los terroristas. \n";
    } else if (roundTimer <= 0 && !bombPlanted) {
        roundOver = true;
        roundWinner = Team::CounterTerrorist;
        std::cout << "Se acabó el tiempo sin bomba. Ganan los antiterroristas. \n";
    }
}
/*
GameInfo Match::generateGameInfo(const std::string& playerName) const {
    GameInfo info;

    for (const auto& p : players) {
        if (p.getId() == playerName) {
            info.posX = p.getX();
            info.posY = p.getY();
            info.health = p.getHealth();
            info.equippedWeapon = p.getEquippedWeapon();
            info.bullets = p.getEquippedWeapon() == WeaponType::PRIMARY && p.getPrimaryWeapon() ?
                           p.getPrimaryWeapon()->getBullets() :
                           (p.getEquippedWeapon() == WeaponType::SECONDARY && p.getSecondaryWeapon()
? p.getSecondaryWeapon()->getBullets() : 0); } else { info.otherPlayers.push_back(PlayerInfo{
                p.getId(), p.getX(), p.getY(), p.isAlive(), p.getType()
            });
        }
    }

    info.bombPlanted = bombPlanted;
    info.bombX = bombPosX;
    info.bombY = bombPosY;
    info.timeLeft = roundTimer;

    return info;
}
*/
GameInfo Match::generateGameInfo() const {
    std::vector<PlayerInfo> playersInfo;
    GameInfo gameInfo(this->phase, roundTimer, playersInfo);
    return gameInfo;
}
void Match::showPlayers() const {
    std::cout << "Players in match:\n";
    for (const auto& p: players) {
        std::cout << " - Player " << p.getId() << " en (" << p.getX() << "," << p.getY() << ") "
                  << (p.isAlive() ? "[VIVO]" : "[MUERTO]") << "\n";
    }
}
