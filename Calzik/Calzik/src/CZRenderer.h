#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>

class CZRenderer
{
public:
	CZRenderer(HWND hwnd);
	
	void Render();

private:
	IDXGISwapChain* mSwapChain;
	ID3D11Device* mDevice;
	ID3D11DeviceContext* mDeviceContext;
	ID3D11RenderTargetView* mRenderTargetView;
	ID3D11Buffer* mVertexBuffer = nullptr;
	ID3D11VertexShader* mVertexShader = nullptr;
	ID3D11PixelShader* mPixelShader = nullptr;
	ID3D11InputLayout* mInputLayout = nullptr;

	struct Vertex
	{
		float x, y, z;
		float r, g, b;
	};
};