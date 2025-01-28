#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <string>

// Link libs (an alternative to using CMake's target_link_libraries)
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

// Global Direct3D pointers
IDXGISwapChain*         g_SwapChain         = nullptr;
ID3D11Device*           g_D3DDevice         = nullptr;
ID3D11DeviceContext*    g_D3DContext        = nullptr;
ID3D11RenderTargetView* g_RenderTargetView  = nullptr;

// Pipeline objects
ID3D11Buffer*           g_VertexBuffer      = nullptr;
ID3D11VertexShader*     g_VertexShader      = nullptr;
ID3D11PixelShader*      g_PixelShader       = nullptr;
ID3D11InputLayout*      g_InputLayout       = nullptr;

// Forward declarations
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
bool InitD3D(HWND hwnd);
void CleanupD3D();
void RenderFrame();
bool InitPipelineAndBuffers();

//--------------------------------------------------------------------------------------
// WinMain: Entry point for Windows Desktop apps
//--------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    // 1. Register the window class
    const char CLASS_NAME[] = "DXWindowClass";

    WNDCLASSEX wc = {};
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc; 
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error", MB_ICONERROR | MB_OK);
        return 0;
    }

    // 2. Create the window
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Direct3D 11 - Draw Triangle",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1280, 720,
        NULL, NULL,
        hInstance,
        NULL
    );
    if (!hwnd) {
        MessageBox(NULL, "Window Creation Failed!", "Error", MB_ICONERROR | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 3. Initialize Direct3D
    if (!InitD3D(hwnd)) {
        MessageBox(NULL, "Failed to initialize Direct3D", "Error", MB_ICONERROR | MB_OK);
        return 0;
    }

    // 4. Create pipeline (shaders, input layout) and vertex buffer
    if (!InitPipelineAndBuffers()) {
        MessageBox(NULL, "Failed to initialize pipeline/buffers", "Error", MB_ICONERROR | MB_OK);
        return 0;
    }

    // 5. Main message loop
    MSG msg = {};
    while (true) {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            // Render loop
            RenderFrame();
        }
    }

    // 6. Clean up
    CleanupD3D();
    return (int)msg.wParam;
}

//--------------------------------------------------------------------------------------
// WndProc: Handle window messages
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
}

//--------------------------------------------------------------------------------------
// InitD3D: Creates device, swap chain, render target, viewport
//--------------------------------------------------------------------------------------
bool InitD3D(HWND hwnd)
{
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1;
    sd.BufferDesc.Width = 1280;
    sd.BufferDesc.Height = 720;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    // Uncomment to enable D3D11 debug layer (requires Graphics Tools installed):
    // createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;

    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,                // default adapter
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createDeviceFlags,
        nullptr, 0,             // default feature levels
        D3D11_SDK_VERSION,
        &sd,
        &g_SwapChain,
        &g_D3DDevice,
        &featureLevel,
        &g_D3DContext
    );
    if (FAILED(hr))
        return false;

    // Create render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = g_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
    if (FAILED(hr))
        return false;

    hr = g_D3DDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_RenderTargetView);
    pBackBuffer->Release();
    if (FAILED(hr))
        return false;

    g_D3DContext->OMSetRenderTargets(1, &g_RenderTargetView, nullptr);

    // Set viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)1280;
    vp.Height = (FLOAT)720;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_D3DContext->RSSetViewports(1, &vp);

    return true;
}

//--------------------------------------------------------------------------------------
// InitPipelineAndBuffers: Create shaders, input layout, vertex buffer
//--------------------------------------------------------------------------------------
bool InitPipelineAndBuffers()
{
    HRESULT hr;

    //
    // 1. Define and compile our HLSL shaders (inline for demo)
    //
    const char* vsSource = R"(
    struct VS_INPUT {
        float3 pos   : POSITION;
        float4 color : COLOR;
    };

    struct PS_INPUT {
        float4 pos   : SV_POSITION;
        float4 color : COLOR;
    };

    PS_INPUT main(VS_INPUT input) {
        PS_INPUT output;
        // Pass position as clip-space (x,y in [-1..1], z=0)
        output.pos = float4(input.pos, 1.0);
        output.color = input.color;
        return output;
    }
    )";

    const char* psSource = R"(
    struct PS_INPUT {
        float4 pos   : SV_POSITION;
        float4 color : COLOR;
    };

    float4 main(PS_INPUT input) : SV_TARGET {
        return input.color; // Just output the vertex color
    }
    )";

    // Compile vertex shader
    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    hr = D3DCompile(vsSource, strlen(vsSource), nullptr, nullptr, nullptr,
                    "main", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), "VS Compile Error", MB_OK);
            errorBlob->Release();
        }
        return false;
    }

    // Create the vertex shader
    hr = g_D3DDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_VertexShader);
    if (FAILED(hr)) {
        vsBlob->Release();
        return false;
    }

    // Compile pixel shader
    ID3DBlob* psBlob = nullptr;
    hr = D3DCompile(psSource, strlen(psSource), nullptr, nullptr, nullptr,
                    "main", "ps_5_0", 0, 0, &psBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            MessageBoxA(nullptr, (char*)errorBlob->GetBufferPointer(), "PS Compile Error", MB_OK);
            errorBlob->Release();
        }
        vsBlob->Release();
        return false;
    }

    // Create the pixel shader
    hr = g_D3DDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_PixelShader);
    if (FAILED(hr)) {
        vsBlob->Release();
        psBlob->Release();
        return false;
    }

    //
    // 2. Create input layout (match VS_INPUT in HLSL)
    //
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        // SemanticName, SemanticIndex, Format, InputSlot, AlignedByteOffset, InputSlotClass, InstanceDataStepRate
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,                         D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,                     D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = g_D3DDevice->CreateInputLayout(
        layoutDesc, 
        _countof(layoutDesc),
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        &g_InputLayout
    );
    vsBlob->Release();
    psBlob->Release();
    if (FAILED(hr)) {
        return false;
    }

    //
    // 3. Create a vertex buffer for a simple triangle
    //
    struct Vertex {
        float x, y, z;
        float r, g, b, a;
    };

    Vertex triangleVerts[] = {
        //   X      Y     Z      R    G    B    A
        {  0.0f,  0.5f, 0.0f,  1.0f,0.0f,0.0f,1.0f }, // Top (Red)
        {  0.45f,-0.5f, 0.0f,  0.0f,1.0f,0.0f,1.0f }, // Right (Green)
        { -0.45f,-0.5f, 0.0f,  0.0f,0.0f,1.0f,1.0f }  // Left (Blue)
    };

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(triangleVerts);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = triangleVerts;

    hr = g_D3DDevice->CreateBuffer(&bd, &initData, &g_VertexBuffer);
    if (FAILED(hr)) {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------
// RenderFrame: Clear screen, set pipeline, draw triangle, present
//--------------------------------------------------------------------------------------
void RenderFrame()
{
    // 1. Clear the render target to a solid color (Cornflower Blue)
    float clearColor[4] = { 0.39f, 0.58f, 0.93f, 1.0f };
    g_D3DContext->ClearRenderTargetView(g_RenderTargetView, clearColor);

    // 2. Set our pipeline states (shaders, input layout)
    g_D3DContext->IASetInputLayout(g_InputLayout);

    g_D3DContext->VSSetShader(g_VertexShader, nullptr, 0);
    g_D3DContext->PSSetShader(g_PixelShader, nullptr, 0);

    // 3. Set vertex buffer
    UINT stride = sizeof(float) * 7;  // (x,y,z) + (r,g,b,a)
    UINT offset = 0;
    g_D3DContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

    // 4. Set primitive topology (triangle list)
    g_D3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 5. Draw 3 vertices => 1 triangle
    g_D3DContext->Draw(3, 0);

    // 6. Present (swap buffers)
    g_SwapChain->Present(1, 0);
}

//--------------------------------------------------------------------------------------
// CleanupD3D: Release D3D objects
//--------------------------------------------------------------------------------------
void CleanupD3D()
{
    if (g_VertexBuffer)      g_VertexBuffer->Release();
    if (g_InputLayout)       g_InputLayout->Release();
    if (g_VertexShader)      g_VertexShader->Release();
    if (g_PixelShader)       g_PixelShader->Release();
    if (g_SwapChain)         g_SwapChain->Release();
    if (g_RenderTargetView)  g_RenderTargetView->Release();
    if (g_D3DContext)        g_D3DContext->Release();
    if (g_D3DDevice)         g_D3DDevice->Release();
}
