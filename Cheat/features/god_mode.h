#pragma once
#include "feature.h"

namespace features
{
	class GodMode : public Feature {
	public:
		void DrawUI() override;
		void OnInit() override;
	};
}