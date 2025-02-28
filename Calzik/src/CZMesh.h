#pragma once

#include "CZObject.h"
#include "CZTexture.h"

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
		DirectX::XMFLOAT2 UV;
	};

	CZTexture* Texture;

	ComPtr<ID3D11Buffer> VertexBuffer;
	ComPtr<ID3D11Buffer> IndexBuffer;
	UINT IndexCount;

	CZMesh(ID3D11Device* device, std::vector<Vertex> vertexList, std::vector<UINT> indexList, CZTexture* texture);
};