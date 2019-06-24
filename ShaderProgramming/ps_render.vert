#version 330

layout (location = 0) in float Type;
layout (location = 1) in vec3 Position;

out vec3 Position0;
out float Type0;

// PASS THROUGH VERTEX SHADER
void main()
{
    Position0 = Position;
	Type0 = Type;

} 