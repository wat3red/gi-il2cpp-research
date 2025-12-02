#pragma once

#include "config_var.h"
#include "config_manager.h"

#include <windows.h>
#include <imgui/imgui.h>

class Config {
public:
	struct ESP {
		ConfigVar<bool> enabled{ ("esp"), ("enabled"), false };

		ConfigVar<bool> box2D{ ("esp"), ("box2d"), false };
		ConfigVar<ImColor> box2DColor{ ("esp"), ("box2DColor"), ImColor(255, 255, 255, 255) };

		ConfigVar<bool> box3D{ ("esp"), ("box3d"), false };
		ConfigVar<ImColor> box3DColor{ ("esp"), ("box3DColor"), ImColor(255, 255, 255, 255) };

		ConfigVar<bool> name{ ("esp"), ("name"), false };
		ConfigVar<ImColor> nameColor{ ("esp"), ("nameColor"), ImColor(255, 255, 255, 255) };
		ConfigVar<float> nameSize{ ("esp"), ("nameSize"), 16.f };

		ConfigVar<bool> line{ ("esp"), ("line"), false };
		ConfigVar<ImColor> lineColor{ ("esp"), ("lineColor"), ImColor(255, 255, 255, 255) };
		ConfigVar<int> lineBase{ ("esp"), ("lineBase"), 0 };
		ConfigVar<int> lineTarget{ ("esp"), ("lineTarget"), 0 };
		ConfigVar<float> lineThickness{ ("esp"), ("lineThickness"), 2.f };

		std::map<int32_t, ConfigVar<bool>> filters;
		
		ESP() {
			// Initialize filters for common entities. 
			// You can add more from your Enum list here.
			filters.emplace(2, ConfigVar<bool>(("esp_filter"), ("Monster"), true));
			filters.emplace(12, ConfigVar<bool>(("esp_filter"), ("NPC"), false));
			filters.emplace(26, ConfigVar<bool>(("esp_filter"), ("Chest"), true));
			filters.emplace(21, ConfigVar<bool>(("esp_filter"), ("GatherPoint"), true));
			filters.emplace(1, ConfigVar<bool>(("esp_filter"), ("Avatar"), false));
			filters.emplace(66, ConfigVar<bool>(("esp_filter"), ("BlackMud"), false));
		}
	} esp;

	struct SettingsUI {
		ConfigVar<int> theme{ ("settingsUI"), ("theme"), 0 };
		ConfigVar<float> menuScale{ ("settingsUI"), ("menuScale"), 1.f };

		struct Hotkeys {
			ConfigVar<int> openMenu{ ("settingsUI.hotkeys"), ("openMenu"), VK_INSERT };
			ConfigVar<int> esp{ ("settingsUI.hotkeys"), ("esp"), 0 };

		} hotkeys;
	} settingsUI;
};

extern Config config;
