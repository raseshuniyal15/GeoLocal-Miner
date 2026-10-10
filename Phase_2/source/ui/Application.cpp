#include "Application.h"
#include "../../third_party/imgui/imgui.h"
#include "../../third_party/imgui/backends/imgui_impl_win32.h"
#include "../../third_party/imgui/backends/imgui_impl_opengl2.h"
#include "../data/DataLoader.h"
#include <GL/gl.h>
#include <iostream>

// Forward declare ImGui message handler
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace GeoLocalMiner {

Application::Application()
    : hWnd(nullptr), hDC(nullptr), hRC(nullptr),
      windowWidth(1440), windowHeight(900), isRunning(false) {}

Application::~Application() {
    cleanup();
}

LRESULT WINAPI Application::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED) {
                // Resize handled in render loop via GetClientRect
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                return 0;
            break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
    }
    return ::DefWindowProcA(hWnd, msg, wParam, lParam);
}

bool Application::initWindow() {
    WNDCLASSEXA wc = { sizeof(WNDCLASSEXA), CS_CLASSDC, Application::WndProc, 0L, 0L, 
                       ::GetModuleHandleA(nullptr), nullptr, nullptr, nullptr, nullptr, 
                       "GeoLocalMinerWindowClass", nullptr };
    ::RegisterClassExA(&wc);

    hWnd = ::CreateWindowExA(
        0L,
        wc.lpszClassName,
        windowTitle.c_str(),
        WS_OVERLAPPEDWINDOW,
        100, 100, windowWidth, windowHeight,
        nullptr, nullptr, wc.hInstance, nullptr
    );

    if (!hWnd) {
        std::cerr << "Failed to create Win32 window." << std::endl;
        return false;
    }

    return true;
}

bool Application::initOpenGL() {
    hDC = ::GetDC(hWnd);

    PIXELFORMATDESCRIPTOR pfd;
    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int format = ::ChoosePixelFormat(hDC, &pfd);
    if (!format || !::SetPixelFormat(hDC, format, &pfd)) {
        std::cerr << "Failed to set OpenGL pixel format." << std::endl;
        return false;
    }

    hRC = ::wglCreateContext(hDC);
    if (!hRC || !::wglMakeCurrent(hDC, hRC)) {
        std::cerr << "Failed to initialize OpenGL rendering context." << std::endl;
        return false;
    }

    return true;
}

void Application::setupTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Professional Dark Geoscience Theme
    colors[ImGuiCol_Text]                  = ImVec4(0.92f, 0.94f, 0.96f, 1.00f);
    colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.55f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]              = ImVec4(0.10f, 0.12f, 0.16f, 1.00f);
    colors[ImGuiCol_ChildBg]               = ImVec4(0.12f, 0.14f, 0.18f, 1.00f);
    colors[ImGuiCol_PopupBg]               = ImVec4(0.14f, 0.16f, 0.22f, 1.00f);
    colors[ImGuiCol_Border]                = ImVec4(0.22f, 0.26f, 0.32f, 1.00f);
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]               = ImVec4(0.16f, 0.19f, 0.24f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.22f, 0.26f, 0.34f, 1.00f);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(0.26f, 0.32f, 0.42f, 1.00f);
    colors[ImGuiCol_TitleBg]               = ImVec4(0.08f, 0.10f, 0.14f, 1.00f);
    colors[ImGuiCol_TitleBgActive]         = ImVec4(0.12f, 0.15f, 0.20f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.08f, 0.10f, 0.14f, 0.75f);
    colors[ImGuiCol_MenuBarBg]             = ImVec4(0.12f, 0.14f, 0.18f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.10f, 0.12f, 0.16f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.25f, 0.30f, 0.38f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.35f, 0.42f, 0.52f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.45f, 0.55f, 0.68f, 1.00f);
    colors[ImGuiCol_CheckMark]             = ImVec4(0.26f, 0.75f, 0.95f, 1.00f);
    colors[ImGuiCol_SliderGrab]            = ImVec4(0.26f, 0.75f, 0.95f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.40f, 0.85f, 1.00f, 1.00f);
    colors[ImGuiCol_Button]                = ImVec4(0.18f, 0.32f, 0.46f, 1.00f);
    colors[ImGuiCol_ButtonHovered]         = ImVec4(0.24f, 0.42f, 0.60f, 1.00f);
    colors[ImGuiCol_ButtonActive]          = ImVec4(0.30f, 0.52f, 0.74f, 1.00f);
    colors[ImGuiCol_Header]                = ImVec4(0.18f, 0.24f, 0.32f, 1.00f);
    colors[ImGuiCol_HeaderHovered]         = ImVec4(0.24f, 0.32f, 0.44f, 1.00f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(0.28f, 0.38f, 0.52f, 1.00f);
    colors[ImGuiCol_Separator]             = ImVec4(0.22f, 0.26f, 0.34f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.35f, 0.42f, 0.55f, 1.00f);
    colors[ImGuiCol_SeparatorActive]       = ImVec4(0.45f, 0.55f, 0.70f, 1.00f);
    colors[ImGuiCol_ResizeGrip]            = ImVec4(0.20f, 0.25f, 0.32f, 1.00f);
    colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.30f, 0.38f, 0.48f, 1.00f);
    colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.40f, 0.50f, 0.65f, 1.00f);
    colors[ImGuiCol_Tab]                   = ImVec4(0.14f, 0.18f, 0.24f, 1.00f);
    colors[ImGuiCol_TabHovered]            = ImVec4(0.24f, 0.32f, 0.42f, 1.00f);
    colors[ImGuiCol_TabActive]             = ImVec4(0.20f, 0.28f, 0.38f, 1.00f);
    colors[ImGuiCol_PlotHistogram]         = ImVec4(0.26f, 0.75f, 0.95f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(0.40f, 0.85f, 1.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.14f, 0.17f, 0.22f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.24f, 0.28f, 0.36f, 1.00f);
    colors[ImGuiCol_TableBorderLight]      = ImVec4(0.18f, 0.22f, 0.28f, 1.00f);
    colors[ImGuiCol_TableRowBg]            = ImVec4(0.11f, 0.13f, 0.17f, 1.00f);
    colors[ImGuiCol_TableRowBgAlt]         = ImVec4(0.13f, 0.15f, 0.20f, 1.00f);

    style.WindowRounding    = 4.0f;
    style.ChildRounding     = 4.0f;
    style.FrameRounding     = 3.0f;
    style.PopupRounding     = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding      = 3.0f;
    style.TabRounding       = 4.0f;
    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 1.0f;
    style.ItemSpacing       = ImVec2(8.0f, 6.0f);
    style.FramePadding      = ImVec2(6.0f, 4.0f);
}

bool Application::initialize(int width, int height, const char* title) {
    windowWidth = width;
    windowHeight = height;
    windowTitle = (title && title[0] != '\0') ? title : "GeoLocal-Miner";

    if (!initWindow()) return false;
    if (!initOpenGL()) return false;

    // Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    setupTheme();

    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplOpenGL2_Init();

    // Ingest the initial synthetic placer demo dataset
    DataLoader::generateSyntheticPlacerDataset(grid, 20, 20);

    // Initialize the computational engine and dashboard
    dashboard.initialize(grid, prospectivityEngine, priorityEngine, validationEngine);

    // Default selection to the top-scoring hotspot cell for user delight
    mapView.setSelectedCell(6, 3);

    ::ShowWindow(hWnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hWnd);
    isRunning = true;

    return true;
}

void Application::run() {
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));

    while (isRunning) {
        while (::PeekMessageA(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessageA(&msg);
            if (msg.message == WM_QUIT) {
                isRunning = false;
            }
        }
        if (!isRunning) break;

        // Query active client dimensions
        RECT rect;
        ::GetClientRect(hWnd, &rect);
        int displayW = rect.right - rect.left;
        int displayH = rect.bottom - rect.top;

        if (displayW <= 0 || displayH <= 0) {
            ::Sleep(10);
            continue;
        }

        // Start Dear ImGui Frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Top-level Docking/Full Layout Window
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(displayW), static_cast<float>(displayH)));
        ImGuiWindowFlags rootFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                     ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));

        ImGui::Begin("RootWorkspace", nullptr, rootFlags);

        // 1. Top Header Banner
        dashboard.renderHeader();
        ImGui::Spacing();

        // Calculate layout geometry
        float availW = ImGui::GetContentRegionAvail().x;
        float availH = ImGui::GetContentRegionAvail().y;

        float bottomPanelH = 260.0f;
        float middleH = availH - bottomPanelH - 10.0f;

        float leftPanelW = 340.0f;
        float rightPanelW = 380.0f;
        float centerW = availW - leftPanelW - rightPanelW - 16.0f;

        // Row 1: Left Controls | Center Heatmap | Right Inspector
        // 1a. Left Controls Panel
        ImGui::BeginChild("LeftControlPanel", ImVec2(leftPanelW, middleH), true);
        dashboard.renderControls(grid, prospectivityEngine, priorityEngine, validationEngine, mapView);
        ImGui::EndChild();

        ImGui::SameLine();

        // 1b. Center Heatmap Panel
        ImGui::BeginChild("CenterHeatmapPanel", ImVec2(centerW, middleH), true);
        bool cellChanged = false;
        mapView.render(grid, cellChanged);
        ImGui::EndChild();

        ImGui::SameLine();

        // 1c. Right Inspector Panel
        ImGui::BeginChild("RightInspectorPanel", ImVec2(rightPanelW, middleH), true);
        int selR = mapView.getSelectedRow();
        int selC = mapView.getSelectedCol();
        zoneDetails.render(grid, selR, selC);
        mapView.setSelectedCell(selR, selC);
        ImGui::EndChild();

        ImGui::Spacing();

        // Row 2: Bottom Panel (Scorecard + Priority Ranking Table)
        ImGui::BeginChild("BottomPanel", ImVec2(-1, bottomPanelH), true);
        dashboard.renderSummaryCards();
        ImGui::Spacing();
        dashboard.renderRankingTable(grid, mapView, zoneDetails);
        ImGui::EndChild();

        ImGui::End(); // End RootWorkspace
        ImGui::PopStyleVar(3);

        // Rendering
        ImGui::Render();
        glViewport(0, 0, displayW, displayH);
        glClearColor(0.08f, 0.10f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        ::SwapBuffers(hDC);

        // Cap to ~60 FPS to prevent burning CPU
        ::Sleep(15);
    }
}

void Application::cleanup() {
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (hRC) {
        ::wglMakeCurrent(nullptr, nullptr);
        ::wglDeleteContext(hRC);
        hRC = nullptr;
    }
    if (hDC && hWnd) {
        ::ReleaseDC(hWnd, hDC);
        hDC = nullptr;
    }
    if (hWnd) {
        ::DestroyWindow(hWnd);
        hWnd = nullptr;
    }
    ::UnregisterClassA("GeoLocalMinerWindowClass", ::GetModuleHandleA(nullptr));
}

} // namespace GeoLocalMiner
