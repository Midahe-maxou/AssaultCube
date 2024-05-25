#pragma once
#ifndef ASSAULTCUBE
#define ASSAULTCUBE

#include "template_ext.h"


// Offsets

#define PLAYER_STATIC_OFFSET 0x10F4F4
#define PLAYER_OFFSET_LIST { 0 }


class Player;


class Weapon : public Pointer
{
public:

	DEFINE_RO_VARIABLE(uint32_t, WeaponId, 0x4);

	std::shared_ptr<Player> getOwner();
	void setOwner(std::shared_ptr<Pointer> val);



	DEFINE_POINTER_VARIABLE(char, Name, 0xC);
	DEFINE_POINTER_VARIABLE(int, Ammo, 0x10);
	DEFINE_POINTER_VARIABLE(int, Clip, 0x14);
	DEFINE_DYNAMIC_VARIABLE(uint32_t, NbConsecutiveShot, 0x1C);
	DEFINE_DYNAMIC_VARIABLE(uint32_t, IsReloading, 0x20);

	DEFINE_MULTI_LEVEL_POINTER_VARIABLE(short, WeaponReloadLatency, 0xC, 0x108);

	DEFINE_MULTI_LEVEL_POINTER_VARIABLE(short, WeaponShootLatency, 0xC, 0x10A);

	Weapon(_In_ uintptr_t address)
		:Pointer(address) {}
}; //Size: 0x0030


class Player : public Pointer
{
public:
	DEFINE_DYNAMIC_VARIABLE(Vector3F, CameraLocation, 0x4);
	DEFINE_DYNAMIC_VARIABLE(Vector3F, Velocity, 0x10);
	DEFINE_DYNAMIC_VARIABLE(Vector3F, CameraVelocity, 0x28);
	DEFINE_DYNAMIC_VARIABLE(Vector3F, Location, 0x34);
	DEFINE_DYNAMIC_VARIABLE(Vector3F, Direction, 0x40);

	DEFINE_DYNAMIC_VARIABLE(uint32_t, HealthDisplay, 0xF8);
	DEFINE_DYNAMIC_VARIABLE(uint32_t, ArmorDisplay, 0xFC);

	DEFINE_DYNAMIC_VARIABLE(uint32_t, PistolAmmo, 0x114);
	DEFINE_DYNAMIC_VARIABLE(uint32_t, RifleAmmo, 0x128);
	DEFINE_DYNAMIC_VARIABLE(uint32_t, PistolClip, 0x13C);
	DEFINE_DYNAMIC_VARIABLE(uint32_t, RifleClip, 0x150);

	DEFINE_DYNAMIC_VARIABLE(uint32_t, KnifeRechardeTime, 0x160);
	DEFINE_DYNAMIC_VARIABLE(uint32_t, PistolRechardeTime, 0x164);
	DEFINE_DYNAMIC_VARIABLE(uint32_t, RifleRechardeTime, 0x178);

	DEFINE_DYNAMIC_VARIABLE(uint32_t, KnifeNbOfShot, 0x188);
	DEFINE_DYNAMIC_VARIABLE(uint32_t, PistolNbOfShot, 0x18C);
	DEFINE_DYNAMIC_VARIABLE(uint32_t, RifleNbOfShot, 0x1A0);

	
	DEFINE_CLASS_INSTANCE_VARIABLE(Weapon, LastWeapon, 0x370);
	DEFINE_CLASS_INSTANCE_VARIABLE(Weapon, CurrentWeapon, 0x374);
	DEFINE_CLASS_INSTANCE_VARIABLE(Weapon, MainWeapon, 0x37C);
	DEFINE_CLASS_INSTANCE_VARIABLE(Weapon, LastWeaponFired, 0x384);


	DEFINE_RO_VARIABLE(bool, isOnTheGround, 0x69);
	DEFINE_RO_VARIABLE(bool, isNotMoving, 0x70);
	DEFINE_RO_VARIABLE(bool, isLeftKeyPressed, 0x8C);
	DEFINE_RO_VARIABLE(bool, isRightKeyPressed, 0x8D);
	DEFINE_RO_VARIABLE(bool, isFWKeyPressed, 0x8E);
	DEFINE_RO_VARIABLE(bool, isBWKeyPressed, 0x8F);

	//uint8_t Fw / Bw key pressed; //0x0080
	//uint8_t Lt / Rt key pressed; //0x0081


	//char Name[255]; //0x0225



	Player(uintptr_t baseAddress)
		:Pointer(baseAddress) {}

	void setWeapon(_In_ int weaponId);
	void nextWeapon();
	void prevWeapon();
};



#endif // ASSAULTCUBE