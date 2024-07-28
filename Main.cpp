#include <iostream>
#include <windows.h>
#include <thread>
#include <vector>
#include <unordered_map>
#include <Psapi.h>
#include <tchar.h>
#include <d3d9.h>
#include <chrono>
#include <ctime>
#pragma comment(lib, "d3d9.lib")

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

#include "oauth2.h"
#include "google_calendar.h"

// Global
bool showEventConfigWindow = false;
size_t selectedLogIndex = 0;
char eventTitle[128] = "";
char eventDescription[256] = "";
char eventColor[64] = "";

std::vector<std::string> logs;
void AddLog(const std::string& message) {
    logs.push_back(message);
}

HWND main_hwnd = nullptr;
LPDIRECT3DDEVICE9 g_pd3dDevice = nullptr;
D3DPRESENT_PARAMETERS g_d3dpp;
LPDIRECT3D9 g_pD3D = nullptr;

std::string clientId = "";
std::string clientSecret = "";
std::string redirectUri = "";
OAuth2 oauth2(clientId, clientSecret, redirectUri);
std::string accessToken = "";
bool isAuthenticated = false;

std::vector<std::string> monitoredPrograms = { "notepad.exe", "devenv.exe", "chrome.exe" };
std::unordered_map<std::string, int> programTimes;
std::string activeWindow = "";
std::string mainProgram = "";

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// DirectX shit
bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
        return false;

    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;
    return true;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

// Get Program names
std::string GetActiveWindowTitle()
{
    char title[256];
    HWND hwnd = GetForegroundWindow();
    if (hwnd != NULL) {
        GetWindowText(hwnd, title, sizeof(title));
        return std::string(title);
    }
    return "";
}

std::string GetActiveWindowProcessName()
{
    HWND hwnd = GetForegroundWindow();
    if (hwnd == NULL) return "";

    DWORD processId;
    GetWindowThreadProcessId(hwnd, &processId);

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess == NULL) return "";

    char processName[MAX_PATH] = "<unknown>";
    if (GetModuleBaseName(hProcess, NULL, processName, sizeof(processName) / sizeof(char))) {
        CloseHandle(hProcess);
        return std::string(processName);
    }
    CloseHandle(hProcess);
    return "";
}

bool IsMonitoredProgram(const std::string& processName)
{
    return std::find(monitoredPrograms.begin(), monitoredPrograms.end(), processName) != monitoredPrograms.end();
}

void MonitorActiveWindow()
{
    while (true) {
        std::string processName = GetActiveWindowProcessName();
        if (IsMonitoredProgram(processName)) {
            programTimes[processName]++;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// Clipboard
void SetClipboardText(const std::string& text)
{
    if (OpenClipboard(NULL))
    {
        EmptyClipboard();
        HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
        if (hGlobal)
        {
            memcpy(GlobalLock(hGlobal), text.c_str(), text.size() + 1);
            GlobalUnlock(hGlobal);
            SetClipboardData(CF_TEXT, hGlobal);
            GlobalFree(hGlobal);
        }
        CloseClipboard();
    }
}

void CreateGoogleCalendarEvent(const std::string& calendarId, const std::string& summary, const std::string& description,
    const std::string& startTime, const std::string& endTime)
{
    if (!accessToken.empty())
    {
        GoogleCalendar calendar(accessToken);
        calendar.createEvent(calendarId, summary, description, startTime, endTime);
    }
    else
    {
        AddLog("Error: No access token available. Please authenticate first.");
    }
}

// ImGui-Window for Google Auth
void ShowOAuthWindow()
{
    ImGui::Begin("OAuth2 Authorization");

    if (!isAuthenticated)
    {
        std::string authUrl = oauth2.getAuthorizationUrl();
        ImGui::TextWrapped("Gehe zu diesem URL und autorisiere die Anwendung: %s", authUrl.c_str());

        if (ImGui::Button("Copy URL"))
        {
            SetClipboardText(authUrl);
            AddLog("Authorization URL copied to clipboard.");
        }

        static char authCode[128] = "";
        ImGui::InputText("Gib den Autorisierungscode ein", authCode, IM_ARRAYSIZE(authCode));

        if (ImGui::Button("Authorize"))
        {
            accessToken = oauth2.getAccessToken(authCode);
            if (!accessToken.empty())
            {
                isAuthenticated = true;
                AddLog("Authorization successful. Access token received.");
            }
            else
            {
                AddLog("Authorization failed. No access token received.");
            }
        }
    }
    else
    {
        ImGui::Text("Erfolgreich authentifiziert!");
    }

    ImGui::End();
}

// ImGui-Window for Management
void ShowProgramManagerWindow()
{
    ImGui::Begin("Programme Hinzufuegen");

    ImGui::Text("Ueberwachte Programme (Drag & Drop moeglich):");
    for (const auto& program : monitoredPrograms) {
        bool isMainProgram = (mainProgram == program);
        if (ImGui::Selectable(program.c_str(), &isMainProgram)) {
            mainProgram = isMainProgram ? program : "";
        }
    }

    static char newProgram[128] = "";
    ImGui::InputText("Add Program (exe)", newProgram, IM_ARRAYSIZE(newProgram));
    if (ImGui::Button("Add")) {
        if (std::find(monitoredPrograms.begin(), monitoredPrograms.end(), newProgram) == monitoredPrograms.end()) {
            monitoredPrograms.push_back(newProgram);
            programTimes[newProgram] = 0; // Initialisieren Sie die Zeit für das neue Programm
            AddLog("Program added: " + std::string(newProgram));
        }
        memset(newProgram, 0, sizeof(newProgram));
    }

    if (ImGui::Button("Remove") && !mainProgram.empty()) {
        monitoredPrograms.erase(std::remove(monitoredPrograms.begin(), monitoredPrograms.end(), mainProgram), monitoredPrograms.end());
        programTimes.erase(mainProgram); // Entfernen Sie auch die Zeit für das Programm
        AddLog("Program removed: " + mainProgram);
        mainProgram.clear();
    }

    ImGui::End();
}

// ImGui-Widnow for Live-Tracking
void ShowLiveTrackingWindow()
{
    ImGui::Begin("Aktives Programm");
    activeWindow = GetActiveWindowProcessName();
    ImGui::Text("Aktuelles aktives Programm: %s", activeWindow.c_str());

    ImGui::Text("Log:");
    ImGui::BeginChild("Log", ImVec2(0, 100), true, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& entry : programTimes) {
        ImGui::Text("%s: %d Sekunden", entry.first.c_str(), entry.second);
    }
    ImGui::EndChild();
    ImGui::End();
}

void ShowConsoleWindow() {
    ImGui::Begin("Developer Console");
    for (size_t i = 0; i < logs.size(); ++i) {
        ImGui::TextWrapped("%s", logs[i].c_str());
        ImGui::SameLine();
        if (ImGui::Button(("Push##" + std::to_string(i)).c_str())) {
            showEventConfigWindow = true;
            selectedLogIndex = i;
        }
    }
    ImGui::End();
}

void ShowEventConfigWindow() {
    if (showEventConfigWindow) {
        ImGui::Begin("Event Configuration", &showEventConfigWindow);

        ImGui::InputText("Title", eventTitle, IM_ARRAYSIZE(eventTitle));
        ImGui::InputText("Description", eventDescription, IM_ARRAYSIZE(eventDescription));
        ImGui::InputText("Color", eventColor, IM_ARRAYSIZE(eventColor));

        // Default start time is now minus the logged seconds
        auto endTime = std::chrono::system_clock::now();
        auto startTime = endTime - std::chrono::seconds(programTimes[monitoredPrograms[selectedLogIndex]]);

        // Display start and end time
        std::time_t startTime_t = std::chrono::system_clock::to_time_t(startTime);
        std::time_t endTime_t = std::chrono::system_clock::to_time_t(endTime);
        char startTimeStr[128];
        char endTimeStr[128];

        // Use gmtime_s instead of gmtime
        std::tm startTm;
        std::tm endTm;
        gmtime_s(&startTm, &startTime_t);
        gmtime_s(&endTm, &endTime_t);
        std::strftime(startTimeStr, sizeof(startTimeStr), "%Y-%m-%dT%H:%M:%SZ", &startTm);
        std::strftime(endTimeStr, sizeof(endTimeStr), "%Y-%m-%dT%H:%M:%SZ", &endTm);

        ImGui::Text("Start Time: %s", startTimeStr);
        ImGui::Text("End Time: %s", endTimeStr);

        if (ImGui::Button("Create Event")) {
            CreateGoogleCalendarEvent("primary", eventTitle, eventDescription, startTimeStr, endTimeStr);
            showEventConfigWindow = false;
        }

        ImGui::End();
    }
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("ImGui Example"), NULL };
    RegisterClassEx(&wc);
    HWND hwnd = CreateWindow(wc.lpszClassName, _T("ImGui DirectX9 Example"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    std::thread monitorThread(MonitorActiveWindow);
    monitorThread.detach();

    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ShowOAuthWindow();
        ShowProgramManagerWindow();
        ShowLiveTrackingWindow();
        ShowConsoleWindow();
        ShowEventConfigWindow();

        ImGui::Render();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(255.0f * 0.45f), (int)(255.0f * 0.55f), (int)(255.0f * 0.60f), (int)(255.0f * 1.00f));
        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }

        HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
        {
            ResetDevice();
        }
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}
