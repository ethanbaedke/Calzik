#include "CZTexture.h"

#include <WICTextureLoader.h>

CZTexture::CZTexture(ID3D11Device* device, const wchar_t* filePath)
{
	DirectX::CreateWICTextureFromFile(device, filePath, nullptr, TextureSRV.GetAddressOf());
}
