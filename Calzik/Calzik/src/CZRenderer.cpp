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
        D3D11_SDK_VERSION, &scd, &mSwapChain, &mDevice, nullptr, &mDeviceContext);

    // Get Back Buffer and Create Render Target View
    ID3D11Texture2D* backBuffer = nullptr;
    mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    mDevice->CreateRenderTargetView(backBuffer, nullptr, &mRenderTargetView);
    backBuffer->Release();

    // Set Render Target
    mDeviceContext->OMSetRenderTargets(1, &mRenderTargetView, nullptr);

    // Set Viewport
    D3D11_VIEWPORT viewport = {};
    viewport.Width = 800;
    viewport.Height = 600;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    mDeviceContext->RSSetViewports(1, &viewport);

    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    CompileShader(mVertexShaderSource, "main", "vs_5_0", &vsBlob);
    CompileShader(mPixelShaderSource, "main", "ps_5_0", &psBlob);

    mDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &mVertexShader);
    mDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &mPixelShader);
    mDeviceContext->VSSetShader(mVertexShader, 0, 0);
    mDeviceContext->PSSetShader(mPixelShader, 0, 0);

    // Define input layout
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    mDevice->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &mInputLayout);
    mDeviceContext->IASetInputLayout(mInputLayout);

    vsBlob->Release();
    psBlob->Release();

    // Create vertex buffer
    Vertex vertices[] = {
        {  0.0f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f },
        {  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f },
        { -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f }
    };
    D3D11_BUFFER_DESC bd = {};
    bd.ByteWidth = sizeof(vertices);
    D3D11_SUBRESOURCE_DATA initData = { vertices };
    mDevice->CreateBuffer(&bd, &initData, &mVertexBuffer);
}

void CZRenderer::Render()
{
    // Clear Screen with a Color
    float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    mDeviceContext->ClearRenderTargetView(mRenderTargetView, clearColor);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    mDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
    mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mDeviceContext->Draw(3, 0);

    // Present the Frame
    mSwapChain->Present(1, 0);
}

HRESULT CZRenderer::CompileShader(const char* source, const char* entryPoint, const char* target, ID3DBlob** shaderBlob) {
    ID3DBlob* errorBlob = nullptr;
    HRESULT hr = D3DCompile(source, strlen(source), nullptr, nullptr, nullptr, entryPoint, target, 0, 0, shaderBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }
        return hr;
    }
    if (errorBlob) errorBlob->Release();
    return S_OK;
}