#version 430 core

layout (triangles, fractional_odd_spacing, cw) in;

uniform sampler2D terrainHeightmap;


void main(void){ 


	vec4 newPos = vec4(gl_TessCoord.x * gl_in[0].gl_Position + 
		gl_TessCoord.y * gl_in[1].gl_Position +
		gl_TessCoord.z * gl_in[2].gl_Position);

	vec3 height = texture(terrainHeightmap, (newPos.xy+1)*0.5f).rgb;

	newPos.z = newPos.z + height.g*0.3;
	//gl_Position = 
	//(
	//	gl_TessCoord.x * gl_in[0].gl_Position + 
	//	gl_TessCoord.y * gl_in[1].gl_Position +
	//	gl_TessCoord.z * gl_in[2].gl_Position
	//);
	gl_Position = newPos;
	//pos = newPos;
}

