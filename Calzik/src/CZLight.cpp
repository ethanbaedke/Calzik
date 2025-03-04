#include "CZLight.h"

CZLight::CZLight(DirectX::XMVECTOR position, DirectX::XMVECTOR color)
{
    LightingData.position = position;
    LightingData.color = color;
}
