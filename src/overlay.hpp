#pragma once
#include <windows.h>
#include <d3d11.h>

#include <functional>
#include <string>

namespace adrenaline {

// Transparent, topmost, click-through Direct3D 11 overlay that ImGui draws onto.
class Overlay {
public:
    using Callback = std::function<void()>;

    bool create(const std::wstring& title, const std::wstring& className);
    // preFrame runs before ImGui::NewFrame (window tracking, memory reads).
    // render runs between NewFrame and Render (drawing).
    void run(const Callback& preFrame, const Callback& render);
    void destroy();

    HWND hwnd() const { return hwnd_; }
    void setClickThrough(bool clickThrough);

private:
    static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    bool createDevice();
    void destroyDevice();
    bool createRenderTarget();
    void destroyRenderTarget();

    HWND hwnd_ = nullptr;
    ID3D11Device* device_ = nullptr;
    ID3D11DeviceContext* context_ = nullptr;
    IDXGISwapChain* swapChain_ = nullptr;
    ID3D11RenderTargetView* renderTarget_ = nullptr;
    bool clickThrough_ = true;
    bool running_ = false;
};

} // namespace adrenaline
