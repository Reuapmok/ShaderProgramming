#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec4 pos;
out vec2 texCoords;
out vec3 normal;

vec3 CalculateNormal(vec3 p1, vec3 p2, vec3 p3)
{

	vec3 normal;

	vec3 u = p2 - p1;
	vec3 v = p3 - p1;

	normal.x = (u.y * v.z) - (u.z * v.y);
	normal.y = (u.z * v.x) - (u.x * v.z);
	normal.z = (u.x * v.y) - (u.y * v.x);

	return normalize(normal);
}

void main() {    

	

	normal = CalculateNormal(gl_in[0].gl_Position.xyz,gl_in[1].gl_Position.xyz,gl_in[2].gl_Position.xyz);
	texCoords = (gl_in[0].gl_Position.xy + 1.0f) * 0.5f;
	pos = projection * view * model * gl_in[0].gl_Position;
    gl_Position = pos; 
    EmitVertex();

	normal = CalculateNormal(gl_in[0].gl_Position.xyz,gl_in[1].gl_Position.xyz,gl_in[2].gl_Position.xyz);
	texCoords = (gl_in[1].gl_Position.xy + 1.0f) * 0.5f;
	pos = projection * view * model * gl_in[1].gl_Position;
    gl_Position = pos; 
    EmitVertex();

	normal = CalculateNormal(gl_in[0].gl_Position.xyz,gl_in[1].gl_Position.xyz,gl_in[2].gl_Position.xyz);
	texCoords = (gl_in[2].gl_Position.xy + 1.0f) * 0.5f;
	pos = projection * view * model * gl_in[2].gl_Position;
    gl_Position = pos; 
    EmitVertex();

    EndPrimitive();
}  