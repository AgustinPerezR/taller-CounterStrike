#ifndef ENTITYFACTORY_H
#define ENTITYFACTORY_H

#include "../../../../common/dtos/EntitySnapshot.h"
#include "../../../../common/game_info/game_info.h"

#include "ComponentManager.h"

///
/// @brief Attach the corresponding components to the entity type that we need to create.
/// Furthermore, it initializes the components to the right values.
///
class EntityFactory {
private:
    ComponentManager& comp_mgr;

    void create_player_entt(const Entity& new_entt, const EntitySnapshot& snap) const;
    void create_weapon_entt(const Entity& new_entt, const EntitySnapshot& snap) const;
    void create_bullet_entt(const Entity& new_entt, const EntitySnapshot& snap) const;
    void create_bomb_entt(const Entity& new_entt, const EntitySnapshot& snap) const;

    void setPlayersComponentsCapacity(int numPlayers) const;
    void setBombComponentsCapacity(int numBomb) const;

public:
    EntityFactory(ComponentManager& cm, int numPlayers);

    void create_specific_entity(const Entity& new_entt, const EntitySnapshot& snap) const;

    void createEntityPlayer(const Entity& new_entt, const LocalPlayerInfo& p);

    void destroy(const Entity& entt) const;
};

#endif  // ENTITYFACTORY_H
