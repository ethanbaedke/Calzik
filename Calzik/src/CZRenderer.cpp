#include "CZRenderer.h"

CZRenderer::CZRenderer(HWND hwnd)
{
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;
    scd.BufferDesc.Width = 800;
    scd.BufferDesc.Height = 600;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
        D3D11_SDK_VERSION, &scd, mSwapChain.GetAddressOf(), mDevice.GetAddressOf(), nullptr, mDeviceContext.GetAddressOf());

    // Get Back Buffer and Create Render Target View
    ComPtr<ID3D11Texture2D> backBuffer = nullptr;
    mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backBuffer.GetAddressOf());
    mDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, mRenderTargetView.GetAddressOf());

    // Set Render Target
    mDeviceContext->OMSetRenderTargets(1, mRenderTargetView.GetAddressOf(), nullptr);

    // Set Viewport
    D3D11_VIEWPORT viewport = {};
    viewport.Width = 800;
    viewport.Height = 600;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    mDeviceContext->RSSetViewports(1, &viewport);

    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

    ComPtr<ID3DBlob> vsBlob;
    ComPtr<ID3DBlob> vsErrorBlob;
    if (FAILED(D3DCompileFromFile(L"src/shaders/BasicShader.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, vsBlob.GetAddressOf(), vsErrorBlob.GetAddressOf())))
    {
        OutputDebugStringA((char*)vsErrorBlob->GetBufferPointer());
    }

    ComPtr<ID3DBlob> psBlob;
    ComPtr<ID3DBlob> psErrorBlob;
    if (FAILED(D3DCompileFromFile(L"src/shaders/BasicShader.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, psBlob.GetAddressOf(), psErrorBlob.GetAddressOf())))
    {
        OutputDebugStringA((char*)psErrorBlob->GetBufferPointer());
    }

    mDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &mVertexShader);
    mDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &mPixelShader);
    mDeviceContext->VSSetShader(mVertexShader.Get(), 0, 0);
    mDeviceContext->PSSetShader(mPixelShader.Get(), 0, 0);

    // Define input layout
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    mDevice->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &mInputLayout);
    mDeviceContext->IASetInputLayout(mInputLayout.Get());

    // Create vertex buffer
    Vertex vertices[] = {
        {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}}, // 0
        {{-0.5f, +0.5f, -0.5f}, {0.0f, 1.0f, 0.0f, 1.0f}}, // 1
        {{+0.5f, +0.5f, -0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}}, // 2
        {{+0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f, 1.0f}}, // 3
        {{-0.5f, -0.5f, +0.5f}, {1.0f, 0.0f, 1.0f, 1.0f}}, // 4
        {{-0.5f, +0.5f, +0.5f}, {0.0f, 1.0f, 1.0f, 1.0f}}, // 5
        {{+0.5f, +0.5f, +0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // 6
        {{+0.5f, -0.5f, +0.5f}, {0.0f, 0.0f, 0.0f, 1.0f}}  // 7
    };
    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = sizeof(vertices);
    D3D11_SUBRESOURCE_DATA vertexBufferData = { vertices };
    mDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, mVertexBuffer.GetAddressOf());

    // Create index buffer
    UINT16 indices[] = {
        0, 1, 2, 0, 2, 3,  // Front
        4, 6, 5, 4, 7, 6,  // Back
        4, 5, 1, 4, 1, 0,  // Left
        3, 2, 6, 3, 6, 7,  // Right
        1, 5, 6, 1, 6, 2,  // Top
        4, 0, 3, 4, 3, 7   // Bottom
    };
    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.ByteWidth = sizeof(indices);
    D3D11_SUBRESOURCE_DATA indexBufferData = { indices };
    mDevice->CreateBuffer(&indexBufferDesc, &indexBufferData, mIndexBuffer.GetAddressOf());

    // Create constant buffer
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(ConstantBuffer);
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = 0;
    cbDesc.MiscFlags = 0;
    cbDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA cbData = {};
    ConstantBuffer cbValues = {};
    cbData.pSysMem = &cbValues;

    mDevice->CreateBuffer(&cbDesc, &cbData, &mConstantBuffer);
    mDeviceContext->VSSetConstantBuffers(0, 1, mConstantBuffer.GetAddressOf());
}

void CZRenderer::Update()
{
    static float angle = 0.0f;
    angle += 0.01f; // Rotate slowly

    DirectX::XMMATRIX world = DirectX::XMMatrixRotationRollPitchYaw(angle, angle, angle);
    DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(
        DirectX::XMVectorSet(0.0f, 1.0f, -3.0f, 1.0f),  // Eye Position
        DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),   // Look-at Position
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)    // Up Vector
    );
    DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XM_PIDIV4, // 45-degree FOV
        960.0f / 540.0f,    // Aspect Ratio
        0.1f, 100.0f        // Near/Far Planes
    );

    ConstantBuffer cbValues = {};
    cbValues.world = world;
    cbValues.view = view;
    cbValues.proj = proj;

    mDeviceContext->UpdateSubresource(mConstantBuffer.Get(), 0, nullptr, &cbValues, 0, 0);
}

void CZRenderer::Render()
{
    // Clear Screen with a Color
    float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    mDeviceContext->ClearRenderTargetView(mRenderTargetView.Get(), clearColor);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    mDeviceContext->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &stride, &offset);
    mDeviceContext->IASetIndexBuffer(mIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
    mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mDeviceContext->DrawIndexed(36, 0, 0);

    // Present the Frame
    mSwapChain->Present(1, 0);
}