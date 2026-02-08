#pragma once
#include "feature.h"

namespace features
{
	class Noclip : public Feature {
	public:
		void DrawUI() override;
		void OnUpdate() override;
		void UpdateHotkeys() override;
	};
}