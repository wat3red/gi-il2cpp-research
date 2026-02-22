#pragma once
#include "feature.h"

namespace features
{
	class MapTeleport : public Feature {
	public:
		void DrawUI() override;
		void OnUpdate();
		void OnInit() override;
		void UpdateHotkeys();
	};
}