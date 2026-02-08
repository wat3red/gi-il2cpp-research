#pragma once
#include "feature.h"

namespace features
{
	class AutoTalk : public Feature {
	public:
		void DrawUI() override;
		void OnInit() override;
	};
}