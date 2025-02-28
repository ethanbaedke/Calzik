#include "CZTexture.h"

#include <WICTextureLoader.h>

CZTexture::CZTexture(ID3D11Device* device)
{
	DirectX::CreateWICTextureFromFile(device, L"png/Crate.png", nullptr, TextureSRV.GetAddressOf());
}
