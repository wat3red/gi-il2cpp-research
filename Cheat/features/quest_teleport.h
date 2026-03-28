#pragma once
#include "feature.h"

namespace features
{
	class QuestTeleport : public Feature {
	public:
		void DrawUI() override;
		void OnUpdate() override;
		void UpdateHotkeys() override;
		void OnInit() override;
	};
}
