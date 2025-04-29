#include "CZLight.h"

CZLight::CZLight(DirectX::XMVECTOR position, DirectX::XMVECTOR color)
{
    LightingData.lightType = 0;
    LightingData.position = position;
    LightingData.color = color;
}

CZLight::CZLight(DirectX::XMVECTOR position, DirectX::XMVECTOR color, DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix)
{
    LightingData.lightType = 1;
    LightingData.position = position;
    LightingData.color = color;
    LightingData.viewMatrix = viewMatrix;
    LightingData.projectionMatrix = projectionMatrix;
}
