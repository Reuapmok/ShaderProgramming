#version 410 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

//out VS_OUT {
//	vec2 texCoords;
//} vs_out;

out vec2 TexCoords;

void main()
{
    //vs_out.texCoords = aPos;
	TexCoords = aPos;
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
}  