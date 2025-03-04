#pragma once

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
	
	void Update();
	void Render();

private:
	static const UINT MAX_LIGHTS = 5;

	struct FrameConstantData
	{
		DirectX::XMVECTOR eyeWorldPosition;
		CZLight::LightData lights[MAX_LIGHTS];
	};

	CZFBXLoader mFBXLoader;

	ComPtr<IDXGISwapChain> mSwapChain;
	ComPtr<ID3D11Device> mDevice;
	ComPtr<ID3D11DeviceContext> mDeviceContext;
	ComPtr<ID3D11RenderTargetView> mRenderTargetView;
	ComPtr<ID3D11Texture2D> mDepthStencilBuffer;
	ComPtr<ID3D11DepthStencilView> mDepthStencilView;
	ComPtr<ID3D11DepthStencilState> mDepthStencilState;
	ComPtr<ID3D11VertexShader> mVertexShader;
	ComPtr<ID3D11PixelShader> mPixelShader;
	ComPtr<ID3D11InputLayout> mInputLayout;
	ComPtr<ID3D11SamplerState> mSamplerState;
	ComPtr<ID3D11Buffer> mFrameConstantBuffer;

	std::vector<CZMesh*> mMeshObjects;
	std::vector<CZLight*> mLightObjects;

	void SortCZObjects(std::vector<CZObject*> objs);
};