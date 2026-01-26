#include "directx_hook.h"
#include "gui/gui.h"
#include "gui/menu.h"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_dx11.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <minhook/include/MinHook.h>

#include <psapi.h>
#include <iostream>
#include <vector>
#include <thread>

#define STB_IMAGE_IMPLEMENTATION

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK hook_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	ImGui::GetIO().MouseDrawCursor = Menu::GetInstance().m_IsOpen;
	ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

	if (uMsg == WM_PAINT) {
		HDC         hdc;
		PAINTSTRUCT ps;
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		return S_OK;
	}

	if (Menu::GetInstance().m_IsOpen) {
		return true;
	}



	return CallWindowProc(OriginalWndProcHandler, hWnd, uMsg, wParam, lParam);
}

HRESULT GetDeviceAndCtxFromSwapchain(IDXGISwapChain* pSwapChain, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext) {
	HRESULT ret = pSwapChain->GetDevice(__uuidof(ID3D11Device), (PVOID*)ppDevice);

	if (SUCCEEDED(ret))
		(*ppDevice)->GetImmediateContext(ppContext);

	return ret;
}

ResizeBuffers o_bufs;
static IDXGISwapChain* gswc = 0;
HRESULT hook_ResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
	//printf("resized@\n");
	if (!MainRenderTargetView)
		return o_bufs(pSwapChain, BufferCount, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM, SwapChainFlags);

	DXGI_SWAP_CHAIN_DESC Desc;
	pSwapChain->GetDesc(&Desc);

	//printf("obufs: %p\n", o_bufs);
	if (MainRenderTargetView && pContext) {
		pContext->OMSetRenderTargets(0, 0, 0);
		MainRenderTargetView->Release();
	}
	HRESULT hr = o_bufs(pSwapChain, BufferCount, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM, SwapChainFlags);
	//printf("hr: %X, %lu, %lu, %lu, %i, %lu\n", hr, BufferCount, Width, Height, NewFormat, SwapChainFlags);
	ID3D11Texture2D* pBuffer;
	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBuffer);
	if (pBuffer == nullptr) return hr;

	pDevice->CreateRenderTargetView(pBuffer, NULL, &MainRenderTargetView);
	pBuffer->Release();
	pContext->OMSetRenderTargets(1, &MainRenderTargetView, NULL);

	// Set up the viewport.
	D3D11_VIEWPORT vp;
	vp.Width = Width;
	vp.Height = Height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pContext->RSSetViewports(1, &vp);

	return hr;
}

IDXGISwapChainPresent present;

static BOOL g_bInitialised = false;
static bool g_SwapChainOccluded = false;
static DWORD_PTR* g_origResize = 0;
HRESULT __fastcall hook_Present(IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags) {
	if (!g_bInitialised) {
		if (SUCCEEDED(pChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice))) {
			ImGui::CreateContext();

			DXGI_SWAP_CHAIN_DESC desc0 = {};
			pChain->GetDesc(&desc0);
			window = desc0.OutputWindow;

			if (!pDevice) return present(pChain, SyncInterval, Flags);
			pDevice->GetImmediateContext(&pContext);

			ID3D11Texture2D* BackBuffer;
			pChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&BackBuffer);
			if (!BackBuffer) return present(pChain, SyncInterval, Flags);

			D3D11_RENDER_TARGET_VIEW_DESC desc = {};
			memset(&desc, 0, sizeof(desc));
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // most important change!
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			pDevice->CreateRenderTargetView(BackBuffer, &desc, &MainRenderTargetView);
			BackBuffer->Release();

			DWORD old = 0;
			VirtualProtect((LPVOID)g_origResize, 100, PAGE_READWRITE, &old);
			*g_origResize = (uintptr_t)hook_ResizeBuffers;
			VirtualProtect((LPVOID)g_origResize, 100, old, &old);

			GUI::Init(window, pDevice, pContext);
			ImGui_ImplDX11_CreateDeviceObjects();

			OriginalWndProcHandler = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (__int3264)hook_WndProc);
			g_bInitialised = true;
		}
	}

	if (!g_SwapChainOccluded && MainRenderTargetView)
	{
		ImGui_ImplWin32_NewFrame();
		ImGui_ImplDX11_NewFrame();
		GUI::Render();
		pContext->OMSetRenderTargets(1, &MainRenderTargetView, nullptr);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	HRESULT hr = present(pChain, SyncInterval, Flags);
	g_SwapChainOccluded = hr == DXGI_STATUS_OCCLUDED;

	return hr;
}

namespace dx_hook {
	void HookPresent() {
		printf(("[...] Hooking Present...\n"));
		std::thread(findDirect11Present).detach();
	}

	typedef long(*PresentImplCore)(IDXGISwapChain* this_, void* a2, uint32_t a3, void* a4, uint32_t a5, void* a6, void* a7, void* a8);
	PresentImplCore o_PresentImplCore;

	long hk_PresentImplCore(IDXGISwapChain* this_, void* a2, uint32_t a3, void* a4, uint32_t a5, void* a6, void* a7, void* a8)
	{
		if (!gswc && this_) {
			gswc = this_;
		}

		return o_PresentImplCore(this_, a2, a3, a4, a5, a6, a7, a8);
	}

	static uintptr_t foundPIC = 0;

	uintptr_t pattern_scan(uintptr_t module_base, const char* signature) {
		static auto pattern_to_byte = [](const char* pattern) {
			auto bytes = std::vector<int>{};
			auto start = const_cast<char*>(pattern);
			auto end = const_cast<char*>(pattern) + strlen(pattern);
			for (auto current = start; current < end; ++current) {
				if (*current == '?') {
					++current;
					if (*current == '?')
						++current;
					bytes.push_back(-1);
				}
				else {
					bytes.push_back(strtoul(current, &current, 16));
				}
			}
			return bytes;
			};

		auto dosHeader = (PIMAGE_DOS_HEADER)module_base;
		auto ntHeaders = (PIMAGE_NT_HEADERS)((uint8_t*)module_base + dosHeader->e_lfanew);

		auto size_of_image = ntHeaders->OptionalHeader.SizeOfImage;
		auto pattern_bytes = pattern_to_byte(signature);
		auto module_bytes = reinterpret_cast<uint8_t*>(module_base);

		auto s = pattern_bytes.size();
		auto d = pattern_bytes.data();
		auto CurrentSig = 0;
		int TimesFound = 0;
		for (auto i = 0ul; i < size_of_image - s; ++i) {
			bool found = true;
			for (auto j = 0ul; j < s; ++j) {
				if (module_bytes[i + j] != d[j] && d[j] != -1) {
					found = false; // if not found keep scanning
					break;
				}
			}
			if (found) {
				return (uintptr_t)&module_bytes[i];
			}
		}
		return 0;
	}

	static IDXGISwapChainPresent findDirect11Present()
	{
		while (!GetModuleHandleA(("dxgi.dll")))
		{
			Sleep(100);
		}

		printf(("Trying pattern #1...\n"));
		foundPIC = pattern_scan((uintptr_t)GetModuleHandleA(("dxgi.dll")), ("40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? F8 FF FF 48 81 EC"));
		if (foundPIC == 0)
		{
			printf(("Trying pattern #2...\n"));
			foundPIC = pattern_scan((uintptr_t)GetModuleHandleA(("dxgi.dll")), ("48 8B C4 55 53 56 57 41 54 41 55 41 56 41 57 48 8D A8 58 F7 FF FF 48 81"));
			if (foundPIC == 0)
			{
				MessageBoxA(FindWindowA("UnityWndClass", 0), ("Couldn't find PIC!\nCreate a ticket in our discord server"), ("DXGI ERROR"), (MB_TOPMOST));
			}
		}

		o_PresentImplCore = (long(*)(IDXGISwapChain * this_, void* a2, uint32_t a3, void* a4, uint32_t a5, void* a6, void* a7, void* a8))(foundPIC);

		MH_Initialize();
		MH_CreateHook((LPVOID)foundPIC, hk_PresentImplCore, (LPVOID*)&o_PresentImplCore);
		MH_EnableHook((LPVOID)foundPIC);

		printf(("Hooked present at 0x%X\n"), foundPIC);

		while (!gswc) {
			Sleep(100);
		}

		MH_DisableHook((LPVOID)foundPIC);

		const DWORD_PTR* pSwapChainVtable = reinterpret_cast<DWORD_PTR*>(gswc);
		pSwapChainVtable = reinterpret_cast<DWORD_PTR*>(pSwapChainVtable[0]);

		IDXGISwapChainPresent orig = (IDXGISwapChainPresent)pSwapChainVtable[8];
		ResizeBuffers orig_resize = (ResizeBuffers)pSwapChainVtable[13];

		present = orig;
		o_bufs = orig_resize;

		auto targ = (DWORD_PTR*)(pSwapChainVtable + 8);
		auto targ_resize = (DWORD_PTR*)(pSwapChainVtable + 13);

		DWORD old = 0;
		VirtualProtect((LPVOID)targ, 100, PAGE_READWRITE, &old);
		*targ = (uintptr_t)hook_Present;
		VirtualProtect((LPVOID)targ, 100, old, &old);
		g_origResize = targ_resize;
		gswc->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice);
		gswc->Release();

		return orig;
	}
}
