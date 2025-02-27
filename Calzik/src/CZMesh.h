#pragma once

#include "CZObject.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <vector>

using Microsoft::WRL::ComPtr;

class CZMesh : public CZObject
{
public:
	struct Vertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
	};

	ComPtr<ID3D11Buffer> VertexBuffer;
	ComPtr<ID3D11Buffer> IndexBuffer;

	CZMesh(const std::vector<float>& vertexPositions, const std::vector<UINT>& indices, ID3D11Device* device);
};