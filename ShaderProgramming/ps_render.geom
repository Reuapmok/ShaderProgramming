#version 430

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

in vec3 Position0[];
in float Type0[];

uniform mat4 projection;
uniform mat4 view;
uniform vec3 cameraPos;
//uniform vec3 lightPos;
//uniform vec3 lightPos;

out vec2 TexCoords;
out float Type1;

#define PARTICLE_TYPE_SPAWNER	1.0f
#define PARTICLE_TYPE_NORMAL	0.0f
#define PARTICLE_TYPE_SNOW		2.0f


void main()
{
    
	float size;
	Type1 = Type0[0];

	// PARTICLE LAUNCHER 
	//
	// NO SPAWNER?
   if(Type0[0] != PARTICLE_TYPE_SPAWNER)
   {
	
	if(Type0[0] == PARTICLE_TYPE_SNOW)
		size = 0.05f;
	else
		size = 0.03f;

	vec4 position = projection * view * vec4(Position0[0] ,1.0f) ;

	gl_Position = position + vec4(-size, -size, 0.0, 0.0);    // 1:bottom-left
	TexCoords = vec2(0,1);
	EmitVertex();   
	
	gl_Position = position + vec4( size, -size, 0.0, 0.0);    // 2:bottom-right
	TexCoords = vec2(1,1);
	EmitVertex();
	
	gl_Position = position + vec4(-size,  size, 0.0, 0.0);    // 3:top-left
	TexCoords = vec2(0,0);
	EmitVertex();
	
	gl_Position = position + vec4( size,  size, 0.0, 0.0);    // 4:top-right
	TexCoords = vec2(1,0);
	EmitVertex(); 
	
	EndPrimitive();

	}

} 