#pragma once
#include "feature.h"

namespace features
{
	class AutoLoot : public Feature {
	public:
		void DrawUI() override;
		void OnUpdate() override;
		void OnInit() override;
	};
}