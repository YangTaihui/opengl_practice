#version 330 core
layout (location = 0) in vec3 aPos;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	TexCoord = vec2(0.5*(aPos.x+1), 0.5*(aPos.z+1));
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
}