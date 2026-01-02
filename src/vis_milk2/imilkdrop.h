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


// Linux port todo:
// - x64 port
// - DXVK for DX9
// - Implement smallest needed subset of Win32 APIs (MessageBox etc.)
// - Drive WndProc from SDL events if needed
// - Challenge: D3DX9 shader compiler...
//      - Try wined3dx first and see what happens
//      - Try Microsoft d3dx as a test
//      - Best ship solution would probably be to use FXC/D3DX9 in win(e) to precompile every
//        needed preset to the best extent possible and then load those from disk...
//      - Long term solution would be to do whatever ProjectM does to convert
//        HLSL to GLSL and then compile that to SPIR-V and then transform the SPIR-V
//        to DXSO (or backdoor the SPIR-V directly into DXVK)