#pragma once

#include "CZObject.h"

#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class CZTexture : public CZObject
{
public:
	ComPtr<ID3D11ShaderResourceView> TextureSRV;

	CZTexture(ID3D11Device* device, const wchar_t* filePath);
};