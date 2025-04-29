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
		UINT lightType = 0;
		DirectX::XMVECTOR position;
		DirectX::XMVECTOR color;
		DirectX::XMMATRIX viewMatrix;
		DirectX::XMMATRIX projectionMatrix;
	};

	LightData LightingData;

	CZLight(DirectX::XMVECTOR position, DirectX::XMVECTOR color);
	CZLight(DirectX::XMVECTOR position, DirectX::XMVECTOR color, DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix);
};