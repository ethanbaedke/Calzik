#include "CZRenderer.h"

#include "CZTexture.h"
#include "CZMesh.h"

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
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    mDevice->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &mInputLayout);
    mDeviceContext->IASetInputLayout(mInputLayout.Get());

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

    // Create the texture sampler state
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    
    mDevice->CreateSamplerState(&sampDesc, mSamplerState.GetAddressOf());
    mDeviceContext->PSSetSamplers(0, 1, mSamplerState.GetAddressOf());

    // Load cube object
    SortCZObjects(mFBXLoader.LoadFBXFile("fbx/Cube.fbx", mDevice.Get()));
}

void CZRenderer::Update()
{
    static float angle = 0.0f;
    angle += 0.01f; // Rotate slowly

    DirectX::XMMATRIX world = DirectX::XMMatrixRotationRollPitchYaw(angle, angle, angle);
    DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(
        DirectX::XMVectorSet(0.0f, 1.0f, -5.0f, 1.0f),  // Eye Position
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

    UINT stride = sizeof(CZMesh::Vertex);
    UINT offset = 0;
    mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for (int i = 0; i < mMeshObjects.size(); i++)
    {
        // Bind the meshes texture to the pipeline
        mDeviceContext->PSSetShaderResources(0, 1, mMeshObjects[i]->Texture->TextureSRV.GetAddressOf());

        // Bind the meshes vertex and index buffers to the pipeline
        mDeviceContext->IASetVertexBuffers(0, 1, mMeshObjects[i]->VertexBuffer.GetAddressOf(), & stride, & offset);
        mDeviceContext->IASetIndexBuffer(mMeshObjects[i]->IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        mDeviceContext->DrawIndexed(mMeshObjects[i]->IndexCount, 0, 0);
    }

    // Present the Frame
    mSwapChain->Present(1, 0);
}

void CZRenderer::SortCZObjects(std::vector<CZObject*> objs)
{
    for (int i = 0; i < objs.size(); i++)
    {
        if (CZMesh* mesh = dynamic_cast<CZMesh*>(objs[i]))
            mMeshObjects.push_back(mesh);
    }
}
