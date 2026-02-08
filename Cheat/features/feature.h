#pragma once

class Feature {
public:
	virtual void DrawUI() {}			// Called every frame in ImGui thread(menu)
	virtual void DrawBackgroundUI() {}	// Called every frame in ImGui thread(outside menu)
	virtual void OnInit() {};			// Called once during loading
	virtual void OnUpdate() {};		// Called every frame in game thread
	virtual void UpdateHotkeys() {};	// Called every N ms in ImGui thread
};
