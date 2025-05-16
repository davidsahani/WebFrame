#include "webview.hpp"
#include <codecvt>
#include <wil/resource.h>
#include <wil/com.h> // Required for wil::unique_cotaskmem_string
#include <comdef.h>  // For _com_error
#include "Log.hpp"

#define HR_MESSAGE(hr) WebView::GetHResultMessage(hr).c_str()

using namespace Microsoft::WRL;

WebView::WebView(HWND hwnd, std::string env_options) {
    std::wstring options = std::wstring(env_options.begin(), env_options.end());
    InitWebView(hwnd, options);
}

WebView::~WebView() {
    webviewController.Reset();
    webview.Reset();
}

void WebView::InitWebView(HWND hwnd, std::wstring env_options) {
    // Set WebView2 additional arguments via environment variable
    SetEnvironmentVariableW(L"WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS", env_options.c_str());

    CreateCoreWebView2EnvironmentWithOptions(
        nullptr, nullptr, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [hwnd,
             this](HRESULT result, ICoreWebView2Environment *env) -> HRESULT {
                if (FAILED(result)) {
                    Log::Critical("Failed to create WebView2 environment. Error: %s", HR_MESSAGE(result));
                    return result;
                }

                env->CreateCoreWebView2Controller(
                    hwnd,
                    Callback<
                        ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [hwnd, this](
                            HRESULT result, ICoreWebView2Controller *controller
                        ) -> HRESULT {
                            if (FAILED(result)) {
                                Log::Critical("Failed to create WebView2 controller. Error: %s", HR_MESSAGE(result));
                                return result;
                            }

                            webviewController = controller;
                            webviewController->get_CoreWebView2(&webview);

                            if (initializedCallback) {
                                initializedCallback();
                            }

                            webview->add_NavigationStarting(
                                Callback<ICoreWebView2NavigationStartingEventHandler>(
                                    [this](ICoreWebView2 *sender, ICoreWebView2NavigationStartingEventArgs *args) -> HRESULT {
                                        wil::unique_cotaskmem_string uri;
                                        args->get_Uri(&uri);
                                        if (urlCallback) {
                                            std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
                                            urlCallback(myconv.to_bytes(uri.get()));
                                        }
                                        return S_OK;
                                    }
                                ).Get(),
                                nullptr
                            );

                            return S_OK;
                        }
                    ).Get()
                );

                return S_OK;
            }
        ).Get()
    );
}

void WebView::Resize(const RECT &bounds) {
    if (webviewController) {
        webviewController->put_Bounds(bounds);
    }
}

void WebView::GoBack() {
    if (webview) {
        webview->GoBack();
    } else {
        Log::Warning("GoBack Failed. WebView2 not initialized.");
    }
}

void WebView::GoForward() {
    if (webview) {
        webview->GoForward();
    } else {
        Log::Warning("GoForward Failed. WebView2 not initialized.");
    }
}

void WebView::Reload() {
    if (webview) {
        webview->Reload();
    } else {
        Log::Warning("Reload Failed. WebView2 not initialized.");
    }
}

void WebView::Navigate(std::string url) {
    std::wstring uri = std::wstring(url.begin(), url.end());

    if (webview) {
        webview->Navigate(uri.c_str());
    } else {
        Log::Warning("Navigation Failed. WebView2 not initialized.");
    }
}

void WebView::ExecuteScript(std::string script, std::function<void(std::string)> resultCallback) {
    if (!webview) {
        Log::Warning("ExecuteScript Failed. WebView2 not initialized.");
        return;
    }

    std::wstring script_w = std::wstring(script.begin(), script.end());

    const HRESULT hr = webview->ExecuteScript(
        script_w.c_str(),
        Callback<ICoreWebView2ExecuteScriptCompletedHandler>([resultCallback](HRESULT errorCode, LPCWSTR result) -> HRESULT {
            if (SUCCEEDED(errorCode) && result) {
                std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
                std::string resultStr = myconv.to_bytes(result);
                resultCallback(resultStr);
            }
            return S_OK;
        }).Get()
    );

    if (FAILED(hr)) {
        Log::Error("ExecuteScript Failed. Error: %s", HR_MESSAGE(hr));
    }
}

std::string WebView::GetHResultMessage(HRESULT hr) {
    // First, try _com_error
    _com_error err(hr);
    std::string message = err.ErrorMessage();

    // If the message is generic or empty, use FormatMessage
    if (message == "Unknown error" || message.empty()) {
        char *msgBuffer = nullptr;
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msgBuffer, 0, nullptr);

        if (msgBuffer) {
            message = msgBuffer;
            LocalFree(msgBuffer);
        } else {
            message = "Unknown HRESULT error";
        }
    }

    return message;
}