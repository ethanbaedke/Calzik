#pragma once

#include "CZRenderTargetTexture.h"
#include "CZFBXLoader.h"
#include "CZObject.h"
#include "CZMesh.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <vector>

using Microsoft::WRL::ComPtr;

class CZRenderer
{
public:
	CZRenderer(HWND hwnd);
	~CZRenderer();
	
	void Update();
	void Render();

private:
	static const UINT MAX_LIGHTS = 5;

	struct FrameConstantData
	{
		DirectX::XMVECTOR eyeWorldPosition;
		CZLight::LightData lights[MAX_LIGHTS];
	};

	struct ShadowMapConstantData
	{
		DirectX::XMVECTOR lightPos;
		DirectX::XMMATRIX viewMatrix;
		DirectX::XMMATRIX projectionMatrix;
	};

	CZFBXLoader mFBXLoader;

	ComPtr<IDXGISwapChain> mSwapChain;
	ComPtr<ID3D11Device> mDevice;
	ComPtr<ID3D11DeviceContext> mDeviceContext;
	ComPtr<ID3D11VertexShader> mVertexShader;
	ComPtr<ID3D11PixelShader> mPixelShader;
	ComPtr<ID3D11VertexShader> mShadowVertexShader;
	ComPtr<ID3D11PixelShader> mShadowPixelShader;
	ComPtr<ID3D11InputLayout> mInputLayout;
	ComPtr<ID3D11SamplerState> mSamplerState;
	ComPtr<ID3D11SamplerState> mShadowMapSamplerState;
	ComPtr<ID3D11Buffer> mFrameConstantBuffer;
	ComPtr<ID3D11Buffer> mShadowMapConstantBuffer[MAX_LIGHTS];

	CZRenderTargetTexture* mBackBufferRenderTarget;
	CZRenderTargetTexture* mShadowMapRenderTarget[MAX_LIGHTS];

	std::vector<CZMesh*> mMeshObjects;
	std::vector<CZLight*> mLightObjects;

	void SortCZObjects(std::vector<CZObject*> objs);
};