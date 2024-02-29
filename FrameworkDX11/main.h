#pragma once

#include <windows.h>
#include <windowsx.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <DirectXCollision.h>
#include "DDSTextureLoader.h"
#include "resource.h"
#include <iostream>

#include "DrawableGameObject.h"
#include "structures.h"

#include <vector>

#include "imgui\\imgui.h"
#include "imgui\\imgui_impl_dx11.h"
#include "imgui\\imgui_impl_win32.h"
#include "constants.h"
#include "Camera.h"
#include "DebugLogger.cpp"
#include "FullScreenQuad.h"

using namespace std;

typedef vector<DrawableGameObject*> vecDrawables;

Camera* g_pCamera;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT		InitWindow(HINSTANCE hInstance, int nCmdShow);
HRESULT		InitDevice();
HRESULT		InitRunTimeParameters();
HRESULT		InitWorld(int width, int height);
void		CleanupDevice();
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void		Render();



//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE               g_hInst = nullptr;
HWND                    g_hWnd = nullptr;
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11Device1* g_pd3dDevice1 = nullptr;
ID3D11DeviceContext* g_pImmediateContext = nullptr;
ID3D11DeviceContext1* g_pImmediateContext1 = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
IDXGISwapChain1* g_pSwapChain1 = nullptr;
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
ID3D11RenderTargetView* g_pFSQRenderTargetView = nullptr;
ID3D11Texture2D* g_pDepthStencil = nullptr;
ID3D11DepthStencilView* g_pDepthStencilView = nullptr;

ID3D11VertexShader* g_pVertexShaderDefault = nullptr;
ID3D11VertexShader* g_pVertexShaderOther = nullptr;

ID3D11VertexShader* g_pFSQVertexShaderDefault = nullptr;

ID3D11PixelShader* g_pPixelShaderDefault = nullptr;
ID3D11PixelShader* g_pPixelShaderOther = nullptr;

ID3D11PixelShader* g_pFSQPixelShaderDefault = nullptr;

ID3D11InputLayout* g_pVertexLayoutDefault = nullptr;
ID3D11InputLayout* g_pVertexLayoutOther = nullptr;

ID3D11InputLayout* g_pFSQVertexLayoutDefault = nullptr;

ID3D11Buffer* g_pConstantBufferDefault = nullptr;
ID3D11Buffer* g_pConstantBufferOther = nullptr;

ID3D11Buffer* g_pLightConstantBufferDefault = nullptr;
ID3D11Buffer* g_pLightConstantBufferOther = nullptr;

ID3D11Buffer* g_pFSQConstantBufferDefault = nullptr;

ID3D11ShaderResourceView* g_FSQShaderResourceView = nullptr;

ID3D11Texture2D* fSQTextureMap = nullptr;

XMMATRIX                g_Projection;

int						g_viewWidth;
int						g_viewHeight;

DrawableGameObject		g_GameObject;
FullScreenQuad		g_FullScreenQuad;


//debug stuffs below
bool openControlsWindow = false;
bool openCubeWindow = false;
bool openLightWindow = false;
bool openRenderingWindow = false;
DebugLogger debug;
void SetUpGUI();

//cube
XMFLOAT3 g_cubeRotaionSpeed = {0.0f,0.0f, 0.0f};
XMFLOAT3 g_cubePosition = {0.0f,0.0f, 0.0f};

//controls
bool g_CenterMouse = true;
bool g_PKeyPressed = false;

//Lighting
XMFLOAT4 g_lightColor = { 1.0f, 1.0f, 1.0f,1.0f};
XMFLOAT3 g_lightPosition = { 0.0f, 0.0f, 0.0f};
bool g_lightOnCamera = true;
bool g_LightType = true;

//Rendering
int g_RendererShader = 0;
