#define MINIAUDIO_IMPLEMENTATION
#include <windows.h>
#include <shellapi.h>
#include <d3d11.h>
#include <tchar.h>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <thread>
#include <algorithm>
#include <dwmapi.h>
#include "../resource.h"
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001
#define ID_TRAY_SHOW 1002
#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif
#define DWMSBT_MAINHOSTBACKDROP 2
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "KeyCapture.hpp"
#include "Play.hpp"

static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

NOTIFYICONDATAW g_NotifyIconData = {};
bool g_WindowVisible = true; // almost done writing of this prog i like the sound 

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

std::vector<std::string> get_soundpacks() {
    std::vector<std::string> packs;
    std::filesystem::path res_path = std::filesystem::current_path() / "resources";
    if (std::filesystem::exists(res_path)) {
        for (const auto& entry : std::filesystem::directory_iterator(res_path)) {
            if (entry.is_directory()) packs.push_back(entry.path().filename().string());
        }
    }
    if (packs.empty()) packs.push_back("cherrymx-black-pbt");
    std::sort(packs.begin(), packs.end());
    return packs;
}

void save_settings(int selected, int volume) {
    nlohmann::json j;
    j["selected_index"] = selected;
    j["volume"] = volume;
    std::ofstream f("settings.json");
    if (f.is_open()) f << j.dump(4);
}

void load_settings(int& selected, int& volume, int max_index) {
    selected = 0;
    volume = 50;

    std::ifstream f("settings.json");
    if (f.is_open()) {
        try {
            nlohmann::json j;
            f >> j;
            selected = j.value("selected_index", 0);
            volume = j.value("volume", 50);
            if (max_index == 0) selected = 0;
            else if (selected >= max_index) selected = 0;
        }
        catch (const std::exception& e) {
            selected = 0;
            volume = 50;
        }
    }
}

bool IsSystemDarkMode() {
    HKEY hKey;
    DWORD dwValue = 1;
    DWORD dwSize = sizeof(dwValue);
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExW(hKey, L"AppsUseLightTheme", NULL, NULL, (LPBYTE)&dwValue, &dwSize);
        RegCloseKey(hKey);
    }
    return dwValue == 0;
}



void SetAutoStart(bool enable) {
    HKEY hKey;
    const wchar_t* czStartName = L"CyEcho";
    wchar_t szPathToExe[MAX_PATH];
    GetModuleFileNameW(NULL, szPathToExe, MAX_PATH);

    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        if (enable) {
            RegSetValueExW(hKey, czStartName, 0, REG_SZ, (BYTE*)szPathToExe, (wcslen(szPathToExe) + 1) * sizeof(wchar_t));
        }
        else {
            RegDeleteValueW(hKey, czStartName);
        }
        RegCloseKey(hKey);
    }
}

bool IsAutoStartEnabled() {
    HKEY hKey;
    bool enabled = false;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExW(hKey, L"CyEcho", NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
            enabled = true;
        }
        RegCloseKey(hKey);
    }
    return enabled;
}

void ApplyModernTheme() {
    auto& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.ChildRounding = 6.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding = 6.0f;
    style.TabRounding = 6.0f;
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.70f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
}

// main window
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPWSTR lpCmdLine, int nShowCmd) {
    wchar_t szExePath[MAX_PATH];
    GetModuleFileNameW(NULL, szExePath, MAX_PATH);
    PathRemoveFileSpecW(szExePath);
    SetCurrentDirectoryW(szExePath);
    std::vector<std::string> sounds = get_soundpacks();
    if (sounds.empty()) {
        MessageBoxW(NULL, L"No sound packs found in 'resources' folder!", L"Error", MB_ICONERROR);
        return 1;
    }

    int selected = 0;
    int volume = 50;
    load_settings(selected, volume, (int)sounds.size());

    std::string initialPath = "resources/" + sounds[selected];
    KeyCapture::global_player = std::make_unique<Play::Player>(initialPath);
    KeyCapture::config.volume.store((float)volume);

    hook_set_dispatch_proc(&KeyCapture::dispatch_proc);
    std::thread hook_thread([]() {
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
        hook_run();
        });

    const wchar_t* className = L"CyEchoGUI";
    const wchar_t* windowTitle = L"CyEcho";

    WNDCLASSEXW wc = { sizeof(wc) };
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wc.lpszClassName = className;

    if (!::RegisterClassExW(&wc)) return 1;
    DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    HWND hwnd = ::CreateWindowW(className, windowTitle, dwStyle, 100, 100, 450, 350, NULL, NULL, hInstance, NULL);

    BOOL useDarkMode = IsSystemDarkMode();
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(useDarkMode));
    int backdropType = DWMSBT_MAINHOSTBACKDROP;
    DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdropType, sizeof(backdropType));
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)wc.hIcon);
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)wc.hIconSm);

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClassW(className, hInstance);
        return 1;
    }

    // System Tray Setup
    g_NotifyIconData.cbSize = sizeof(g_NotifyIconData);
    g_NotifyIconData.hWnd = hwnd;
    g_NotifyIconData.uID = 1;
    g_NotifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_NotifyIconData.uCallbackMessage = WM_TRAYICON;
    g_NotifyIconData.hIcon = wc.hIconSm;
    wcscpy_s(g_NotifyIconData.szTip, L"CyEcho - Keyboard Sounds");
    Shell_NotifyIconW(NIM_ADD, &g_NotifyIconData);

    ::ShowWindow(hwnd, nShowCmd);
    ::UpdateWindow(hwnd);

    // ImGui Setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ApplyModernTheme();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
    const float clear_color[4] = { 0.00f, 0.00f, 0.00f, 0.00f };
    bool done = false;
    while (!done) {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT) done = true;
        }
        if (done) break;

        if (g_WindowVisible) {
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(io.DisplaySize);
            ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

            ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "CyEcho Status: Active");
            ImGui::Separator();

            if (ImGui::SliderInt("Master Volume", &volume, 0, 100)) {
                KeyCapture::config.volume.store((float)volume);
                save_settings(selected, volume);
            }

            ImGui::Spacing();
            ImGui::Text("Active Sound Profile:");
            if (ImGui::BeginListBox("##Profiles", ImVec2(-FLT_MIN, -80))) {
                for (int n = 0; n < (int)sounds.size(); n++) {
                    const bool is_selected = (selected == n);
                    if (ImGui::Selectable(sounds[n].c_str(), is_selected)) {
                        selected = n;
                        KeyCapture::global_player->loadProfile("resources/" + sounds[selected]);
                        save_settings(selected, volume);
                    }
                }
                ImGui::EndListBox();
            }
            static bool autoStart = IsAutoStartEnabled();
            if (ImGui::Checkbox("Launch on Startup", &autoStart)) {
                SetAutoStart(autoStart);
            }
            if (ImGui::Button("Hide to Tray", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0))) {
                g_WindowVisible = false;
                ::ShowWindow(hwnd, SW_HIDE);
            }
            ImGui::SameLine();
            if (ImGui::Button("Exit App", ImVec2(-FLT_MIN, 0))) {
                done = true;
            }

            ImGui::End();

            // Rendering
            ImGui::Render();
            const float clear_color[4] = { 0.05f, 0.05f, 0.05f, 1.00f };
            g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
            g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            g_pSwapChain->Present(1, 0);

            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
    }
    Shell_NotifyIconW(NIM_DELETE, &g_NotifyIconData);
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    hook_stop();
    if (hook_thread.joinable()) hook_thread.join();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(className, hInstance);

    return 0;
}

bool CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT flags = 0;
    D3D_FEATURE_LEVEL fl;
    const D3D_FEATURE_LEVEL fls[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, fls, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &fl, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget() { if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; } }
void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_pSwapChain) g_pSwapChain->Release();
    if (g_pd3dDeviceContext) g_pd3dDeviceContext->Release();
    if (g_pd3dDevice) g_pd3dDevice->Release();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;
    switch (msg) {
    case WM_TRAYICON:
        if (LOWORD(lParam) == WM_LBUTTONDBLCLK) {
            g_WindowVisible = true;
            ::ShowWindow(hWnd, SW_SHOW);
            ::SetForegroundWindow(hWnd);
        }
        else if (LOWORD(lParam) == WM_RBUTTONUP) {
            POINT curPoint;
            GetCursorPos(&curPoint);
            HMENU hMenu = CreatePopupMenu();
            InsertMenuW(hMenu, 0, MF_BYPOSITION | MF_STRING, ID_TRAY_SHOW, L"Open Settings");
            InsertMenuW(hMenu, 1, MF_BYPOSITION | MF_STRING, ID_TRAY_EXIT, L"Exit CyEcho");
            SetForegroundWindow(hWnd);
            int clicked = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, curPoint.x, curPoint.y, 0, hWnd, NULL);
            if (clicked == ID_TRAY_EXIT) PostQuitMessage(0);
            if (clicked == ID_TRAY_SHOW) { g_WindowVisible = true; ShowWindow(hWnd, SW_SHOW); }
            DestroyMenu(hMenu);
        }
        break;
    case WM_CLOSE:
        g_WindowVisible = false;
        ::ShowWindow(hWnd, SW_HIDE);
        return 0;
    case WM_SETTINGCHANGE: {
        if (lParam && wcscmp((LPCWSTR)lParam, L"ImmersiveColorSet") == 0) {
            BOOL useDarkMode = TRUE;
            DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(useDarkMode));
        }
        return 0;
    }
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED) {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND: if ((wParam & 0xFFF0) == SC_KEYMENU) return 0; break;
    case WM_DESTROY: ::PostQuitMessage(0); return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}