struct VsOut
{
    float2 texC : TEXCOORD;
#ifndef _VIEWPORT_MASK
    float4 posH : SV_POSITION;
#else
    float4 posH : POSITION;
#endif
};

VsOut main(float4 posS: POSITION, float2 texC: TEXCOORD)
{
    VsOut vOut;
    vOut.texC = float2(texC.x, 1.0 - texC.y);
    vOut.posH = posS;
    return vOut;
}
