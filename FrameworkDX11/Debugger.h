#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <vector>

#include "Camera.h"
#include "imgui\\imgui.h"
#include "imgui\\imgui_impl_dx11.h"
#include "imgui\\imgui_impl_win32.h"

class Debugger
{
public:
    Debugger();
    ~Debugger();
    void Init(HWND* hWnd, ID3D11Device* pd3dDevice, ID3D11DeviceContext* pImmediateContext);

    void Draw(const char* title);

    void AddLog(const std::string& msg);
    void SaveLogToFile();

private:
    std::vector<std::string> messages;

    bool openControlsWindow = false;
    bool openCubeWindow = false;
    bool openLightWindow = false;
    bool openRenderingWindow = false;
};