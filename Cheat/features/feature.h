#pragma once

class Feature {
public:
	virtual void DrawUI() {}			// Called every frame in ImGui thread(menu)
	virtual void DrawBackgroundUI() {}	// Called every frame in ImGui thread(outside menu)
	virtual void OnInit() = 0;			// Called once during loading
	virtual void OnUpdate() = 0;		// Called every frame in game thread
};
