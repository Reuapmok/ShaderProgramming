#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

//out vec4 pos;

void main()
{
	// render position from light-space
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0f);
	//pos = lightSpaceMatrix * model * vec4(aPos, 1.0);
}