cbuffer ExternalData : register(b0)
{
    float3 colorTint;
    //int switches for like shadow map and the like go here
    //camera pos goes in here for lights
    //number of lights go in here
    //padding if needed
}

//uniforms buffer for textures goes here
//uniforms / constant buffers are used to pass data from the CPU to the GPU
//what differs is that for Texture2D we use the register keyword to specify the texture slot
//where as a cbuffer is just a buffer of data not a reference to a texture resource
//Texture2D SurfaceTexture : register(t0);


struct VertexToPixel
{
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

//output struct //needs lighting shadows emission gamma correction etc
float4 main(VertexToPixel input) : SV_TARGET
{
    return float4(colorTint, 1);
}