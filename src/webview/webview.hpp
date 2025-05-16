#pragma once
#include <string>
#include <functional>
#include <windows.h>
#include <wrl.h>
#include <webview2.h>

class WebView {
  public:
    WebView(HWND hwnd, std::string env_options);
    ~WebView();
    void GoBack();
    void GoForward();
    void Reload();
    void Navigate(std::string url);
    void ExecuteScript(std::string script, std::function<void(std::string)> resultCallback);

  public:
    void OnInitialized(std::function<void()> cb) { initializedCallback = cb; };
    void OnUrlChange(std::function<void(std::string url)> cb) { urlCallback = cb; };

  public:
    void Resize(const RECT &bounds);

  private:
    void InitWebView(HWND hwnd, std::wstring env_options);
    std::string GetHResultMessage(HRESULT hr);
    std::function<void()> initializedCallback = nullptr;
    std::function<void(std::string url)> urlCallback = nullptr;

  private:
    Microsoft::WRL::ComPtr<ICoreWebView2Controller> webviewController;
    Microsoft::WRL::ComPtr<ICoreWebView2> webview;
};
