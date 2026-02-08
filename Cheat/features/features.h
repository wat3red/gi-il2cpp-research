#pragma once
#include "feature.h"

#include <vector>


namespace features
{
	void InitAllFeatures();
	void UpdateAllFeatures();
	void DrawAllUI();
	void DrawAllBackgroundUI();
	void UpdateAllHotkeys();

	extern bool is_initialized;
	extern std::vector<Feature*> all_features;
};
