#pragma once
#include "feature.h"

namespace features
{
	class GameSpeed : public Feature {
	public:
		static void MarkLocalThread();

		void DrawUI() override;
		void OnInit() override;
	};
}