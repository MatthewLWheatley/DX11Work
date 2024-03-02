#define _XM_NO_INTRINSICS_

#include "main.h"

int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );

    if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
        return 0;

    if( FAILED( InitDevice() ) )
    {
        CleanupDevice();
        return 0;
    }

    // Main message loop
    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            Render();
        }
    }

    CleanupDevice();

    return ( int )msg.wParam;
}

//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow )
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon( hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    wcex.hCursor = LoadCursor( nullptr, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"lWindowClass";
    wcex.hIconSm = LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

    // Create window
    g_hInst = hInstance;
    RECT rc = { 0, 0, 1280, 720 };

	g_viewWidth = SCREEN_WIDTH;
	g_viewHeight = SCREEN_HEIGHT;

    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( L"lWindowClass", L"DirectX 11",
                           WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                           nullptr );
    if( !g_hWnd )
        return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DCompile
//
// With VS 11, we could load up prebuilt .cso files instead...
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile( const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;

    // Disable optimizations to further improve shader debugging
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* pErrorBlob = nullptr;
    hr = D3DCompileFromFile( szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob );
    if( FAILED(hr) )
    {
        if( pErrorBlob )
        {
            OutputDebugStringA( reinterpret_cast<const char*>( pErrorBlob->GetBufferPointer() ) );
            pErrorBlob->Release();
        }
        return hr;
    }
    if( pErrorBlob ) pErrorBlob->Release();

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect(g_hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
            D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);

        if (hr == E_INVALIDARG)
        {
            // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
            hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
        }

        if (SUCCEEDED(hr))
            break;
    }
    if (FAILED(hr))
        return hr;



    // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
    IDXGIFactory1* dxgiFactory = nullptr;
    {
        IDXGIDevice* dxgiDevice = nullptr;
        hr = g_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
        if (SUCCEEDED(hr))
        {
            IDXGIAdapter* adapter = nullptr;
            hr = dxgiDevice->GetAdapter(&adapter);
            if (SUCCEEDED(hr))
            {
                hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
                adapter->Release();
            }
            dxgiDevice->Release();
        }
    }
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"Failed to create device.", L"Error", MB_OK);
        return hr;
    }

    // Create swap chain
    IDXGIFactory2* dxgiFactory2 = nullptr;
    hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
    if (dxgiFactory2)
    {
        // DirectX 11.1 or later
        hr = g_pd3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&g_pd3dDevice1));
        if (SUCCEEDED(hr))
        {
            (void)g_pImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&g_pImmediateContext1));
        }

        DXGI_SWAP_CHAIN_DESC1 sd = {};
        sd.Width = width;
        sd.Height = height;
        sd.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;//  DXGI_FORMAT_R16G16B16A16_FLOAT;////DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 1;

        hr = dxgiFactory2->CreateSwapChainForHwnd(g_pd3dDevice, g_hWnd, &sd, nullptr, nullptr, &g_pSwapChain1);
        if (SUCCEEDED(hr))
        {
            hr = g_pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&g_pSwapChain));
        }

        dxgiFactory2->Release();
    }
    else
    {
        // DirectX 11.0 systems
        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 1;
        sd.BufferDesc.Width = width;
        sd.BufferDesc.Height = height;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = g_hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;

        hr = dxgiFactory->CreateSwapChain(g_pd3dDevice, &sd, &g_pSwapChain);
    }

    // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
    dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER);

    dxgiFactory->Release();

    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"Failed to create swapchain.", L"Error", MB_OK);
        return hr;
    }

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"Failed to create a back buffer.", L"Error", MB_OK);
        return hr;
    }

    hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
    pBackBuffer->Release();
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"Failed to create a render target.", L"Error", MB_OK);
        return hr;
    }

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth = {};
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = g_pd3dDevice->CreateTexture2D(&descDepth, nullptr, &g_pDepthStencil);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"Failed to create a depth / stencil texture.", L"Error", MB_OK);
        return hr;
    }



    D3D11_TEXTURE2D_DESC textureDesc = {};
    // Texture description
    ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
    textureDesc.Width = width; // Desired texture width
    textureDesc.Height = height; // Desired texture height
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Texture format
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

    //// Create the texture
    //hr = g_pd3dDevice->CreateTexture2D(&textureDesc, nullptr, &renderTargetTexture);
    //// Create the render target view
    //hr = g_pd3dDevice->CreateRenderTargetView(renderTargetTexture, nullptr, &renderToTextureRTV);
    //// Create the shader resource view
    //hr = g_pd3dDevice->CreateShaderResourceView(renderTargetTexture, nullptr, &renderToTextureSRV);


    // Create the texture
    hr = g_pd3dDevice->CreateTexture2D(&textureDesc, nullptr, &g_FSQTargetTexture);

    D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
    renderTargetViewDesc.Format = textureDesc.Format;
    renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    renderTargetViewDesc.Texture2D.MipSlice = 0; // Use the first and only mip level.

    hr = g_pd3dDevice->CreateRenderTargetView(g_FSQTargetTexture, &renderTargetViewDesc, &g_pFSQRenderTargetView);


    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
    shaderResourceViewDesc.Format = textureDesc.Format;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MostDetailedMip = 0; // The most detailed mip level.
    shaderResourceViewDesc.Texture2D.MipLevels = 1; // The total number of mip levels to use.

    hr = g_pd3dDevice->CreateShaderResourceView(g_FSQTargetTexture, &shaderResourceViewDesc, &g_pFSQRenderToTextureSRV);


    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"Failed to create a depth / stencil view.", L"Error", MB_OK);
        return hr;
    }

    //g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports( 1, &vp );


	hr = InitRunTimeParameters();
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"Failed to initialise mesh.", L"Error", MB_OK);
		return hr;
	}

	hr = InitWorld(width, height);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"Failed to initialise world.", L"Error", MB_OK);
		return hr;
	}



    g_FSQGS.setSize(1.0f);
    g_FSQGS.setPos(XMFLOAT2(0.0f, 0.0f));

    hr = g_GameObject.initMesh(g_pd3dDevice, g_pImmediateContext);
    hr = g_FSQ.initMesh(g_pd3dDevice, g_pImmediateContext);
    hr = g_FSQGS.initMesh(g_pd3dDevice, g_pImmediateContext);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"Failed to init mesh in game object.", L"Error", MB_OK);
        return hr;
    }


    g_Debugger.Init(&g_hWnd, g_pd3dDevice, g_pImmediateContext);

    return S_OK;
}

// ***************************************************************************************
// InitMesh
// ***************************************************************************************
HRESULT		InitRunTimeParameters()
{
    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;

    HRESULT hr = CompileShaderFromFile(L"shader.fx", "VS", "vs_4_0", &pVSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the vertex shader
    hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShaderDefault);
    if (FAILED(hr))
    {
        pVSBlob->Release();
        return hr;
    }


    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = ARRAYSIZE(layout);

    // Set the input layout
    g_pImmediateContext->IASetInputLayout(g_pVertexLayoutDefault);

    // Compile the pixel shader
    ID3DBlob* pPSBlob = nullptr;

    hr = CompileShaderFromFile(L"shader.fx", "PS", "ps_4_0", &pPSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }


    // Create the pixel shader
    hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShaderDefault);
    pPSBlob->Release();
    if (FAILED(hr))
        return hr;


    // Create the input layout
    hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &g_pVertexLayoutDefault);
    pVSBlob->Release();
    if (FAILED(hr))
        return hr;
    ID3DBlob* pVSBlobOther = nullptr;

    // Create the constant buffer
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pConstantBufferDefault);
    if (FAILED(hr))
        return hr;

    // Create the light constant buffer
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(LightPropertiesConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pLightConstantBufferDefault);
    if (FAILED(hr))
        return hr;

    //--------------------------------------------------------------------------------------

    // Compile the vertex shader
    ID3DBlob* pVSBlob2 = nullptr;

    hr = CompileShaderFromFile(L"FSQ.fx", "VS", "vs_4_0", &pVSBlob2);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the vertex shader
    hr = g_pd3dDevice->CreateVertexShader(pVSBlob2->GetBufferPointer(), pVSBlob2->GetBufferSize(), nullptr, &g_pFSQVertexShaderDefault);
    if (FAILED(hr))
    {
        pVSBlob2->Release();
        return hr;
    }


    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout2[] =
    {
    	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements2 = ARRAYSIZE(layout2);

    // Set the input layout
    g_pImmediateContext->IASetInputLayout(g_pFSQVertexLayoutDefault);

    // Compile the pixel shader
    ID3DBlob* pPSBlob2 = nullptr;

    hr = CompileShaderFromFile(L"FSQ.fx", "PS", "ps_4_0", &pPSBlob2);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the pixel shader
    hr = g_pd3dDevice->CreatePixelShader(pPSBlob2->GetBufferPointer(), pPSBlob2->GetBufferSize(), nullptr, &g_pFSQPixelShaderDefault);
    pPSBlob2->Release();
    if (FAILED(hr))
        return hr;

    hr = CompileShaderFromFile(L"FSQ.fx", "PSGS", "ps_4_0", &pPSBlob2);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the pixel shader
    hr = g_pd3dDevice->CreatePixelShader(pPSBlob2->GetBufferPointer(), pPSBlob2->GetBufferSize(), nullptr, &g_pFSQGSPixelShaderDefault);
    pPSBlob2->Release();
    if (FAILED(hr))
        return hr;

    // Create the input layout
    hr = g_pd3dDevice->CreateInputLayout(layout2, numElements2, pVSBlob2->GetBufferPointer(), pVSBlob2->GetBufferSize(), &g_pFSQVertexLayoutDefault);
    pVSBlob2->Release();
    if (FAILED(hr))
        return hr;

    // Create the constant buffer
    D3D11_BUFFER_DESC bd2 = {};
    bd2.Usage = D3D11_USAGE_DEFAULT;
    bd2.ByteWidth = sizeof(ConstantBuffer);
    bd2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd2.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer(&bd2, nullptr, &g_pFSQConstantBufferDefault);
    if (FAILED(hr))
        return hr;
    return hr;
}

// ***************************************************************************************
// InitWorld
// ***************************************************************************************
HRESULT		InitWorld(int width, int height)
{
    g_pCamera = new Camera(XMFLOAT3(0.0f, 0, -6), XMFLOAT3(0, 0, 1), XMFLOAT3(0.0f, 1.0f, 0.0f));

	// Initialize the projection matrix
    constexpr float fovAngleY = XMConvertToRadians(60.0f);
	g_Projection = XMMatrixPerspectiveFovLH(fovAngleY, (float)width / (FLOAT)height, 0.01f, 1000.0f);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
    g_GameObject.cleanup();
    g_FSQ.cleanup();
    g_FSQGS.cleanup();

    // Remove any bound render target or depth/stencil buffer
    ID3D11RenderTargetView* nullViews[] = { nullptr };
    g_pImmediateContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);

    if( g_pImmediateContext ) g_pImmediateContext->ClearState();
    // Flush the immediate context to force cleanup
    if (g_pImmediateContext1) g_pImmediateContext1->Flush();
    g_pImmediateContext->Flush();

    if (g_pLightConstantBufferDefault)
        g_pLightConstantBufferDefault->Release();
    if (g_pVertexLayoutDefault) g_pVertexLayoutDefault->Release();
    if (g_pFSQVertexLayoutDefault) g_pFSQVertexLayoutDefault->Release();
    if( g_pConstantBufferDefault ) g_pConstantBufferDefault->Release();
    if( g_pFSQConstantBufferDefault ) g_pFSQConstantBufferDefault->Release();
    if( g_pVertexShaderDefault ) g_pVertexShaderDefault->Release();
    if( g_pFSQVertexShaderDefault ) g_pFSQVertexShaderDefault->Release();
    if( g_pDepthStencil ) g_pDepthStencil->Release();
    if( g_pDepthStencilView ) g_pDepthStencilView->Release();
    if( g_pRenderTargetView ) g_pRenderTargetView->Release();
    if( g_pSwapChain1 ) g_pSwapChain1->Release();
    if( g_pSwapChain ) g_pSwapChain->Release();
    if( g_pImmediateContext1 ) g_pImmediateContext1->Release();
    if( g_pImmediateContext ) g_pImmediateContext->Release();

    // Cleanup ImGui
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    ID3D11Debug* debugDevice = nullptr;
    g_pd3dDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debugDevice));

    if (g_pd3dDevice1) g_pd3dDevice1->Release();
    if (g_pd3dDevice) g_pd3dDevice->Release();

    // handy for finding dx memory leaks
    debugDevice->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);

    if (debugDevice)
        debugDevice->Release();
}

// Function to center the mouse in the window
void CenterMouseInWindow(HWND hWnd)
{
    // Get the dimensions of the window
    RECT rect;
    GetClientRect(hWnd, &rect);

    // Calculate the center position
    POINT center;
    center.x = (rect.right - rect.left) / 2;
    center.y = (rect.bottom - rect.top) / 2;

    // Convert the client area point to screen coordinates
    ClientToScreen(hWnd, &center);

    // Move the cursor to the center of the screen
    //SetCursorPos(center.x, center.y);
}

//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    

    PAINTSTRUCT ps;
    HDC hdc;

    float movement = 0.2f;
    static bool mouseDown = false;

    ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam);

    switch( message )
    {
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEWHEEL:
    case WM_KEYDOWN:
        switch (wParam)
        {
        case 27:
            PostQuitMessage(0);
            break;
        case 'W':
            g_pCamera->MoveForward(movement);  // Adjust distance as needed
            break;
        case 'A':
            g_pCamera->StrafeLeft(movement);  // Adjust distance as needed
            break;
        case 'S':
            g_pCamera->MoveBackward(movement);  // Adjust distance as needed
            break;
        case 'D':
            g_pCamera->StrafeRight(movement);  // Adjust distance as needed
            break;
        case 'P':
            if (!g_PKeyPressed)
            {
                g_Debugger.AddLog("Mouse Lock Toggled");
                g_CenterMouse = !g_CenterMouse;
                g_PKeyPressed = true;
            }
            break;
        }
        break;
    case WM_KEYUP:
    	switch (wParam)
            {
            case 'P':
                g_PKeyPressed = false;
                break;
            }
    	break;
    case WM_LBUTTONDOWN:
        mouseDown = true;
        break;
    case WM_LBUTTONUP:
        mouseDown = false;
        break;
    case WM_MOUSEMOVE:
    {
        if (!mouseDown || !g_CenterMouse) // Check if mouse is down and centering is enabled
        {
            break;
        }
        // Get the dimensions of the window
        RECT rect;
        GetClientRect(hWnd, &rect);

        // Calculate the center position of the window
        POINT windowCenter;
        windowCenter.x = (rect.right - rect.left) / 2;
        windowCenter.y = (rect.bottom - rect.top) / 2;

        // Convert the client area point to screen coordinates
        ClientToScreen(hWnd, &windowCenter);

        // Get the current cursor position
        POINTS mousePos = MAKEPOINTS(lParam);
        POINT cursorPos = { mousePos.x, mousePos.y};
        ClientToScreen(hWnd, &cursorPos);

        // Calculate the delta from the window center
        POINT delta;
        delta.x = cursorPos.x - windowCenter.x;
        delta.y = cursorPos.y - windowCenter.y;

        // Update the camera with the delta
        // (You may need to convert POINT to POINTS or use the deltas as is)
        g_pCamera->UpdateLookAt({ static_cast<short>(delta.x), static_cast<short>(delta.y) });

        // Recenter the cursor
        SetCursorPos(windowCenter.x, windowCenter.y);
    }
    break;
   
    case WM_ACTIVATE:
        if (LOWORD(wParam) != WA_INACTIVE) 
        {
            CenterMouseInWindow(hWnd);
        }
        break;
    case WM_PAINT:
        hdc = BeginPaint( hWnd, &ps );
        EndPaint( hWnd, &ps );
        break;

    case WM_DESTROY:
        PostQuitMessage( 0 );
        break;

        // Note that this tutorial does not handle resizing (WM_SIZE) requests,
        // so we created the window without the resize border.

    default:
        return DefWindowProc( hWnd, message, wParam, lParam );
    }
    return 0;
}

void setupLightForRender()
{
    Light light;
    light.Enabled = static_cast<int>(true);
    if (g_LightType)light.LightType = PointLight;
    else light.LightType = DirectionalLight;
    light.Color = XMFLOAT4(g_lightColor);
    light.SpotAngle = XMConvertToRadians(45.0f);
    light.ConstantAttenuation = 1.0f;
    light.LinearAttenuation = 1;
    light.QuadraticAttenuation = 1;

    // set up the light

    XMFLOAT4 LightPosition(g_pCamera->GetPosition().x, g_pCamera->GetPosition().y, g_pCamera->GetPosition().z, 1);
    if (!g_lightOnCamera)
    {
        LightPosition = XMFLOAT4(g_lightPosition.x, g_lightPosition.y, g_lightPosition.z, 1.0f);
    }
    else
    {
        g_lightPosition = { LightPosition.x,LightPosition.y,LightPosition.z };
    }
    light.Position = LightPosition;
    XMVECTOR LightDirection = XMVectorSet(-LightPosition.x, -LightPosition.y, -LightPosition.z, 0.0f);
    LightDirection = XMVector3Normalize(LightDirection);
    XMStoreFloat4(&light.Direction, LightDirection);

    LightPropertiesConstantBuffer lightProperties;
    lightProperties.EyePosition = XMFLOAT4(g_pCamera->GetPosition().x, g_pCamera->GetPosition().y, g_pCamera->GetPosition().z, 0.0f);
    lightProperties.Lights[0] = light;
    g_pImmediateContext->UpdateSubresource(g_pLightConstantBufferDefault, 0, nullptr, &lightProperties, 0, 0);
}

float calculateDeltaTime()
{
    // Update our time
    static float deltaTime = 0.0f;
    static ULONGLONG timeStart = 0;
    ULONGLONG timeCur = GetTickCount64();
    if (timeStart == 0)
        timeStart = timeCur;
    deltaTime = (timeCur - timeStart) / 1000.0f;
    timeStart = timeCur;

    float FPS60 = 1.0f / 60.0f;
    static float cummulativeTime = 0;

    // cap the framerate at 60 fps 
    cummulativeTime += deltaTime;
    if (cummulativeTime >= FPS60) {
        cummulativeTime = cummulativeTime - FPS60;
    }
    else {
        return 0;
    }

    return deltaTime;
}

//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void Render()
{
    // Calculate delta time (capped at 60 fps)
    float t = calculateDeltaTime();
    if (t == 0.0f)
        return;
    switch (g_RendererShader)
    {
    case 1:
        g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);
        // Clear the render target and depth stencil if necessary
        g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::Gray);
        g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
        break;
    default:

        g_pImmediateContext->OMSetRenderTargets(1, &g_pFSQRenderTargetView, g_pDepthStencilView);
        // Clear the render target and depth stencil if necessary
        g_pImmediateContext->ClearRenderTargetView(g_pFSQRenderTargetView, Colors::Gray);
        g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
        break;
    }
    // Update the cube transform, material, etc.
    g_GameObject.update(t, g_pImmediateContext, g_cubeRotaionSpeed, g_cubePosition);

    // Get the game object world transform
    XMMATRIX mGO = XMLoadFloat4x4(g_GameObject.getTransform());

    // Store world and the view / projection in a constant buffer for the vertex shader to use
    ConstantBuffer cb1;
    cb1.mWorld = XMMatrixTranspose(mGO);
    cb1.mView = XMMatrixTranspose(g_pCamera->GetViewMatrix());
    cb1.mProjection = XMMatrixTranspose(g_Projection);
    //cb1.vOutputColor = XMFLOAT4(0, 0, 0, 0);

    setupLightForRender();

    // Update subresources and set shaders for game object
    g_pImmediateContext->UpdateSubresource(g_pConstantBufferDefault, 0, nullptr, &cb1, 0, 0);
    g_pImmediateContext->IASetInputLayout(g_pVertexLayoutDefault);
    g_pImmediateContext->VSSetShader(g_pVertexShaderDefault, nullptr, 0);
    g_pImmediateContext->PSSetShader(g_pPixelShaderDefault, nullptr, 0);
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBufferDefault);
    g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pLightConstantBufferDefault);

    ID3D11Buffer* materialCB = g_GameObject.getMaterialConstantBuffer();
    g_pImmediateContext->PSSetConstantBuffers(1, 1, &materialCB);

    g_GameObject.draw(g_pImmediateContext);

    if (g_RendererShader != 1)
    {
        cb1.mWorld = XMMatrixIdentity();
        cb1.mView = XMMatrixIdentity();
        cb1.mProjection = XMMatrixIdentity();
        g_pImmediateContext->UpdateSubresource(g_pConstantBufferDefault, 0, nullptr, &cb1, 0, 0);

        g_pImmediateContext->PSSetShader(g_pFSQPixelShaderDefault, nullptr, 0);

        g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

        g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::MidnightBlue);
        g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

        if (g_RendererShader == 0)
        {
            g_FSQ.setTexture(g_pFSQRenderToTextureSRV);
            g_FSQ.draw(g_pImmediateContext);
        }

        if (g_RendererShader == 2)
        {
            g_pImmediateContext->PSSetShader(g_pFSQGSPixelShaderDefault, nullptr, 0);
            g_FSQGS.setTexture(g_pFSQRenderToTextureSRV);
            g_FSQGS.draw(g_pImmediateContext);
        }
    }

    g_Debugger.Draw("DebugMenu");
    g_pSwapChain->Present(0, 0);
}
