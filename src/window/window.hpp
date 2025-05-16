#pragma once
#include <string>
#include <tchar.h>
#include <functional>
#include <d3d11.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

struct WindowParams {
    std::string windowName;
    int posX;
    int posY;
    int width;
    int height;
};

class Window {
  public:
    Window(WindowParams winParams);
    ~Window();

    bool Init();
    bool Draw();
    void Render();
    bool IsRunning() { return running; }
    void StopRunning() { running = false; }
    void OnMove(std::function<void()> callback) { moveCallback = callback; }
    void OnResize(std::function<void()> callback) { resizeCallback = callback; }
    void TriggerResizeCallback() {
        if (resizeCallback) resizeCallback();
    }

  public:
    HWND GetWindowHandle() { return hwnd; }
    ID3D11Device *GetDevice() { return d3dDevice; }

  private:
    HWND hwnd;
    WNDCLASSEXW wc;
    bool running = true;
    std::function<void()> moveCallback = nullptr;
    std::function<void()> resizeCallback = nullptr;

  private:
    bool CreateDeviceD3D(HWND hWnd);
    void CleanupDeviceD3D();
    void CreateRenderTarget();
    void CleanupRenderTarget();

  private:
    bool swapChainOccluded = false;
    IDXGISwapChain *swapChain = nullptr;
    ID3D11Device *d3dDevice = nullptr;
    ID3D11DeviceContext *d3dDeviceContext = nullptr;
    ID3D11RenderTargetView *mainRenderTargetView = nullptr;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  private:
    static int posX, posY;
    static UINT resizeWidth, resizeHeight;
    static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};
