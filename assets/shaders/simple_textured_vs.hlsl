cbuffer SceneConstantBuffer : register(b0)
{
	float4 offset;
	float4 padding[15];
}

struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

PSInput main(float4 position : POSITION, float4 uv : TEXCOORD)
{
	PSInput result;

	result.position = position + offset;
	result.uv = uv;

	return result;
}