#include "CZMesh.h"

CZMesh::CZMesh(ID3D11Device* device, std::vector<Vertex> vertexList, std::vector<UINT> indexList, CZTexture* diffuseTexture, CZTexture* normalTexture, DirectX::XMMATRIX worldMatrix)
    : WorldMatrix(worldMatrix), DiffuseTexture(diffuseTexture), NormalTexture(normalTexture)
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

    // Create mesh constant buffer
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(RenderItemConstantData);
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = 0;
    cbDesc.MiscFlags = 0;
    cbDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA cbData = {};
    RenderItemConstantData cbValues = {};
    cbData.pSysMem = &cbValues;

    device->CreateBuffer(&cbDesc, &cbData, &ConstantBuffer);
}
