#include "AssaultCube.h"


#include "template_ext.h"


std::shared_ptr<Player> Weapon::getOwner() {
	uintptr_t address = ReadMemory<uintptr_t>(addressPtr, 0x8);
	return std::make_shared<Player>(Player(address));
}

void Weapon::setOwner(std::shared_ptr<Pointer> val) {
	WriteMemory<intptr_t>(addressPtr, 0x8, val->getAddressPtr());
};


void Player::setWeapon(_In_ int weaponId)
{
}