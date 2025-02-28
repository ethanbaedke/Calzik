#include "CZMesh.h"

CZMesh::CZMesh(ID3D11Device* device, std::vector<Vertex> vertexList, std::vector<UINT> indexList, CZTexture* texture)
    : Texture(texture)
{
    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = sizeof(Vertex) * vertexList.size();
    D3D11_SUBRESOURCE_DATA vertexBufferData = { vertexList.data() };
    device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, VertexBuffer.GetAddressOf());

    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.ByteWidth = sizeof(UINT) * indexList.size();
    D3D11_SUBRESOURCE_DATA indexBufferData = { indexList.data() };
    device->CreateBuffer(&indexBufferDesc, &indexBufferData, IndexBuffer.GetAddressOf());

    IndexCount = indexList.size();
}
