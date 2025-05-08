uniform float3 iResolution;
uniform float iTime;
uniform float iTimeDelta;
uniform float iFrameRate;
uniform int iFrame;

float4 main(float2 fragCoord: TEXCOORD) : SV_TARGET0
{
    return float4(fragCoord.x, fragCoord.y, cos(iTime), 1.0);
}
