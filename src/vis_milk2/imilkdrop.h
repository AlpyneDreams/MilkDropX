#pragma once

// YUCK!
#include <windows.h>

struct IDirect3DDevice9;

struct IMilkDrop
{
    virtual int PluginPreInitialize(HWND hWinampWnd, HINSTANCE hWinampInstance) = 0;
    virtual int PluginInitialize(IDirect3DDevice9* device, struct _D3DPRESENT_PARAMETERS_* d3dpp, HWND hwnd, int iWidth, int iHeight) = 0;
    virtual void PluginQuit() = 0;

    // Render!
    virtual int PluginRender(unsigned char* pWaveL, unsigned char* pWaveR) = 0;

    // Handle input and window events
    virtual LRESULT PluginShellWindowProc(HWND hWnd, unsigned uMsg, WPARAM wParam, LPARAM lParam) = 0;

    // Save config
    virtual void MyWriteConfig() = 0;
}; 
