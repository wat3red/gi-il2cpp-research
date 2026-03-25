#pragma once
#include "feature.h"

namespace features
{
	class AutoDestroy : public Feature {
	public:
		void DrawUI() override;
		void OnUpdate() override;
		void OnInit() override;
	};
}