#include "gui.h"
#include "menu.h"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_dx11.h>
#include <imgui/backends/imgui_impl_win32.h>

#include <iostream>

void GUI::Init(HWND window, ID3D11Device* device, ID3D11DeviceContext* context) {
	ImGui::CreateContext();
	ImGui_ImplWin32_EnableDpiAwareness();
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(device, context);

	printf("[+] ImGui bootstrapped succesfully\n");

	ImGui::GetIO().IniFilename = nullptr;
	ImGui::GetIO().MouseDrawCursor = false;
}

void GUI::Render() {
	ImGui::NewFrame();

	if (GetAsyncKeyState(VK_INSERT) & 1) {
		Menu::GetInstance().m_IsOpen = !Menu::GetInstance().m_IsOpen;
	}

	Menu::GetInstance().Draw();

	ImGui::Render();
}