#include "pch.h"
#include "IniFile.hpp"
#include "FunctionHook.h"
#include "Dropdash.h"

extern "C"
{
	FunctionHook<void, EntityData1*, EntityData2*, CharObj2*> Sonic_Act1_h(Sonic_Act1);
	FunctionHook<Bool, EntityData1*, CharObj2*> Sonic_JumpCancel_h(Sonic_JumpCancel_p);
	int dropdashTimer = 0;
	bool canChargeDropDash = false;

	std::string dropdashButton;

	void Sonic_Act1_r(EntityData1* ed1, EntityData2* ed2, CharObj2* co2)
	{
		if (ed1->Action == 8) // Jump
		{
			Buttons buttons;
			if (dropdashButton == "Spindash (B and X)") buttons = AttackButtons;
			else if (dropdashButton == "Whistle (Y)")   buttons = WhistleButtons;

			if (canChargeDropDash == false && (buttons & PressedButtons[ed1->CharIndex]))
				canChargeDropDash = true;

			if (canChargeDropDash && (buttons & HeldButtons[ed1->CharIndex]))
				dropdashTimer++;
			else if (canChargeDropDash)
				dropdashTimer = 0;

			if (dropdashTimer > 15)
				QueueSound_DualEntity(767, ed1, 1, 0, 2);
		}
		else if (ed1->Action != 2) //!Walk
		{
			canChargeDropDash = false;
			dropdashTimer = 0;
		}

		Sonic_Act1_h.Original(ed1, ed2, co2);

		if (dropdashTimer > 15 && ed1->Action == 2) // Walk
		{
			ed1->Status |= (Status_Attack | Status_Ball);
			co2->SpindashSpeed = 5.0 + min(dropdashTimer / 15, 5.0);

			ed1->Action = 5; // Roll
			co2->AnimationThing.Index = 15;
			co2->Speed.x = co2->SpindashSpeed;
			co2->SpindashSpeed = 0.0;
			dropdashTimer = 0;
			PlaySound(768, 0, 0, 0);
		}
	}

	Bool Sonic_JumpCancel_r(EntityData1* a1, CharObj2* a2)
	{
		return false;
	}

	__declspec(dllexport) void __cdecl Init(const char* path, const HelperFunctions& helperFunctions)
	{
		const IniFile* config = new IniFile(std::string(path) + "\\config.ini");
		dropdashButton = config->getString("", "dropdashButton", "Spindash (B and X)");
		delete config;

		Sonic_Act1_h.Hook(Sonic_Act1_r);
		if (dropdashButton == "Spindash (B and X)") Sonic_JumpCancel_h.Hook(Sonic_JumpCancel_r);
	}

	__declspec(dllexport) ModInfo SADXModInfo = { ModLoaderVer }; // This is needed for the Mod Loader to recognize the DLL.
}