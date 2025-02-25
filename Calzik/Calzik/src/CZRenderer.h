#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>

class CZRenderer
{
public:
	CZRenderer(HWND hwnd);
	
	void Render();

private:
	IDXGISwapChain* mSwapChain;
	ID3D11Device* mDevice;
	ID3D11DeviceContext* mDeviceContext;
	ID3D11RenderTargetView* mRenderTargetView;
	ID3D11Buffer* mVertexBuffer = nullptr;
	ID3D11VertexShader* mVertexShader = nullptr;
	ID3D11PixelShader* mPixelShader = nullptr;
	ID3D11InputLayout* mInputLayout = nullptr;

	struct Vertex
	{
		float x, y, z;
		float r, g, b;
	};

	// Vertex Shader (HLSL)
	const char* mVertexShaderSource = R"(
    struct VS_INPUT {
        float3 pos : POSITION;
        float3 color : COLOR;
    };
    struct VS_OUTPUT {
        float4 pos : SV_POSITION;
        float4 color : COLOR;
    };
    VS_OUTPUT main(VS_INPUT input) {
        VS_OUTPUT output;
        output.pos = float4(input.pos, 1.0);
        output.color = float4(input.color, 1.0);
        return output;
    }
    )";

    // Pixel Shader (HLSL)
    const char* mPixelShaderSource = R"(
    struct PS_INPUT {
        float4 pos : SV_POSITION;
        float4 color : COLOR;
    };
    float4 main(PS_INPUT input) : SV_TARGET {
        return input.color;
    }
    )";

    HRESULT CompileShader(const char* source, const char* entryPoint, const char* target, ID3DBlob** shaderBlob);
};