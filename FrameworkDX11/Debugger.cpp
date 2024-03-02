#include "Debugger.h"

Debugger::Debugger()
{
	
}

Debugger::~Debugger()
{

}

void Debugger::Init(HWND* hWnd, ID3D11Device* pd3dDevice, ID3D11DeviceContext* pImmediateContext)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(pd3dDevice, pImmediateContext);
}

void Debugger::Draw(const char* title)
{
	// Start ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Create a window called "My Window" and a slider within it
    ImGui::Begin("DebugMenu"); // Begin a new window
    ImGui::Text("foreasier access to debug menu");
    ImGui::Text("Press \"P\" To toggle mouse lock");

    float availableWidth = ImGui::GetContentRegionAvail().x;

    if (ImGui::MenuItem("controls"))
    {
        // Flag to open controls window
        openControlsWindow = true;
    }
    if (ImGui::MenuItem("Cube"))
    {
        // Flag to open Cube window
        openCubeWindow = true;
    }
    if (ImGui::MenuItem("light"))
    {
        // Flag to open light window
        openLightWindow = true;
    }
    if (ImGui::MenuItem("Rendering"))
    {
        // Flag to open light window
        openRenderingWindow = true;
    }

    ImGui::End(); // End the main window

    // Controls window
    if (openControlsWindow)
    {
        ImGui::Begin("Controls Settings", &openControlsWindow); // Use a flag to close the window
        ImGui::Text("Press \'P\' to toggle");
        ImGui::Checkbox("MouseLock", &g_CenterMouse);
        ImGui::End(); // End the Controls window
    }

    // Cube window
    if (openCubeWindow)
    {
        ImGui::Begin("Cube Settings", &openCubeWindow); // Use a flag to close the window
        float sliderWidth = availableWidth / 3.0f - ImGui::GetStyle().ItemSpacing.x; // Adjust for spacing
        ImGui::PushItemWidth(sliderWidth);
        ImGui::Text("Rotation speed of cube");
        ImGui::SliderFloat("##X rotation speed", &g_cubeRotaionSpeed.x, -1.0f, 1.0f);
        ImGui::SameLine();
        ImGui::SliderFloat("##Y rotation speed", &g_cubeRotaionSpeed.y, -1.0f, 1.0f);
        ImGui::SameLine();
        ImGui::SliderFloat("##Z rotation speed", &g_cubeRotaionSpeed.z, -1.0f, 1.0f);
        ImGui::PopItemWidth();
        if (ImGui::Button("Rest to zero")) { g_cubeRotaionSpeed = { 0.0f,0.0f,0.0f }; }

        sliderWidth = availableWidth / 3.0f - ImGui::GetStyle().ItemSpacing.x; // Adjust for spacing
        ImGui::PushItemWidth(sliderWidth);
        ImGui::Text("Position of cube");
        ImGui::InputFloat("##X cube", &g_cubePosition.x);
        ImGui::SameLine();
        ImGui::InputFloat("##Y cube", &g_cubePosition.y);
        ImGui::SameLine();
        ImGui::InputFloat("##Z cube", &g_cubePosition.z);
        ImGui::PopItemWidth();
        if (ImGui::Button("Rest to zero")) { g_cubePosition = { 0.0f,0.0f,0.0f }; }
        ImGui::End();
    }

    // Light window
    if (openLightWindow)
    {
        ImGui::Begin("Light Settings", &openLightWindow); // Use a flag to close the window

        float availableWidth = ImGui::GetContentRegionAvail().x;
        float sliderWidth = availableWidth / 3.0f - ImGui::GetStyle().ItemSpacing.x; // Adjust for spacing
        //light
        sliderWidth = availableWidth / 4.0f - ImGui::GetStyle().ItemSpacing.x; // Adjust for spacing
        ImGui::PushItemWidth(sliderWidth);
        ImGui::Text("Colour Of Light");
        ImGui::SliderFloat("##R", &g_lightColor.x, 0.0f, 1.0f);
        ImGui::SameLine();
        ImGui::SliderFloat("##G", &g_lightColor.y, 0.0f, 1.0f);
        ImGui::SameLine();
        ImGui::SliderFloat("##B", &g_lightColor.z, 0.0f, 1.0f);
        ImGui::SameLine();
        ImGui::SliderFloat("##A", &g_lightColor.w, 0.0f, 1.0f);
        ImGui::PopItemWidth();
        if (ImGui::Button("Rest to White")) { g_lightColor = { 1.0f,1.0f,1.0f,1.0f }; }

        ImGui::Checkbox("light follows camera", &g_lightOnCamera);
        sliderWidth = availableWidth / 3.0f - ImGui::GetStyle().ItemSpacing.x; // Adjust for spacing
        ImGui::PushItemWidth(sliderWidth);
        ImGui::Text("Position Of Light");
        ImGui::InputFloat("##X light pos", &g_lightPosition.x);
        ImGui::SameLine();
        ImGui::InputFloat("##Y light pos", &g_lightPosition.y);
        ImGui::SameLine();
        ImGui::InputFloat("##Z light pos", &g_lightPosition.z);
        ImGui::PopItemWidth();
        if (ImGui::Button("Rest to White"))
        {
            g_pCamera->SetPosition(XMFLOAT3{ 0.0f, 0.0f, -3.0f });
            g_lightPosition = { 0.0f, 0.0f, -3.0f };
        }
        if (ImGui::Button("Switch Light Type"))
        {
            g_LightType = !g_LightType;
        }
        ImGui::SameLine();
        if (g_LightType)
        {
            ImGui::Text("Spot light");
        }
        else ImGui::Text("Direction Light");
        ImGui::End();
    }

    // Rendering window
    if (openRenderingWindow)
    {
        ImGui::Begin("Rendinging Settings", &openControlsWindow); // Use a flag to close the window
        const char* RendererOptions[] = { "FSQ","Non-FSQ","GreyScale","GreyScale" };
        ImGui::Text("Renderer Shader: %s", RendererOptions[g_RendererShader]);
        if (ImGui::BeginMenu("RendererOptions"))
        {
            if (ImGui::MenuItem("FSQ")) {
                g_RendererShader = 0;
                AddLog("FSQ Renderer activated");
            }
            if (ImGui::MenuItem("Non-FSQ"))
            {
                g_RendererShader = 1;
                AddLog("non-FSQ Renderer activated");
            }
            if (ImGui::MenuItem("GreyScale"))
            {
                g_RendererShader = 2;
                AddLog("GreyScale Renderer activated");
            }
            ImGui::EndMenu();
        }
        ImGui::End(); // End the Controls window
    }

    if (ImGui::Begin("Logs"))
    {
        for (const auto& msg : messages) {
            ImGui::TextUnformatted(msg.c_str());
        }

        if (ImGui::Button("Save Log")) {
            SaveLogToFile(); // Call the method to save the log
        }
    }
    ImGui::End();


    // Render ImGui
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}   

void Debugger::AddLog(const std::string& msg)
{
		messages.push_back(msg);
}

void Debugger::SaveLogToFile()
{
    // Get current time
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);

    // Convert time to tm struct
    std::tm now_tm = *std::localtime(&now_time_t);

    // Format time into a string with the format "debug_log{YYYY-MM-DD_HH-MM-SS}.txt"
    char filename[80];
    strftime(filename, sizeof(filename), "DebugLogs\\debug_log%Y-%m-%d_%H-%M-%S.txt", &now_tm);

    // Proceed with saving the log to the file
    std::ofstream fileStream(filename, std::ios::out | std::ios::app);
    if (!fileStream.is_open()) {
        std::cerr << "Failed to open log file for writing: " << filename << std::endl;
        return;
    }

    for (const auto& msg : messages) {
        fileStream << msg << std::endl;
    }

    fileStream.close();
}