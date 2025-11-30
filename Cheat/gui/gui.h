#pragma once
#include <D3D11.h>

namespace GUI {
	void Init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context);
	void Render();
};
