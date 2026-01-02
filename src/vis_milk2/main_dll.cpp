
#include <windows.h>
#include "plugin.h"

CPlugin g_plugin;
HINSTANCE api_orig_hinstance = nullptr;
_locale_t g_use_C_locale;
char keyMappings[8];

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    api_orig_hinstance = hModule;
    return true;
}

IMilkDrop* GetMilkDrop()
{
    return &g_plugin;
}
