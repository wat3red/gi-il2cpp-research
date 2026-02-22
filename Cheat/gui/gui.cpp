#include "gui.h"
#include "menu.h"
#include <game_api/unity/unity_objects.h>

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
	static int  prevCursorLockState = 0;
	static bool prevCursorVisible = false;
	static bool cursorStateSaved = false;

	ImGui::NewFrame();

	if (GetAsyncKeyState(VK_INSERT) & 1)
	{
		auto& menu = Menu::GetInstance();
		menu.m_IsOpen = !menu.m_IsOpen;

		if (menu.m_IsOpen)
		{
			// Save current state only once
			if (!cursorStateSaved)
			{
				prevCursorLockState = Unity::Cursor::GetLockState();
				prevCursorVisible = Unity::Cursor::GetVisible();
				cursorStateSaved = true;
			}

			Unity::Cursor::SetLockState(0); // None
			Unity::Cursor::SetVisible(true);
		}
		else
		{
			if (cursorStateSaved)
			{
				Unity::Cursor::SetLockState(prevCursorLockState);
				Unity::Cursor::SetVisible(prevCursorVisible);
				cursorStateSaved = false;
			}
		}
	}

	Menu::GetInstance().Draw();
	ImGui::Render();
}