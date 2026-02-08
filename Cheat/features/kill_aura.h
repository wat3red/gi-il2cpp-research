#pragma once
#include "feature.h"

namespace features
{
	class KillAura : public Feature {
	public:
		void DrawUI() override;
		void OnInit() override;
	};
}