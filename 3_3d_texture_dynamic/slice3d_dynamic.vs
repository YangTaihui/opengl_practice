#version 330 core
layout (location = 0) in float aAxis;
layout (location = 1) in vec2 aPos;

out vec3 ourColor;
out vec3 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float id0;
uniform float id1;
uniform float id2;

void main()
{
	vec2 aTexCoord = (aPos+1)/2;
	vec4 Pos;
	if (aAxis>1.5){
		TexCoord = vec3(aTexCoord.x, 1-aTexCoord.y, id2);
		Pos = vec4(aPos.x, aPos.y, 2*id2-1, 1.0f);
	}
	else if (aAxis>0.5){
		TexCoord = vec3(aTexCoord.x, 1-id1, aTexCoord.y);
		Pos = vec4(aPos.x, 2*id1-1, aPos.y, 1.0f);
	}
	else{
		TexCoord = vec3(id0, 1-aTexCoord.x, aTexCoord.y);
		Pos = vec4(2*id0-1, aPos.x, aPos.y, 1.0f);
	}
	gl_Position = projection * view * model * Pos;
}