// hooks.cpp
#include "hooks.h"

#pragma comment(lib, "ws2_32.lib")

namespace Hooks {
    // Function defs
    typedef int (WINAPI* send_t)(SOCKET, const char*, int, int);
    typedef int (WINAPI* WSASend_t)(SOCKET, LPWSABUF, DWORD, LPDWORD, DWORD, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE);
    typedef int (WINAPI* connect_t)(SOCKET, const sockaddr*, int);

    // Originals
    send_t o_send = nullptr;
    WSASend_t o_WSASend = nullptr;
    connect_t o_connect = nullptr;

    // Detours
    int WINAPI h_send(SOCKET s, const char* buf, int len, int flags) {
        if (Config::BlockPackets) return len;
        return o_send(s, buf, len, flags);
    }

    int WINAPI h_WSASend(SOCKET s, LPWSABUF buffers, DWORD bufferCount, LPDWORD bytesSent, DWORD flags, LPWSAOVERLAPPED ov, LPWSAOVERLAPPED_COMPLETION_ROUTINE cr) {
        if (Config::BlockPackets) {
            if (bytesSent) *bytesSent = buffers->len;
            return 0;
        }
        return o_WSASend(s, buffers, bufferCount, bytesSent, flags, ov, cr);
    }

    int WINAPI h_connect(SOCKET s, const sockaddr* name, int namelen) {
        if (Config::BlockPackets) {
            WSASetLastError(WSAECONNREFUSED);
            return SOCKET_ERROR;
        }
        return o_connect(s, name, namelen);
    }

    bool Init() {
        if (MH_Initialize() != MH_OK) {
            printf("MinHook init failed!\n");
            return false;
        }

        //MH_CreateHookApi(L"ws2_32", "send", (LPVOID)h_send, (LPVOID*)&o_send);
        //MH_CreateHookApi(L"ws2_32", "WSASend", (LPVOID)h_WSASend, (LPVOID*)&o_WSASend);
        //MH_CreateHookApi(L"ws2_32", "connect", (LPVOID)h_connect, (LPVOID*)&o_connect);

        return MH_EnableHook(MH_ALL_HOOKS) == MH_OK;
    }

    void Uninit() {
        MH_Uninitialize();
    }
}