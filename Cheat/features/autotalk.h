#pragma once
#include "feature.h"

namespace features
{
	class Autotalk : public Feature {
	public:
		void DrawUI() override;
		void DrawBackgroundUI() override;
		void OnInit() override;
		void OnUpdate() override;
	};
}