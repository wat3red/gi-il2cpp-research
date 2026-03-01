#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

#include <game_api/include.h>
#include <logger/logger.h>
#include <config/imgui_config.h>
#include <config/config.h>

#include <minhook/include/MinHook.h>

class Feature {
public:
	virtual void DrawUI() {}			// Called every frame in ImGui thread(menu)
	virtual void DrawBackgroundUI() {}	// Called every frame in ImGui thread(outside menu)
	virtual void OnInit() {};			// Called once during loading
	virtual void OnUpdate() {};		// Called every frame in game thread
	virtual void UpdateHotkeys() {};	// Called every N ms in ImGui thread
};
