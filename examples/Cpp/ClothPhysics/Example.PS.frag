// GLSL Fragment Shader "PS"
// Generated by XShaderCompiler
// 11/10/2019 20:18:28

#version 140

#if GL_ES
precision mediump float;
precision mediump sampler2D;
#endif

in vec4 xsv_NORMAL0;
in vec2 xsv_TEXCOORD0;

out vec4 SV_Target0;

layout(std140, row_major) uniform SceneState
{
    mat4  wvpMatrix;
    mat4  wMatrix;
    vec4  gravity;
    uvec2 gridSize;
    uvec2 _pad0;
    float damping;
    float dTime;
    float dStiffness;
    float _pad1;
    vec4  lightVec;
};

uniform sampler2D colorMap;

void main()
{
    // Compute lighting
    vec3 normal = normalize(xsv_NORMAL0.xyz);
    normal *= vec3(mix(1.0, -1.0, float(gl_FrontFacing)));
    float NdotL = mix(0.2, 1.0, max(0.0, dot(normal, -lightVec.xyz)));
    
    // Sample color texture
    vec4 color = texture(colorMap, xsv_TEXCOORD0);
    color.rgb = mix(color.rgb, vec3(xsv_TEXCOORD0, 1.0), vec3(0.5));
    SV_Target0 = vec4(color.rgb * NdotL, color.a);
}

