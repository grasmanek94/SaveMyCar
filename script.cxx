/*
	Save & Load your favourite cars (with tuning, colors and everything)

	Copyright (c) 2015 - Rafa³ 'grasmanek94/Gamer_Z' Grasman

	License: MIT

	grasmanek94 at gmail dot com

	Instructions:

		Place .ASI file where you GTA is.

		Make sure you have ScriptHookV installed.

		Saving a vehicle: 
			Press keys in the following order:
				CTRL CTRL S SPACE

			Then type in the id of the slot you want to save your current vehicle to (0123456789) [min 0, max 999]

				Hit ENTER

		Loading a vehicle: 
			Press keys in the following order:
				CTRL CTRL L SPACE

			Then type in the id of the slot you want to load a vehicle from (0123456789) [min 0, max 999]

				Hit ENTER

		Done!
*/

#include <string>
#include <ctime>
#include <iostream>
#include <array>
#include <fstream>
#include <vector>

#include "script.hxx"
#include "KeyboardManager.hxx"

#include <Windows.h>
#include <Shlobj.h>
#include <Psapi.h>

#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Shell32.lib")

namespace VehicleLoader
{
	template <typename T = float> struct Vector3B
	{
		T x;
		T y;
		T z;

		Vector3B()
			: x(T()), y(T()), z(T())
		{ }

		Vector3B(T x, T y, T z)
			: x(x), y(y), z(z)
		{ }

		Vector3B<T> operator*(T rhs)
		{
			return Vector3(x * rhs, y * rhs, z * rhs);
		}

		Vector3B<T>& operator*=(T rhs)
		{
			x *= rhs;
			y *= rhs;
			z *= rhs;

			return *this;
		}
	};

	enum CarManagerStates
	{
		CMS_None,
		CMS_Save,
		CMS_Load
	};

	CarManagerStates CarManagerState = CMS_None;

	struct ModInfo
	{
		int Mod;
		int Variation;
	};

	struct VehicleInfo
	{
		Hash Model;
		Vector3B<int> TireSmokeColor;
		Vector3B<int> NeonLightColor;
		std::array<Vector3B<int>, 2> VehicleColor;
		std::array<ModInfo, 25> Mods;
		std::array<int, 2> ExtraColor;
		int WindowTint;
		int WheelType;
		int CRoofstate;
		std::array<int, 4> NeonLights;
		int Livery;
		int Plate;

		VehicleInfo()
		{
			memset(this, 0, sizeof(VehicleInfo));
		}
	};

	class VehicleManager
	{
		std::array<VehicleInfo, 1000> Vehicles;

		std::ofstream Writer;

		const std::wstring filename;
		std::wstring filepath;
	public:

		int currentselection;

		const std::vector<unsigned char> save_sequence;
		const std::vector<unsigned char> load_sequence;

		VehicleManager()
			: filename(L"Rockstar Games\\GTA V\\SavedVehicles.dat"),
			save_sequence({ VK_CONTROL, VK_CONTROL, 'S', VK_SPACE }),
			load_sequence({ VK_CONTROL, VK_CONTROL, 'L', VK_SPACE }),
			currentselection(0)
		{
			WCHAR path[MAX_PATH];
			HRESULT hr = SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, path);
			filepath.append(path);
			if (filepath[filepath.size() - 1] != (wchar_t)'\\' || filepath[filepath.size() - 1] != (wchar_t)'/')
			{
				filepath.append(L"\\");
			}
			filepath.append(filename);

			std::ifstream Reader(filepath, std::ifstream::in | std::ifstream::binary);

			if (Reader.is_open())
			{
				Reader.read(reinterpret_cast<char*>(&Vehicles), sizeof(Vehicles));
				Reader.close();
				Writer.open(filepath, std::ofstream::out | std::ofstream::in | std::ofstream::binary);
			}
			else
			{
				Writer.open(filepath, std::ofstream::out | std::ofstream::binary);
			}		
		}

		bool Save(size_t key)
		{
			if (!Writer.is_open())
			{
				Writer.open(filepath, std::ofstream::out | std::ofstream::binary);
				if (!Writer.is_open())
				{
					return false;
				}
			}

			Ped playerPed = PLAYER::PLAYER_PED_ID();
			if (!ENTITY::DOES_ENTITY_EXIST(playerPed))
			{
				return false;
			}

			Vehicle playerVeh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
			if (!ENTITY::DOES_ENTITY_EXIST(playerVeh))
			{
				return false;
			}

			VehicleInfo* info = &Vehicles[key];

			info->Model = ENTITY::GET_ENTITY_MODEL(playerVeh);
			info->WindowTint = VEHICLE::GET_VEHICLE_WINDOW_TINT(playerVeh);
			info->WheelType = VEHICLE::GET_VEHICLE_WHEEL_TYPE(playerVeh);
			info->CRoofstate = VEHICLE::GET_CONVERTIBLE_ROOF_STATE(playerVeh);
			info->Livery = VEHICLE::GET_VEHICLE_LIVERY(playerVeh);
			info->Plate = VEHICLE::GET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(playerVeh);

			VEHICLE::_GET_VEHICLE_NEON_LIGHTS_COLOUR(playerVeh, &info->NeonLightColor.x, &info->NeonLightColor.y, &info->NeonLightColor.z);
			VEHICLE::GET_VEHICLE_CUSTOM_PRIMARY_COLOUR(playerVeh, &info->VehicleColor[0].x, &info->VehicleColor[0].y, &info->VehicleColor[0].z);
			VEHICLE::GET_VEHICLE_CUSTOM_SECONDARY_COLOUR(playerVeh, &info->VehicleColor[1].x, &info->VehicleColor[1].y, &info->VehicleColor[1].z);
			VEHICLE::GET_VEHICLE_TYRE_SMOKE_COLOR(playerVeh, &info->TireSmokeColor.x, &info->TireSmokeColor.y, &info->TireSmokeColor.z);
			VEHICLE::GET_VEHICLE_EXTRA_COLOURS(playerVeh, &info->ExtraColor[0], &info->ExtraColor[1]);

			for (int i = 0; i < info->Mods.size(); ++i)
			{
				if (i >= 17 && i <= 22)
				{
					info->Mods[i].Mod = VEHICLE::IS_TOGGLE_MOD_ON(playerVeh, i);
					info->Mods[i].Variation = 0;			
				}
				else
				{
					info->Mods[i].Mod = VEHICLE::GET_VEHICLE_MOD(playerVeh, i);
					info->Mods[i].Variation = VEHICLE::GET_VEHICLE_MOD_VARIATION(playerVeh, i);
				}
			}

			for (int i = 0; i < info->NeonLights.size(); ++i)
			{
				info->NeonLights[i] = VEHICLE::_IS_VEHICLE_NEON_LIGHT_ENABLED(playerVeh, i);
			}

			try
			{
				//not much documentation to be found about seek past eof so we do it the 'safe way'
				Writer.seekp(0);
				Writer.write(reinterpret_cast<char*>(&Vehicles), sizeof(Vehicles));
				Writer.flush();
			}
			catch (std::exception e)
			{
				return false;
			}

			return true;
		}

		bool Load(size_t key)
		{
			Ped playerPed = PLAYER::PLAYER_PED_ID();
			if (!ENTITY::DOES_ENTITY_EXIST(playerPed))
			{
				return false;
			}

			VehicleInfo* info = &Vehicles[key];

			if (STREAMING::IS_MODEL_IN_CDIMAGE(info->Model) && STREAMING::IS_MODEL_A_VEHICLE(info->Model))
			{
				STREAMING::REQUEST_MODEL(info->Model);

				while (!STREAMING::HAS_MODEL_LOADED(info->Model)) 
				{ 
					WAIT(0); 
				}

				Vector3 coords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(PLAYER::PLAYER_PED_ID(), 0.0, 5.0, 0.0);
				Vehicle playerVeh = VEHICLE::CREATE_VEHICLE(info->Model, coords.x, coords.y, coords.z, 0.0, 1, 1);
				VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(playerVeh);

				VEHICLE::SET_VEHICLE_MOD_KIT(playerVeh, 0);
				VEHICLE::SET_VEHICLE_WINDOW_TINT(playerVeh, info->WindowTint);
				VEHICLE::SET_VEHICLE_WHEEL_TYPE(playerVeh, info->WheelType);
				VEHICLE::SET_VEHICLE_LIVERY(playerVeh, info->Livery);
				VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(playerVeh, info->Plate);

				if (info->CRoofstate != 0 && info->CRoofstate != 3)//0 & 3 == closed/closing, 1 & 2 == opening/open
				{
					VEHICLE::LOWER_CONVERTIBLE_ROOF(playerVeh, true);
				}

				VEHICLE::_SET_VEHICLE_NEON_LIGHTS_COLOUR(playerVeh, info->NeonLightColor.x, info->NeonLightColor.y, info->NeonLightColor.z);
				VEHICLE::SET_VEHICLE_CUSTOM_PRIMARY_COLOUR(playerVeh, info->VehicleColor[0].x, info->VehicleColor[0].y, info->VehicleColor[0].z);
				VEHICLE::SET_VEHICLE_CUSTOM_SECONDARY_COLOUR(playerVeh, info->VehicleColor[1].x, info->VehicleColor[1].y, info->VehicleColor[1].z);
				VEHICLE::SET_VEHICLE_TYRE_SMOKE_COLOR(playerVeh, info->TireSmokeColor.x, info->TireSmokeColor.y, info->TireSmokeColor.z);
				VEHICLE::SET_VEHICLE_EXTRA_COLOURS(playerVeh, info->ExtraColor[0], info->ExtraColor[1]);

				for (int i = 0; i < info->Mods.size(); ++i)
				{
					if (i >= 17 && i <= 22)
					{
						VEHICLE::TOGGLE_VEHICLE_MOD(playerVeh, i, info->Mods[i].Mod);
					}
					else
					{
						VEHICLE::SET_VEHICLE_MOD(playerVeh, i, info->Mods[i].Mod, info->Mods[i].Variation);
					}
				}

				for (int i = 0; i < info->NeonLights.size(); ++i)
				{
					VEHICLE::_SET_VEHICLE_NEON_LIGHT_ENABLED(playerVeh, i, info->NeonLights[i]);
				}

				ENTITY::SET_ENTITY_HEADING(playerVeh, ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()));
				PED::SET_PED_INTO_VEHICLE(PLAYER::PLAYER_PED_ID(), playerVeh, -1);

				WAIT(0);

				STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(info->Model);
				ENTITY::SET_VEHICLE_AS_NO_LONGER_NEEDED(&playerVeh);

				return true;
			}
			return false;
		}
	};

	VehicleManager * _VehicleManager;
};

class PatternScanner
{
public:
	static bool memory_compare(const BYTE *data, const BYTE *pattern, const char *mask)
	{
		for (; *mask; ++mask, ++data, ++pattern)
		{
			if (*mask == 'x' && *data != *pattern)
			{
				return false;
			}
		}
		return (*mask) == NULL;
	}

	static size_t FindPattern(std::vector<unsigned short> pattern)
	{
		size_t i;
		size_t size;
		size_t address;

		MODULEINFO info = { 0 };

		address = (size_t)GetModuleHandle(NULL);
		GetModuleInformation(GetCurrentProcess(), GetModuleHandle(NULL), &info, sizeof(MODULEINFO));
		size = (size_t)info.SizeOfImage;

		std::vector<unsigned char> search;
		std::vector<char> mask;

		for (auto elem : pattern)
		{
			if (elem != 0xFFFF)
			{
				search.push_back((unsigned char)elem);
				mask.push_back('x');
			}
			else
			{
				search.push_back(0);
				mask.push_back('?');
			}
		}

		for (i = 0; i < size; ++i)
		{
			if (memory_compare((BYTE *)(address + i), (BYTE *)search.data(), mask.data()))
			{
				return (UINT64)(address + i);
			}
		}
		return 0;
	}
};

class CustomHelpText
{
	const unsigned char bytes[3][2] = 
	{ 
		{ 0x75, 0x4B },
		{ 0x75, 0x1D },
		{ 0x90, 0x90 }
	};

	char* source_string;

	size_t* jumps[2];

	void ReplaceCode(size_t* address, const unsigned char* newbytes, size_t size)
	{
		unsigned long dwProtect;
		unsigned long dwProtect2;

		VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE, &dwProtect);
		memcpy(address, newbytes, size);
		VirtualProtect((LPVOID)address, size, dwProtect, &dwProtect2);
	}

	size_t timer;

	bool Drawing()
	{
		return ((char*)jumps[0])[0] == 0x90;
	}

	std::string TAG;
public:
	CustomHelpText(const std::string& TAG)
		: TAG(TAG)
	{
		timer = 0;

		size_t offset_addr = PatternScanner::FindPattern(std::vector<unsigned short>(
		{
			0x41, 0xF6, 0xC0, 0x01, 0x75, 0x43, 0x83, 0xF9, 0x03, 0x7D, 0x3E
			/*,0x48, 0x63, 0xD1, 
			0x4C, 0x8D, 0x05, 0x8C, 0x4C, 0x36, 0xFF, 0x48, 0x8B, 0xC2, 0x48, 0x69, 0xC0*/
		}));

		size_t offset = *((unsigned long*)(offset_addr + 0x23));
		source_string =// (char*)((size_t)GetModuleHandle(NULL) + 0x234DD30);
			(char*)((size_t)GetModuleHandle(NULL) + offset);

		size_t begin_addr = PatternScanner::FindPattern(std::vector<unsigned short>(
		{
			0x44, 0x8B, 0xC7, 0x48, 0x8B, 0xD6, 0x49, 0x8B, 0xCB, 0x89, 0x44
			/*, 0xFFFF, 0xFFFF, 0xE8, 
			0x57, 0xE3, 0xFF, 0xFF, 0x48, 0x8D, 0x3D, 0x70, 0xD9, 0x35, 0xFF, 0x4C, 0x8D, 0x0D, 
			0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x80, 0xBC, 0x3B, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x00, 
			0x75, 0x4B, 0x48, 0x8B, 0xC3, 0x48, 0x8D, 0x4C, 0x24, 0xFFFF, 0x48, 0x69, 0xC0, 0xFFFF, 
			0xFFFF, 0xFFFF, 0xFFFF, 0x49, 0x03, 0xC1, 0x48, 0x2B, 0xC1, 0x0F, 0xB6, 0x11, 0x44, 
			0x0F, 0xB6, 0x04, 0x01, 0x41, 0x2B, 0xD0, 0x75, 0x08, 0x48, 0xFF, 0xC1, 0x45, 0x85, 
			0xC0, 0x75, 0xEB, 0x85, 0xD2, 0x75, 0x1D*/
		}));

		jumps[0] = (size_t*)(begin_addr + 0x28);
		jumps[1] = (size_t*)(begin_addr + 0x56);
	}

	void Begin()
	{
		if (!Drawing())
		{
			ReplaceCode(jumps[0], bytes[2], 2);
			ReplaceCode(jumps[1], bytes[2], 2);

			timer = 0;
		}
	}

	void SetText(const std::string& text)//char limit is ~599
	{
		std::string combined(TAG + text);
		memcpy(source_string, combined.c_str(), combined.size() + 1);
	}

	void ShowThisFrame()
	{
		UI::DISPLAY_HELP_TEXT_THIS_FRAME("LETTERS_HELP2", false);
	}

	void End()
	{
		if (Drawing())
		{
			source_string[0] = 0;

			ReplaceCode(jumps[0], bytes[0], 2);
			ReplaceCode(jumps[1], bytes[1], 2);

			UI::HIDE_HELP_TEXT_THIS_FRAME();

			timer = 0;
		}
	}

	void Tick()
	{
		size_t time_now = GetTickCount();
		if (time_now < timer)
		{
			ShowThisFrame();
		}
		else
		{
			End();
		}
	}

	void ShowTimedText(const std::string& text, size_t how_many_ms)
	{
		Begin();
		SetText(text);
		timer = (size_t)GetTickCount() + how_many_ms;
	}
};

CustomHelpText* cht;

void OnKeyStateChange(bool pressed, unsigned char vkey)
{
	if (pressed)
	{
		if (VehicleLoader::CarManagerState == VehicleLoader::CMS_None)
		{
			if (kbmgr.BufferContainsArray(VehicleLoader::_VehicleManager->load_sequence))
			{
				cht->Begin();
				kbmgr.ClearBuffer();
				kbmgr.BufferAcceptOnlyNumeric = true;

				VehicleLoader::CarManagerState = VehicleLoader::CMS_Load;
			}
			else if (kbmgr.BufferContainsArray(VehicleLoader::_VehicleManager->save_sequence))
			{
				cht->Begin();
				kbmgr.ClearBuffer();
				kbmgr.BufferAcceptOnlyNumeric = true;

				VehicleLoader::CarManagerState = VehicleLoader::CMS_Save;
			}
		}
		else
		{
			std::vector<unsigned char> seq(kbmgr.GetSequence(4, false));

			VehicleLoader::_VehicleManager->currentselection = atoi((const char*)seq.data());
			if (VehicleLoader::_VehicleManager->currentselection < 0)
			{
				VehicleLoader::_VehicleManager->currentselection = 0;
			}
			if (VehicleLoader::_VehicleManager->currentselection > 999)
			{
				VehicleLoader::_VehicleManager->currentselection = 999;
			}

			if (vkey == VK_RETURN || vkey == VK_BACK)
			{
				cht->End();
				kbmgr.BufferAcceptOnlyNumeric = false;

				if (vkey == VK_RETURN)
				{
					if (VehicleLoader::CarManagerState == VehicleLoader::CMS_Save)
					{
						if (VehicleLoader::_VehicleManager->Save(VehicleLoader::_VehicleManager->currentselection))
						{
							cht->ShowTimedText("Vehicle Saved at slot " + std::to_string(VehicleLoader::_VehicleManager->currentselection) + ".", 4000);
						}
						else
						{
							cht->ShowTimedText("There was a problem while saving your vehicle, please make sure you are in a vehicle and that your documents directory is writable.", 4000);
						}
					}
					else if (VehicleLoader::CarManagerState == VehicleLoader::CMS_Load)
					{
						if (VehicleLoader::_VehicleManager->Load(VehicleLoader::_VehicleManager->currentselection))
						{
							cht->ShowTimedText("Vehicle Loaded from slot " + std::to_string(VehicleLoader::_VehicleManager->currentselection) + ".", 4000);
						}
						else
						{
							cht->ShowTimedText("There was a problem while loading your vehicle, please make sure that you have saved a vehicle earlies in this slot.", 4000);
						}
					}
				}
				else
				{
					cht->ShowTimedText("Operation canceled.", 2000);
				}

				VehicleLoader::CarManagerState = VehicleLoader::CMS_None;
				VehicleLoader::_VehicleManager->currentselection = 0;
			}
		}
	}
}

void main()
{
	VehicleLoader::_VehicleManager = new VehicleLoader::VehicleManager();
	cht = new CustomHelpText("GZ-SMC| ");

	kbmgr.SetOnKeyStateChangeFunction(OnKeyStateChange);

	KeyboardManager::TACS safetyMechanism;
	while (true)
	{
		while (kbmgr.AntiCrashSychronization.try_pop(safetyMechanism))
		{
			kbmgr.CheckKeys(safetyMechanism.first, safetyMechanism.second);
		}

		if (VehicleLoader::CarManagerState != VehicleLoader::CMS_None)
		{
			std::string info;
			if (VehicleLoader::CarManagerState == VehicleLoader::CMS_Load)
			{
				info.append("Type the saved ID you want to load and hit ENTER (BACKSPACE to cancel): ");
			}
			else
			{
				info.append("Type the ID you want to save the current vehicle to and hit ENTER (BACKSPACE to cancel): ");
			}
			info.append(std::to_string(VehicleLoader::_VehicleManager->currentselection));

			cht->SetText(info);
			cht->ShowThisFrame();
		}

		cht->Tick();
		WAIT(0);
	}
}

void ScriptMain()
{
	srand(GetTickCount());
	main();
}