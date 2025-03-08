#pragma once

#include "d3d11.h"
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class CZRenderTargetTexture
{
public:
	CZRenderTargetTexture(ID3D11Device* device, UINT width, UINT height, ID3D11Texture2D* texture);

	void Clear(ID3D11DeviceContext* context, float clearColor[4]);
	void BindAsTarget(ID3D11DeviceContext* context);

	ComPtr<ID3D11ShaderResourceView> ShaderResourceView;

private:
	ComPtr<ID3D11Texture2D> mTargetTexture;
	ComPtr<ID3D11RenderTargetView> mRenderTargetView;
	ComPtr<ID3D11Texture2D> mDepthStencilTexture;
	ComPtr<ID3D11DepthStencilView> mDepthStencilView;
	ComPtr<ID3D11DepthStencilState> mDepthStencilState;
	D3D11_VIEWPORT mViewport;
};