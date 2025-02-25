#pragma once

#include <d3d11.h>

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
};