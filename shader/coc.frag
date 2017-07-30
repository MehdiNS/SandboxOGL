#version 420

in block
{
	vec2 Texcoord;
} In; 

uniform sampler2D Texture;
uniform vec3 Focus;

layout (binding = 2, std140) uniform matrix_data {
    mat4 projectionMatrix;
    mat4 viewMatrix;
	mat4 viewProjInvMatrix;
    mat4 normalMatrix;
    vec4 cameraPos;
    vec4 sunDir;
	vec4 skyData;
};

layout(location = 0, index = 0) out vec4 Color;

void main(void)
{
	mat4 ScreenToView = inverse(projectionMatrix);
    float depth = texture(Texture, inTexCoord).r;
    vec2  xy = inTexCoord * 2.0 -1.0;
    vec4  wViewPos =  ScreenToView * vec4(xy, depth * 2.0 -1.0, 1.0);
    vec3  viewPos = vec3(wViewPos/wViewPos.w);
    float viewDepth = -viewPos.z;
	vec3 focus = vec3(10.f, 1.1f, 1000.f);
    if( viewDepth < focus.x )
        Color = vec4( clamp( abs( (viewDepth - focus.x) / focus.y ), 0.0, 1.0), 0.0, 0.0, 1.0 );
    else
        Color = vec4( clamp( abs( (viewDepth - focus.x) / focus.z ), 0.0, 1.0), 0.0, 0.0, 1.0 );
}