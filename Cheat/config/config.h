#pragma once

#include "config_var.h"
#include "config_manager.h"

#include <windows.h>
#include <imgui/imgui.h>
#include <map>
#include <vector>

class Config {
public:
	struct ESPEntityType {
		int32_t type_id;
		std::string type_name;

		// Visual settings for this entity type
		ConfigVar<bool> enabled;
		ConfigVar<bool> box2D;
		ConfigVar<ImColor> box2DColor;
		ConfigVar<bool> name;
		ConfigVar<ImColor> nameColor;
		ConfigVar<float> nameSize;
		ConfigVar<bool> line;
		ConfigVar<ImColor> lineColor;
		ConfigVar<float> lineThickness;

		ESPEntityType()
			: type_id(0), type_name(""),
			enabled("esp_type.", "enabled", false),
			box2D("esp_type.", "box2D", false),
			box2DColor("esp_type.", "box2DColor", ImColor(255, 255, 255, 255)),
			name("esp_type.", "name", false),
			nameColor("esp_type.", "nameColor", ImColor(255, 255, 255, 255)),
			nameSize("esp_type.", "nameSize", 16.f),
			line("esp_type.", "line", false),
			lineColor("esp_type.", "lineColor", ImColor(255, 255, 255, 255)),
			lineThickness("esp_type.", "lineThickness", 2.f) {
		}

		ESPEntityType(int32_t _type_id, const std::string& _type_name)
			: type_id(_type_id), type_name(_type_name),
			enabled("esp_type." + _type_name, "enabled", false),
			box2D("esp_type." + _type_name, "box2D", false),
			box2DColor("esp_type." + _type_name, "box2DColor", ImColor(255, 255, 255, 255)),
			name("esp_type." + _type_name, "name", false),
			nameColor("esp_type." + _type_name, "nameColor", ImColor(255, 255, 255, 255)),
			nameSize("esp_type." + _type_name, "nameSize", 16.f),
			line("esp_type." + _type_name, "line", false),
			lineColor("esp_type." + _type_name, "lineColor", ImColor(255, 255, 255, 255)),
			lineThickness("esp_type." + _type_name, "lineThickness", 2.f) {
		}
	};

	struct ESP {
		ConfigVar<bool> enabled{ ("esp"), ("enabled"), false };
		ConfigVar<bool> distance{ ("esp"), ("distance"), false };
		ConfigVar<float> maxDistance{ ("esp"), ("maxDistance"), 100.0f };
		ConfigVar<int> lineBase{ ("esp"), ("lineBase"), 0 }; // 0: Bottom, 1: Center, 2: Top

		// Per-entity-type configurations
		std::map<int32_t, ESPEntityType> entity_types;

		ESP() {
			// Initialize all entity types with their settings
			/*entity_types.emplace(0, ESPEntityType(0, "None"));
			entity_types.emplace(1, ESPEntityType(1, "Avatar"));*/
			entity_types.emplace(2, ESPEntityType(2, "Monster")); // add
			entity_types.emplace(3, ESPEntityType(3, "Bullet")); // add
			/*entity_types.emplace(4, ESPEntityType(4, "AttackPhysicalUnit"));
			entity_types.emplace(5, ESPEntityType(5, "AOE"));
			entity_types.emplace(6, ESPEntityType(6, "Camera"));
			entity_types.emplace(7, ESPEntityType(7, "EnviroArea"));
			entity_types.emplace(8, ESPEntityType(8, "Equip"));*/
			entity_types.emplace(9, ESPEntityType(9, "MonsterEquip")); // add
			/*entity_types.emplace(10, ESPEntityType(10, "Grass"));
			entity_types.emplace(11, ESPEntityType(11, "Level"));*/
			entity_types.emplace(12, ESPEntityType(12, "NPC")); // add
			/*entity_types.emplace(13, ESPEntityType(13, "TransPointFirst"));
			entity_types.emplace(14, ESPEntityType(14, "TransPointFirstGadget"));*/
			entity_types.emplace(15, ESPEntityType(15, "TransPointSecond")); // add
			/*entity_types.emplace(16, ESPEntityType(16, "TransPointSecondGadget"));*/
			entity_types.emplace(17, ESPEntityType(17, "DropItem")); // add
			entity_types.emplace(18, ESPEntityType(18, "Field")); // add
			entity_types.emplace(19, ESPEntityType(19, "Gadget")); // add
			/*entity_types.emplace(20, ESPEntityType(20, "Water"));
			entity_types.emplace(21, ESPEntityType(21, "GatherPoint")); */
			entity_types.emplace(22, ESPEntityType(22, "GatherObject")); // add
			/*entity_types.emplace(23, ESPEntityType(23, "AirflowField"));
			entity_types.emplace(24, ESPEntityType(24, "SpeedupField"));*/
			entity_types.emplace(25, ESPEntityType(25, "Gear")); // add
			entity_types.emplace(26, ESPEntityType(26, "Chest")); // add
			/*entity_types.emplace(27, ESPEntityType(27, "EnergyBall"));
			entity_types.emplace(28, ESPEntityType(28, "ElemCrystal"));
			entity_types.emplace(29, ESPEntityType(29, "Timeline"));
			entity_types.emplace(30, ESPEntityType(30, "Worktop"));
			entity_types.emplace(31, ESPEntityType(31, "Team"));*/
			entity_types.emplace(32, ESPEntityType(32, "Platform")); // add
			//entity_types.emplace(33, ESPEntityType(33, "AmberWind"));
			entity_types.emplace(34, ESPEntityType(34, "EnvAnimal")); // add
			//entity_types.emplace(35, ESPEntityType(35, "SealGadget"));
			/*entity_types.emplace(36, ESPEntityType(36, "Tree"));
			entity_types.emplace(37, ESPEntityType(37, "Bush"));
			entity_types.emplace(38, ESPEntityType(38, "QuestGadget"));
			entity_types.emplace(39, ESPEntityType(39, "Lightning"));
			entity_types.emplace(40, ESPEntityType(40, "RewardPoint"));
			entity_types.emplace(41, ESPEntityType(41, "RewardStatue"));
			entity_types.emplace(42, ESPEntityType(42, "MPLevel"));
			entity_types.emplace(43, ESPEntityType(43, "WindSeed"));
			entity_types.emplace(44, ESPEntityType(44, "MpPlayRewardPoint"));
			entity_types.emplace(45, ESPEntityType(45, "ViewPoint"));
			entity_types.emplace(46, ESPEntityType(46, "RemoteAvatar"));
			entity_types.emplace(47, ESPEntityType(47, "GeneralRewardPoint"));
			entity_types.emplace(48, ESPEntityType(48, "PlayTeam"));
			entity_types.emplace(49, ESPEntityType(49, "OfferingGadget"));
			entity_types.emplace(50, ESPEntityType(50, "EyePoint"));
			entity_types.emplace(51, ESPEntityType(51, "MiracleRing"));
			entity_types.emplace(52, ESPEntityType(52, "Foundation"));*/
			//entity_types.emplace(53, ESPEntityType(53, "WidgetGadget"));
			entity_types.emplace(54, ESPEntityType(54, "Vehicle")); // add
			/*entity_types.emplace(55, ESPEntityType(55, "DangerZone"));
			entity_types.emplace(56, ESPEntityType(56, "EchoShell"));
			entity_types.emplace(57, ESPEntityType(57, "HomeGatherObject"));
			entity_types.emplace(58, ESPEntityType(58, "Projector"));
			entity_types.emplace(59, ESPEntityType(59, "Screen"));
			entity_types.emplace(60, ESPEntityType(60, "CustomTile"));
			entity_types.emplace(61, ESPEntityType(61, "FishPool"));
			entity_types.emplace(62, ESPEntityType(62, "FishRod"));
			entity_types.emplace(63, ESPEntityType(63, "CustomGadget"));
			entity_types.emplace(64, ESPEntityType(64, "RoguelikeOperatorGadget"));
			entity_types.emplace(65, ESPEntityType(65, "ActivityInteractGadget"));
			entity_types.emplace(66, ESPEntityType(66, "BlackMud"));
			entity_types.emplace(67, ESPEntityType(67, "SubEquip"));
			entity_types.emplace(68, ESPEntityType(68, "UIInteractGadget"));
			entity_types.emplace(69, ESPEntityType(69, "NightCrowGadget"));
			entity_types.emplace(70, ESPEntityType(70, "Partner"));
			entity_types.emplace(71, ESPEntityType(71, "DeshretObeliskGadget"));
			entity_types.emplace(72, ESPEntityType(72, "CoinCollectLevelGadget"));
			entity_types.emplace(73, ESPEntityType(73, "UgcSpecialGadget"));
			entity_types.emplace(74, ESPEntityType(74, "UgcTowerLevelUpGadget"));
			entity_types.emplace(75, ESPEntityType(75, "JourneyGearOperatorGadget"));
			entity_types.emplace(76, ESPEntityType(76, "CurveMoveGadget"));
			entity_types.emplace(77, ESPEntityType(77, "MagnetPlant"));*/
		}

		ESPEntityType* GetEntityType(int32_t type_id) {
			auto it = entity_types.find(type_id);
			if (it != entity_types.end()) {
				return &it->second;
			}
			return nullptr;
		}
	} esp;

	struct AutoTalk {
		ConfigVar<bool> enabled{ "auto_talk", "enabled", false };
		ConfigVar<bool> auto_choose_reply{ "auto_talk", "auto_choose_reply", false };
		ConfigVar<bool> fast_dialog{ "auto_talk", "fast_dialog", false };
		ConfigVar<float> speed_modifier{ "auto_talk", "speed_modifier", 2.f };
	} auto_talk;

	struct GodMode {
		ConfigVar<bool> enabled{ "god_mode", "enabled", false };
	} god_mode;

	struct KillAura {
		ConfigVar<bool> enabled{ "kill_aura", "enabled", false };
		ConfigVar<float> range{ "kill_aura", "range", 5.f };
		ConfigVar<float> delay{ "kill_aura", "delay", 10.f };
	} kill_aura;

	struct AutoLoot {
		ConfigVar<bool> enabled{ "auto_loot", "enabled", false };
		ConfigVar<bool> treasures{ "auto_loot", "treasures", false };
		ConfigVar<float> loot_range{ "auto_loot", "loot_range", 30.f };
		ConfigVar<float> treasure_range{ "auto_loot", "treasure_range", 5.f };
	} auto_loot;

	struct GameSpeed {
		ConfigVar<bool> enabled{ "game_speed", "enabled", false };
		ConfigVar<float> speed{ "game_speed", "speed", 1.f };
	} game_speed;

	struct MapTeleport {
		ConfigVar<bool> enabled{ "map_teleport", "enabled", false };
		ConfigVar<ImGuiKey> enable_hotkey{ "map_teleport", "enable_hotkey", ImGuiKey_B };
	} map_teleport;

	struct CustomTeleports {
		ConfigVar<bool> enabled{ "custom_teleports", "enabled", false };
		ConfigVar<bool> auto_teleport{ "custom_teleports", "auto_teleport", false };
		ConfigVar<float> teleport_delay{ "custom_teleports", "teleport_delay", 7.f };
	} custom_teleports;

	struct QuestTeleport {
		ConfigVar<bool> enabled{ "quest_teleport", "enabled", false };
		ConfigVar<ImGuiKey> teleport_hotkey{ "quest_teleport", "teleport_hotkey", ImGuiKey_V };
	} quest_teleport;

	struct Noclip {
		ConfigVar<bool> enabled{ "noclip", "enabled", false };
		ConfigVar<float> speed{ "noclip", "speed", 10.f };
		ConfigVar<ImGuiKey> enable_hotkey{ "noclip", "enable_hotkey", ImGuiKey_T };
	} noclip;

	struct SkipCutscene {
		ConfigVar<bool> enabled{ "skip_cutscene", "enabled", false };
	} skip_cutscene;

	struct AutoDestroy {
		ConfigVar<bool> enabled{ "auto_destroy", "enabled", false };
		ConfigVar<float> range{ "auto_destroy", "range", 10.f };
	} auto_destroy;

	struct CostumeMapping {
		int id;
		uint32_t avatar_id;
		uint32_t costume_id;
		uint32_t flycloak_id;

		CostumeMapping(int _id, uint32_t _avatar_id, uint32_t _costume_id, uint32_t _flycloak_id)
			: id(_id), avatar_id(_avatar_id), costume_id(_costume_id), flycloak_id(_flycloak_id) {
		}
	};

	struct CostumeChanger {
		ConfigVar<bool> enabled{ "costume_changer", "enabled", false };
		ConfigVar<int>  mapping_count{ "costume_changer", "mapping_count", 0 };
		std::vector<CostumeMapping> mappings;

		void LoadMappings() {
			mappings.clear();
			const int count = mapping_count;
			for (int i = 0; i < count; i++) {
				const std::string section = "costume_changer.costumes.id_" + std::to_string(i);
				ConfigVar<uint32_t> av{ section, "avatar_id",   0u };
				ConfigVar<uint32_t> co{ section, "costume_id",  0u };
				ConfigVar<uint32_t> fl{ section, "flycloak_id", 0u };
				mappings.emplace_back(i, (uint32_t)av, (uint32_t)co, (uint32_t)fl);
			}
		}

		void SaveMappings() {
			const int old_count = mapping_count;   // read previous value
			const int new_count = (int)mappings.size();

			// Remove old entries if list shrank
			for (int i = new_count; i < old_count; i++) {
				const std::string section = "costume_changer.costumes.id_" + std::to_string(i);

				ConfigVar<uint32_t>{ section, "avatar_id", 0u }.Remove();
				ConfigVar<uint32_t>{ section, "costume_id", 0u }.Remove();
				ConfigVar<uint32_t>{ section, "flycloak_id", 0u }.Remove();
			}

			mapping_count = new_count;

			for (int i = 0; i < new_count; i++) {
				mappings[i].id = i;

				const std::string section = "costume_changer.costumes.id_" + std::to_string(i);

				ConfigVar<uint32_t>{ section, "avatar_id", 0u } = mappings[i].avatar_id;
				ConfigVar<uint32_t>{ section, "costume_id", 0u } = mappings[i].costume_id;
				ConfigVar<uint32_t>{ section, "flycloak_id", 0u } = mappings[i].flycloak_id;
			}
		}

		void SaveMapping(uint32_t avatar_id, uint32_t costume_id, uint32_t flycloak_id) {
			for (auto& m : mappings) {
				if (m.avatar_id == avatar_id) {
					m.costume_id = costume_id;
					m.flycloak_id = flycloak_id;
					SaveMappings();
					return;
				}
			}
			mappings.emplace_back((int)mappings.size(), avatar_id, costume_id, flycloak_id);
			SaveMappings();
		}


		CostumeMapping* GetMapping(uint32_t avatar_id) {
			for (auto& m : mappings)
				if (m.avatar_id == avatar_id) return &m;
			return nullptr;
		}
	} costume_changer;

	struct SettingsUI {
		ConfigVar<int> theme{ ("settingsUI"), ("theme"), 0 };
		ConfigVar<float> menuScale{ ("settingsUI"), ("menuScale"), 1.f };

		struct Hotkeys {
			ConfigVar<int> open_menu{ ("settingsUI.hotkeys"), ("openMenu"), VK_INSERT };
			ConfigVar<int> esp{ ("settingsUI.hotkeys"), ("esp"), 0 };

		} hotkeys;
	} ui_settings;
};

extern Config config;
