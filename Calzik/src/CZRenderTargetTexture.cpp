#include "CZRenderTargetTexture.h"

CZRenderTargetTexture::CZRenderTargetTexture(ID3D11Device* device, UINT width, UINT height, ID3D11Texture2D* texture)
{
	// Create the texture if none is input
	if (texture == nullptr)
	{
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		device->CreateTexture2D(&textureDesc, nullptr, mTargetTexture.GetAddressOf());
	}
	// Use the input texture
	else
	{
		mTargetTexture = texture;
	}

	// Create render target view
	device->CreateRenderTargetView(mTargetTexture.Get(), nullptr, mRenderTargetView.GetAddressOf());

	// Create the depth stencil texture
	D3D11_TEXTURE2D_DESC depthBufferDesc = {};
	depthBufferDesc.Width = width;
	depthBufferDesc.Height = height;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 24-bit depth, 8-bit stencil
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	device->CreateTexture2D(&depthBufferDesc, nullptr, mDepthStencilTexture.GetAddressOf());
	device->CreateDepthStencilView(mDepthStencilTexture.Get(), nullptr, mDepthStencilView.GetAddressOf());

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS; // Closer pixels pass

	device->CreateDepthStencilState(&depthStencilDesc, mDepthStencilState.GetAddressOf());

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;	

	device->CreateShaderResourceView(mTargetTexture.Get(), &srvDesc, ShaderResourceView.GetAddressOf());

	mViewport = {};
	mViewport.Width = width;
	mViewport.Height = height;
	mViewport.MinDepth = 0.0f;
	mViewport.MaxDepth = 1.0f;
}

void CZRenderTargetTexture::Clear(ID3D11DeviceContext* context, float clearColor[4])
{
	context->ClearRenderTargetView(mRenderTargetView.Get(), clearColor);
	context->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void CZRenderTargetTexture::BindAsTarget(ID3D11DeviceContext* context)
{
	context->OMSetDepthStencilState(mDepthStencilState.Get(), 0);
	context->OMSetRenderTargets(1, mRenderTargetView.GetAddressOf(), mDepthStencilView.Get());
	context->RSSetViewports(1, &mViewport);
}
