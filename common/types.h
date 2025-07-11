#ifndef TYPES_H_
#define TYPES_H_

#include <cstdint>
#include <string>
#include <vector>

#include "utils/Vec2D.h"


using ServerEntityID = uint32_t;

enum class Weapon : unsigned char { None, Glock, Ak47, M3, Awp, Knife, Bomb };
enum class TypeWeapon : unsigned char { Primary, Secondary, Knife, Bomb };
enum class AmmoType : unsigned char { None, Primary, Secondary };
enum class Team : unsigned char { Terrorist, CounterTerrorist };
enum class GamePhase : unsigned char { Preparation, Combat, EndOfMatch };
enum class BombState : unsigned char { Dropped, Equipped, Hidden, Planted, Exploded, Defused };

enum class PlayerState : unsigned char {
    Idle,
    Walking,
    Attacking,
    TakingDamage,
    PickingUp,
    Dead,
    DefusingBomb
};
enum class WeaponState : unsigned char { DROPPED, EQUIPPED, HIDDEN };

enum class PlayerSkin {
    Terrorist1,
    Terrorist2,
    Terrorist3,
    Terrorist4,
    CounterTerrorist1,
    CounterTerrorist2,
    CounterTerrorist3,
    CounterTerrorist4
};

enum class TypeItem { Coin, Glock, Ak47, M3, Awp, Bomb };

enum class TypeTileMap { Desert, Aztec, Training };

struct PlayerInfoLobby {
    std::string username;
    Team team;
    bool is_player_host;  // puede variar si el creador original abandona la partida

    PlayerInfoLobby() {}
    PlayerInfoLobby(const std::string& username, const Team team, bool is_player_host):
            username(username), team(team), is_player_host(is_player_host) {}
};


struct MatchRoomInfo {
    bool matchStarted;
    std::vector<PlayerInfoLobby> players;

    MatchRoomInfo() {}
    explicit MatchRoomInfo(const std::vector<PlayerInfoLobby>& players, bool matchStarted = false):
            matchStarted(matchStarted), players(players) {}
};

// DTOs
//--------
// menu.
enum MenuActionType { Exit, Create, Join, List, ListScenarios };

struct MenuAction {
    const MenuActionType type;
    const std::string name_match;
    const std::string scenario_name = "";

    explicit MenuAction(MenuActionType type, const std::string& name_match = "",
                        const std::string& scenario_name = ""):
            type(type), name_match(name_match), scenario_name(scenario_name) {}
};

// lobby
enum class LobbyAction { QuitMatch, StartMatch, ListPlayers };

// game
enum GameActionType {
    Null,
    BuyWeapon,
    BuyAmmo,
    Attack,
    Walk,
    ChangeWeapon,
    PickUp,
    Rotate,
    DefuseBomb,
    ExitMatch
};


struct GameAction {
    GameActionType type = Null;
    Weapon weapon = Weapon::None;  // rellenar si se quiere comprar una.
    AmmoType ammoType = AmmoType::None;
    TypeWeapon typeWeapon = TypeWeapon::Knife;  // rellenar si se quiere cambiar o comprar municion.
    int count_ammo = 0;                         // rellenar si quiere comprar municion
    Vec2D direction;
    float angle = 0;  // para Rotate

    GameAction() {}
    explicit GameAction(GameActionType type, Weapon weapon = Weapon::Glock):
            type(type), weapon(weapon) {}

    explicit GameAction(GameActionType type, AmmoType ammoType, int count_ammo = 0):
            type(type), ammoType(ammoType), count_ammo(count_ammo) {}

    explicit GameAction(GameActionType type, TypeWeapon typeWeapon):
            type(type), typeWeapon(typeWeapon) {}

    explicit GameAction(GameActionType type, const Vec2D& direction):
            type(type), direction(direction) {}

    explicit GameAction(GameActionType type, float angle): type(type), angle(angle) {}

    GameAction(const GameAction&) = default;
    GameAction& operator=(const GameAction&) = default;
};

struct PlayerAction {
    std::string player_username;
    GameAction gameAction;

    PlayerAction() {}
    explicit PlayerAction(const std::string& player_username, const GameAction& gameAction):
            player_username(player_username), gameAction(gameAction) {}
};

#endif  // TYPES_H_
