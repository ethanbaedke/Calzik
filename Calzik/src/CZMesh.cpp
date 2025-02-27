#include "CZMesh.h"

CZMesh::CZMesh(const std::vector<float>& vertexPositions, const std::vector<UINT>& indices, ID3D11Device* device)
{
    std::vector<Vertex> vertices = std::vector<Vertex>(vertexPositions.size() / 3);
    for (int i = 0; i < vertices.size(); i++)
    {
        int vpBaseInd = i * 3;
        vertices[i] = {
            {vertexPositions[vpBaseInd], vertexPositions[vpBaseInd + 1], vertexPositions[vpBaseInd + 2]},
            {1.0f, 1.0f, 1.0f, 1.0f}
        };
    }

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = sizeof(Vertex) * vertices.size();
    D3D11_SUBRESOURCE_DATA vertexBufferData = { vertices.data() };
    device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, VertexBuffer.GetAddressOf());

    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.ByteWidth = sizeof(UINT) * indices.size();
    D3D11_SUBRESOURCE_DATA indexBufferData = { indices.data() };
    device->CreateBuffer(&indexBufferDesc, &indexBufferData, IndexBuffer.GetAddressOf());
}
