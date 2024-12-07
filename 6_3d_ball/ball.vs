#version 330 core
#define PI 3.1416

layout (location = 0) in vec2 uv;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
vec3 aPos;

void main()
{
	aPos.z = cos(uv.x)*cos(uv.y);
	aPos.x = cos(uv.x)*sin(uv.y);
	aPos.y = sin(uv.x);
	TexCoord = vec2(0.5*uv.y/PI, uv.x/PI + 0.5);
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
}