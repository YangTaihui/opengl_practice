#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture samplers
uniform sampler2D texture1;
uniform sampler2D text1;
uniform vec3 textColor;
uniform bool istext;
void main()
{
	//FragColor = texture(texture1, TexCoord) + vec4(textColor, texture(text1, TexCoord).r);
	if (istext)
		FragColor = vec4(textColor, texture(text1, TexCoord).r);
	else
		FragColor = texture(texture1, TexCoord);
}