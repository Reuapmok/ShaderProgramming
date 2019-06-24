#version 330 core


layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec3 texCoord;

//uniform mat4 projection;
//uniform mat4 view;
//uniform mat4 model;
out vec3 Pos;

void main()
{
	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
	//gl_Position = projection * view * model * vec4(aPos, 1.0);


	Pos = aPos;
}
