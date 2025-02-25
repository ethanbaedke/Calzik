#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class CZRenderer
{
public:
	CZRenderer(HWND hwnd);
	
	void Render();

private:
	ComPtr<IDXGISwapChain> mSwapChain;
	ComPtr<ID3D11Device> mDevice;
	ComPtr<ID3D11DeviceContext> mDeviceContext;
	ComPtr<ID3D11RenderTargetView> mRenderTargetView;
	ComPtr<ID3D11Buffer> mVertexBuffer = nullptr;
	ComPtr<ID3D11Buffer> mIndexBuffer = nullptr;
	ComPtr<ID3D11VertexShader> mVertexShader = nullptr;
	ComPtr<ID3D11PixelShader> mPixelShader = nullptr;
	ComPtr<ID3D11InputLayout> mInputLayout = nullptr;

	struct Vertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT4 Color;
	};
};