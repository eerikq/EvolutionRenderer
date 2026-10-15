struct VSInput {
    float3 inPosition;
    float3 inColor;
    float2 inTexCoord;
};

struct UniformBuffer {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};

struct VSOutput {
    float4 pos : SV_Position;
    float3 color;
    float2 texCoord;
};

ConstantBuffer<UniformBuffer> ubo;
Sampler2D texture;

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;

    float4x4 mvp = mul(ubo.proj, mul(ubo.view, ubo.model));
    output.pos = mul(mvp, float4(input.inPosition, 1.0));
    output.color = input.inColor;
    output.texCoord = input.inTexCoord;

    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {
    return texture.Sample(vertIn.texCoord);
}
