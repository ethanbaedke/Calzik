#include "CZRenderer.h"

#include "CZTexture.h"
#include "CZMesh.h"

CZRenderer::CZRenderer(HWND hwnd)
{
    // Create swap chain
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

    // Get the back buffer texture
    ComPtr<ID3D11Texture2D> backBuffer = nullptr;
    mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backBuffer.GetAddressOf());

    // Create a render target texture using it
    mBackBufferRenderTarget = new CZRenderTargetTexture(mDevice.Get(), 800, 600, backBuffer.Get());

    // Create the shadow map target texture
    for (int i = 0; i < 5; i++)
    {
        mShadowMapRenderTarget[i] = new CZRenderTargetTexture(mDevice.Get(), 2048, 2048, nullptr);
    }

    // Setup the frame constant buffer
    D3D11_BUFFER_DESC fcbDesc = {};
    fcbDesc.ByteWidth = sizeof(FrameConstantData);
    fcbDesc.Usage = D3D11_USAGE_DEFAULT;
    fcbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    fcbDesc.CPUAccessFlags = 0;
    fcbDesc.MiscFlags = 0;
    fcbDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA fcbData = {};
    FrameConstantData fcbValues = {};
    fcbData.pSysMem = &fcbValues;

    mDevice->CreateBuffer(&fcbDesc, &fcbData, &mFrameConstantBuffer);

    // Setup the shadow map constant buffer
    D3D11_BUFFER_DESC smcbDesc = {};
    smcbDesc.ByteWidth = sizeof(ShadowMapConstantData);
    smcbDesc.Usage = D3D11_USAGE_DEFAULT;
    smcbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    smcbDesc.CPUAccessFlags = 0;
    smcbDesc.MiscFlags = 0;
    smcbDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA smcbData = {};
    FrameConstantData smcbValues = {};
    smcbData.pSysMem = &smcbValues;

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        mDevice->CreateBuffer(&smcbDesc, &smcbData, &mShadowMapConstantBuffer[i]);
    }

    // Compile shaders
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

    ComPtr<ID3DBlob> vsShadowBlob;
    ComPtr<ID3DBlob> vsShadowErrorBlob;
    if (FAILED(D3DCompileFromFile(L"src/shaders/ShadowMap.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, vsShadowBlob.GetAddressOf(), vsShadowErrorBlob.GetAddressOf())))
    {
        OutputDebugStringA((char*)vsShadowErrorBlob->GetBufferPointer());
    }

    ComPtr<ID3DBlob> psShadowBlob;
    ComPtr<ID3DBlob> psShadowErrorBlob;
    if (FAILED(D3DCompileFromFile(L"src/shaders/ShadowMap.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, psShadowBlob.GetAddressOf(), psShadowErrorBlob.GetAddressOf())))
    {
        OutputDebugStringA((char*)psShadowErrorBlob->GetBufferPointer());
    }

    mDevice->CreateVertexShader(vsShadowBlob->GetBufferPointer(), vsShadowBlob->GetBufferSize(), nullptr, &mShadowVertexShader);
    mDevice->CreatePixelShader(psShadowBlob->GetBufferPointer(), psShadowBlob->GetBufferSize(), nullptr, &mShadowPixelShader);

    // Define input layout
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    mDevice->CreateInputLayout(layout, 4, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &mInputLayout);
    mDeviceContext->IASetInputLayout(mInputLayout.Get());

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

    // Create the shadow texture sampler state
    D3D11_SAMPLER_DESC shadowSampDesc = {};
    shadowSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    shadowSampDesc.MinLOD = 0;
    shadowSampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    mDevice->CreateSamplerState(&shadowSampDesc, mShadowMapSamplerState.GetAddressOf());
    mDeviceContext->PSSetSamplers(1, 1, mShadowMapSamplerState.GetAddressOf());

    // Load cube object
    SortCZObjects(mFBXLoader.LoadFBXFile("fbx/CubeFloor.fbx", mDevice.Get()));
}

CZRenderer::~CZRenderer()
{
    delete mBackBufferRenderTarget;
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        delete mShadowMapRenderTarget[i];
    }
}

void CZRenderer::Update()
{
    static float angle = 0.0f;
    static float camAngle = 0.0f;
    //angle += 0.01f; // Rotate slowly
    angle += 0.00f; // No rotation
    camAngle += 0.01f; // Rotate slowly
    //camAngle += 0.00f; // No rotation

    DirectX::XMVECTOR eyeWorldPosition = DirectX::XMVectorSet(DirectX::XMScalarSin(camAngle) * 20, 20.0f, DirectX::XMScalarCos(camAngle) * 20, 1.0f);
    //DirectX::XMVECTOR eyeWorldPosition = DirectX::XMVectorSet(0.0f, 0.0f, -20.0f, 1.0f);
    //DirectX::XMVECTOR eyeWorldPosition = DirectX::XMVectorSet(0.0f, 20.0f, -20.0f, 1.0f);

    FrameConstantData fcdValues = {};
    fcdValues.eyeWorldPosition = eyeWorldPosition;

    // Update light data
    int lightInd = 0;
    while (lightInd < MAX_LIGHTS && lightInd < mLightObjects.size())
    {
        fcdValues.lights[lightInd] = mLightObjects[lightInd]->LightingData;

        ShadowMapConstantData smcd = {};
        smcd.lightPos = mLightObjects[lightInd]->LightingData.position;
        smcd.viewMatrix = mLightObjects[lightInd]->LightingData.viewMatrix;
        smcd.projectionMatrix = mLightObjects[lightInd]->LightingData.projectionMatrix;
        mDeviceContext->UpdateSubresource(mShadowMapConstantBuffer[lightInd].Get(), 0, nullptr, &smcd, 0, 0);

        lightInd++;
    }
    mDeviceContext->UpdateSubresource(mFrameConstantBuffer.Get(), 0, nullptr, &fcdValues, 0, 0);

    DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(
        eyeWorldPosition,  // Eye Position
        DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),   // Look-at Position
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)    // Up Vector
    );
    DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XM_PIDIV4, // 45-degree FOV
        960.0f / 540.0f,    // Aspect Ratio
        0.1f, 100.0f        // Near/Far Planes
    );

    for (int i = 0; i < mMeshObjects.size(); i++)
    {
        DirectX::XMMATRIX world = DirectX::XMMatrixRotationRollPitchYaw(angle, angle, angle) * mMeshObjects[i]->WorldMatrix;

        CZMesh::RenderItemConstantData ricbValues = {};
        ricbValues.world = world;
        ricbValues.view = view;
        ricbValues.proj = proj;

        if (mMeshObjects[i]->DiffuseTexture != nullptr)
            ricbValues.diffTexFlag = 1;
        if (mMeshObjects[i]->NormalTexture != nullptr)
            ricbValues.normTexFlag = 1;

        mDeviceContext->UpdateSubresource(mMeshObjects[i]->ConstantBuffer.Get(), 0, nullptr, &ricbValues, 0, 0);
    }
}

void CZRenderer::Render()
{
    // Clear Screen with a Color
    float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    mBackBufferRenderTarget->Clear(mDeviceContext.Get(), clearColor);
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        mShadowMapRenderTarget[i]->Clear(mDeviceContext.Get(), clearColor);
    }

    UINT stride = sizeof(CZMesh::Vertex);
    UINT offset = 0;
    mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Set the frame constant buffer
    mDeviceContext->VSSetConstantBuffers(1, 1, mFrameConstantBuffer.GetAddressOf());
    mDeviceContext->PSSetConstantBuffers(1, 1, mFrameConstantBuffer.GetAddressOf());

    // Render to shadow maps
    mDeviceContext->VSSetShader(mShadowVertexShader.Get(), 0, 0);
    mDeviceContext->PSSetShader(mShadowPixelShader.Get(), 0, 0);
    for (int f = 0; f < mLightObjects.size(); f++)
    {
        mShadowMapRenderTarget[f]->BindAsTarget(mDeviceContext.Get());
        mDeviceContext->VSSetConstantBuffers(2, 1, mShadowMapConstantBuffer[f].GetAddressOf());
        mDeviceContext->PSSetConstantBuffers(2, 1, mShadowMapConstantBuffer[f].GetAddressOf());

        for (int i = 0; i < mMeshObjects.size(); i++)
        {
            // Bind the meshes constant buffer to the vertex and pixel shader stages (pixel shader for flags)
            mDeviceContext->VSSetConstantBuffers(0, 1, mMeshObjects[i]->ConstantBuffer.GetAddressOf());
            mDeviceContext->PSSetConstantBuffers(0, 1, mMeshObjects[i]->ConstantBuffer.GetAddressOf());

            // Bind the meshes vertex and index buffers to the pipeline
            mDeviceContext->IASetVertexBuffers(0, 1, mMeshObjects[i]->VertexBuffer.GetAddressOf(), &stride, &offset);
            mDeviceContext->IASetIndexBuffer(mMeshObjects[i]->IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
            mDeviceContext->DrawIndexed(mMeshObjects[i]->IndexCount, 0, 0);
        }
    }

    // Render to back buffer
    mBackBufferRenderTarget->BindAsTarget(mDeviceContext.Get());
    mDeviceContext->VSSetShader(mVertexShader.Get(), 0, 0);
    mDeviceContext->PSSetShader(mPixelShader.Get(), 0, 0);

    mDeviceContext->PSSetShaderResources(2, 1, mShadowMapRenderTarget[0]->ShaderResourceView.GetAddressOf());

    for (int i = 0; i < mMeshObjects.size(); i++)
    {
        // Bind any existing textures on the mesh to the pipeline
        if (mMeshObjects[i]->DiffuseTexture != nullptr)
        {
            //mDeviceContext->PSSetShaderResources(0, 1, mTestSecondRenderTarget->ShaderResourceView.GetAddressOf());
            mDeviceContext->PSSetShaderResources(0, 1, mMeshObjects[i]->DiffuseTexture->TextureSRV.GetAddressOf());
        }
        if (mMeshObjects[i]->NormalTexture != nullptr)
        {
            mDeviceContext->PSSetShaderResources(1, 1, mMeshObjects[i]->NormalTexture->TextureSRV.GetAddressOf());
        }

        // Bind the meshes constant buffer to the vertex and pixel shader stages (pixel shader for flags)
        mDeviceContext->VSSetConstantBuffers(0, 1, mMeshObjects[i]->ConstantBuffer.GetAddressOf());
        mDeviceContext->PSSetConstantBuffers(0, 1, mMeshObjects[i]->ConstantBuffer.GetAddressOf());

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
        else if (CZLight* light = dynamic_cast<CZLight*>(objs[i]))
            mLightObjects.push_back(light);
    }
}
