float4x4 ViewProjectionMatrix;

float tiempo;

struct VsInput
{
	float4 position : POSITION;
	float3 normal : NORMAL;
	float2 uv0 : TEXCOORD0;
};

struct VsOutput
{
	float2 uv0 : TEXCOORD0;
	float4 position : POSITION;
};

VsOutput vsMain(const VsInput In) 
{
	VsOutput vsOut;
	float4 pos = In.position;
	float random = clamp(tiempo, 2, 5);
	pos.y += (sin(pos.y+tiempo*random)) * 1.4;
	vsOut.position = mul(ViewProjectionMatrix, pos);
	vsOut.uv0 = In.uv0;
	return vsOut;
}


float4 mainPS() : COLOR 
{
	return float4(0, 0.0, 0.0, 0.0);
}
