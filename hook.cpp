#include <Windows.h>
#include "mhook/mhook-lib/mhook.h"
#include "proxy.h"
#include "dump.h"
#include <Winsock2.h>
#include <map>

using namespace std;

typedef int WSAAPI (*sig_connect)(SOCKET s, const struct sockaddr *name, int namelen);
typedef int WSAAPI (*sig_recv)(SOCKET s, char *buf, int len, int flags);
typedef int WSAAPI (*sig_send)(SOCKET s, const char *buf, int len, int flags);

sig_connect orig_connect = nullptr;
sig_recv orig_recv = nullptr;
sig_send orig_send = nullptr;

map<SOCKET, u_short> socketPortMap;

int WSAAPI detour_connect(SOCKET s, const struct sockaddr *name, int namelen) {
    socketPortMap[s] = ntohs(((sockaddr_in *) name)->sin_port);
    return orig_connect(s, name, namelen);
}

int WSAAPI detour_recv(SOCKET s, char *buf, int len, int flags) {
    auto result = orig_recv(s, buf, len, flags);
    if (auto portResult = socketPortMap.find(s); portResult != socketPortMap.end()) {
        DoDump(buf, result, portResult->second, 2);
    }
    return result;
}

int WSAAPI detour_send(SOCKET s, const char *buf, int len, int flags) {
    auto result = orig_send(s, buf, len, flags);
    if (auto portResult = socketPortMap.find(s); portResult != socketPortMap.end()) {
        DoDump(buf, result, portResult->second, 1);
    }
    return result;
}

bool doHook() {
    HMODULE hDll = LoadLibrary("ws2_32.dll");
    if (!hDll) {
        return FALSE;
    }
    orig_connect = (sig_connect) GetProcAddress(hDll, "connect");
    if (!orig_connect) {
        return FALSE;
    }
    orig_recv = (sig_recv) GetProcAddress(hDll, "recv");
    if (!orig_recv) {
        return FALSE;
    }
    orig_send = (sig_send) GetProcAddress(hDll, "send");
    if (!orig_send) {
        return FALSE;
    }
    HOOK_INFO hooks[] = {{(PVOID *) &orig_connect, (PVOID) detour_connect},
                         {(PVOID *) &orig_recv,    (PVOID) detour_recv},
                         {(PVOID *) &orig_send,    (PVOID) detour_send}};
    return Mhook_SetHookEx(hooks, 3) == 3;
}

bool WINAPI DllMain(HMODULE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        SourceInit();
        if (doHook()) {
            MessageBox(NULL, "Hooks initialized.", "Success", MB_OK);
        } else {
            MessageBox(NULL, "Something went wrong.", "Error", MB_OK);
        }
    }
    return TRUE;
}