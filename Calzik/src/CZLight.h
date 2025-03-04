#pragma once

#include "CZObject.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class CZLight : public CZObject
{
public:
	struct LightData
	{
		DirectX::XMVECTOR position;
		DirectX::XMVECTOR color;
	};

	LightData LightingData;

	CZLight(DirectX::XMVECTOR position, DirectX::XMVECTOR color);
};