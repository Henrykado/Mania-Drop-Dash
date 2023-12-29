#include "pch.h"
#include "IniFile.hpp"
#include "FunctionHook.h"
#include "UsercallFunctionHandler.h"
#include "Dropdash.h"

FunctionHook<void, EntityData1*, EntityData2*, CharObj2*> Sonic_Act1_h(Sonic_Act1);
UsercallFunc(Bool, Sonic_JumpCancel_h, (EntityData1* a1, CharObj2* a2), (a1, a2), 0x492F50, rEAX, rESI, rEDI);

float dropdashSpeed = 0.0f;
int dropdashTimer = 0;
bool canChargeDropDash = false;

std::string dropdashButton;
bool fallDash;
bool isBuffed;

void Sonic_Act1_r(EntityData1* ed1, EntityData2* ed2, CharObj2* co2)
{
	if (ed1->Action == 8 || (fallDash && ed1->Action == 12)) // Jump or (Fall if fallDash is true)
	{
		Buttons buttons;
		if (dropdashButton == "B and X (Spindash)") buttons = AttackButtons;
		else if (dropdashButton == "B")             buttons = Buttons_B;
		else if (dropdashButton == "X")             buttons = Buttons_X;
		else if (dropdashButton == "Y")             buttons = Buttons_Y;

		if (canChargeDropDash == false && (buttons & PressedButtons[ed1->CharIndex]))
			canChargeDropDash = true;

		if (canChargeDropDash && (buttons & HeldButtons[ed1->CharIndex]))
			dropdashTimer++;
		else {
			canChargeDropDash = false;
			dropdashTimer = 0;
		}

		if (dropdashTimer == 15 && fallDash && ed1->Action == 12) {
			ed1->Status |= Status_Ball;
			co2->AnimationThing.Index = 14;
		}

		if (dropdashTimer >= 15)
			QueueSound_DualEntity(767, ed1, 1, 0, 2);
	}
	else
	{
		canChargeDropDash = false;
		dropdashTimer = 0;
	}

	Sonic_Act1_h.Original(ed1, ed2, co2);

	if (dropdashTimer > 15 && (ed1->Action == 2 || ed1->Action == 1)) // Walk or Idle
	{
		ed1->Status |= (Status_Attack | Status_Ball);
		if (!isBuffed)
			dropdashSpeed = 5.0 + min(dropdashTimer / 15, 5.0);
		else
			dropdashSpeed = 7.5 + min(dropdashTimer / 30, 2.5);

		ed1->Action = 5; // Roll
		co2->AnimationThing.Index = 15;
		co2->Speed.x = dropdashSpeed;
		dropdashTimer = 0;
		PlaySound(768, 0, 0, 0);
	}
}

Bool Sonic_JumpCancel_r(EntityData1* a1, CharObj2* a2)
{
	if (dropdashButton == "B and X (Spindash)" ||
		dropdashButton == "X" && (Buttons_B & PressedButtons[0]) == 0 ||
		dropdashButton == "B" && (Buttons_X & PressedButtons[0]) == 0)
		return false;
	return Sonic_JumpCancel_h.Original(a1, a2);
}

extern "C"
{
	__declspec(dllexport) void __cdecl Init(const char* path, const HelperFunctions& helperFunctions)
	{
		const IniFile* config = new IniFile(std::string(path) + "\\config.ini");
		dropdashButton = config->getString("", "dropdashButton", "B and X (Spindash)");
		fallDash = config->getBool("", "fallDash", false);
		isBuffed = config->getBool("", "isBuffed", false);
		delete config;

		Sonic_Act1_h.Hook(Sonic_Act1_r);
		if (dropdashButton != "Y") Sonic_JumpCancel_h.Hook(Sonic_JumpCancel_r);
	}

	__declspec(dllexport) ModInfo SADXModInfo = { ModLoaderVer }; // This is needed for the Mod Loader to recognize the DLL.
}