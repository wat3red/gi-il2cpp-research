#pragma once
#include "feature.h"

namespace features {
	class MapTeleport : public Feature {
	public:
		void DrawUI() override;
		void DrawBackgroundUI() override;
		void OnInit() override;
		void OnUpdate() override;
	};
}