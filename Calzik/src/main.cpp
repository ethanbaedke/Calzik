#include <Windows.h>
#include <iostream>

#include "CZFBXLoader.h"
#include "CZRenderer.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

// Entry point for windows applications.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    // Create console
    AllocConsole();  // Allocate a new console
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);  // Redirect stdout
    freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);  // Redirect stderr
    std::cout << "Console initialized!\n";

    const wchar_t CLASS_NAME[] = L"HelloWorldWindow";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, L"Hello, World!",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 960, 540,
        nullptr, nullptr, hInstance, nullptr);

    if (!hwnd) return 0;

    ShowWindow(hwnd, nShowCmd);

    CZFBXLoader fbxLoader;
    CZRenderer renderer(hwnd);

    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            renderer.Update();
            renderer.Render();
        }
    }

    return static_cast<int>(msg.wParam);
}