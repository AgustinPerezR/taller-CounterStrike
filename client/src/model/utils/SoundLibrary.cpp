#include "../../../../client/include/model/utils/SoundLibrary.h"

#include <stdexcept>

#include "../../../../client/client_constants.h"

SoundLibrary::SoundLibrary() {
    loadPLayerSounds();
    loadBombSounds();
    loadRadioSounds();
}

// Chunk& SoundLibrary::get(const SoundEvent ev) {
//     const auto it = sounds.find(ev);
//     if (it == sounds.end()) {
//         throw std::runtime_error("Sound not loaded for event");
//     }
//     return it->second;
// }

Chunk* SoundLibrary::get(const SoundEvent ev) {
    const auto it = sounds.find(ev);
    if (it == sounds.end()) {
        throw std::runtime_error("Sound not loaded for event");
    }
    return &it->second;
}

bool SoundLibrary::isLoopSound(const SoundEvent ev) const {
    switch (ev) {
        case SoundEvent::Walk:
            return true;
        default:
            return false;
    }
}


void SoundLibrary::loadSound(const SoundEvent ev, const std::string& filepath) {
    if (sounds.contains(ev))
        return;

    sounds.emplace(ev, Chunk(filepath));
}

void SoundLibrary::loadPLayerSounds() {
    loadSound(SoundEvent::Walk, STEP_SOUND);
    loadSound(SoundEvent::PickUpWeapon, CHANGE_WEAPON_SOUND);
    loadSound(SoundEvent::DropWeapon, DROP_WEAPON_SOUND);
    loadSound(SoundEvent::ChangeWeapon, CHANGE_WEAPON_SOUND);
    // loadSound(SoundEvent::Shoot, SHOOT_SOUND);
    loadSound(SoundEvent::ShootGlock, GLOCK_SOUND);
    loadSound(SoundEvent::ShootAWP, AWP_SOUND);
    loadSound(SoundEvent::ShootAK47, AK47_SOUND);
    loadSound(SoundEvent::ShootM3, M3_SOUND);
    loadSound(SoundEvent::KnifeSlash, KNIFE_SLASH_SOUND);
    loadSound(SoundEvent::KnifeHit, KNIFE_HIT_SOUND);
    loadSound(SoundEvent::WeaponClick, WEAPON_CLICK_SOUND);

    loadSound(SoundEvent::TakeDamage, TAKE_DAMAGE_SOUND);
    loadSound(SoundEvent::Die, DIE_SOUND);
}

void SoundLibrary::loadBombSounds() {
    loadSound(SoundEvent::BombBeep, BOMB_BEEP_SOUND);
    loadSound(SoundEvent::BombExploded, BOMB_EXPLOSION_SOUND);
}

void SoundLibrary::loadRadioSounds() {
    loadSound(SoundEvent::PlantBomb, BOMB_PLANTED_SOUND);
    loadSound(SoundEvent::DefuseBomb, BOMB_DEFUSED_SOUND);
    loadSound(SoundEvent::Go, GO_SOUND);
    loadSound(SoundEvent::TerroristWin, TERRORIST_WIN_SOUND);
    loadSound(SoundEvent::CTWin, CT_WIN_SOUND);
}
