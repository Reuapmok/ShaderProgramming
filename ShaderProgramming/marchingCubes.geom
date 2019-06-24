#version 420 core


//	void main( 
//	inoutTriangleStream<GSOutput> Stream, 
//	pointvsOutputGsInputinput[1] )

layout(points) in;						// INPUT TYPE
//layout(points, max_vertices = 15) out;	// OUTPUT TYPE		TESTING

// INPUT TYPE
layout(triangle_strip, max_vertices = 15) out;



// TESTING
in float[] geoValue;
//out float testValue;
out float outValue;

flat in int[] layerId;

flat out int layerID;

// GEOMETRY SHADER INPUT
in struct VS_OUT {
	vec4 wsCoord;	//coords are for the LOWER-LEFT corner of the cell.
	vec3 uvw;		//coords are for the LOWER-LEFT corner of the cell.
	vec4 f0123;
	vec4 f4567;
	uint mc_case;
} vs_out[];

// GEOMETRY SHADER OUTPUT
out struct GS_OUT
{
	vec4 wsCoord_Ambo;
	vec3 wsNormal;
} gs_out;




// NOT MINE 
out Vertex
{

	vec3 position;
	vec2 TexCoords;
	vec3 normal;
	vec2 tCoord;	// TEST

    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;

} vertex_out;
// NOT MINE 

//out vec4 wsCoordTest;

// .xyz = low-quality normal; .w = occlusion 
// Texture3D    grad_ambo_tex;					NOT YET

uniform mat4 projection;
uniform mat4 view;

//uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightPos;				// LIGHT
// GEOMETRY SHADER INPUTS:
// structvsOutputGsInput{ 
// float4 wsCoord: POSITION; 
// float3 uvw: TEX; 
// float4 f0123   : TEX1;  // the density values 
// float4 f4567   : TEX2;  //   at the corners 
// uintmc_case: TEX3;  // 0-255 };


//structGSOutput{ // Stream out to a VB & save for reuse! 
// // .xyz = wsCoord, .w = occlusion 
// float4 wsCoord_Ambo: POSITION; 
// float3 wsNormal: NORMAL;       
// };

// GEOMETRY SHADER OUTPUT
//out GS_OUT {
//	vec4 wsCoord_Ambo;
//	vec3 wsNormal;
//} gs_out;





layout (std140) uniform g_mc_lut { 
	//uint case_to_numpolys[256];
	uint case_to_numpolys[256];
	vec4 cornerAmask0123[12];
	//float abc[24];
	vec4 cornerAmask4567[12]; 
	vec4 cornerBmask0123[12]; 
	vec4 cornerBmask4567[12]; 
	vec3 vec_start[12]; 
	vec3 vec_dir[12]; 
};

layout (std140, binding = 1) uniform g_mc_lut2 { 
	ivec4 g_triTable[1280]; //5*256 
};


//cbuffer g_ambo_lut {
//  float  ambo_dist[16];
//  float4 occlusion_amt[16];
//  float3 g_ray_dirs_32[32];  // 32 rays w/a good poisson distrib.
//  float3 g_ray_dirs_64[64];  // 64 rays w/a good poisson distrib.
//  float3 g_ray_dirs_256[256]; // 256 rays w/a good poisson distrib.
//};




// our volume of density values
uniform sampler3D screenTexture;
// Texture3D field_tex;
// Texture3D grad_ambo_tex;
// Texture3D floaterTex;
// SamplerState Linear_ClampXY_WrapZ;
// SamplerState Nearest_ClampXY_WrapZ;
// SamplerState LinearRepeat;


//float WorldSpaceVolumeHeight= 2.0f*(256.0f/96.0f); 
//vec3 voxelDim = vec3(96.0f, 256.0f, 96.0f); 
//vec3 voxelDimMinusOne = vec3(95.0f, 256.0f, 95.0f); 
//
//
////float wsVoxelSize = 2.0f/95.0f;		// ACTUALLY USED THIS
//float wsVoxelSize = 2.0f/96.0f; 
//vec4 inv_voxelDim = vec4( 1.0f/voxelDim, 0.0f);
//vec4 inv_voxelDimMinusOne = vec4( 1.0f/voxelDimMinusOne, 0.0f );

float WorldSpaceVolumeHeight = 2.0f*(256.0f/96.0f);			// Volume Height
vec3 voxelDim = vec3(96.0f, 256.0f, 96.0f);					// Dimension of the whole cube
vec3 voxelDimMinusOne = vec3(95.0f, 256.0f, 95.0f);			// Dimension -1	->	actual number of voxels

// TODO
//float wsVoxelSize = 2.0f / 95.0f;								// ACTUALLY USED
float wsVoxelSize = 2.0f / 96.0f;								// half	the voxel size?

vec4 inv_voxelDim = vec4( 1.0f/voxelDim, 0.0f);				// percent in 0 to 1
vec4 inv_voxelDimMinusOne = vec4( 1.0f/voxelDimMinusOne, 0.0f );	// percent in 0 to 1 but minus 1




vec3 ComputeNormal(sampler3D tex, vec3 uvw) 
{ 
	vec4 stepP = vec4(inv_voxelDim); 


	vec3 gradient = vec3( 
		texture(tex, uvw + stepP.xww, 0.0f).x -texture(tex, uvw - stepP.xww, 0.0f).x, 
		texture(tex, uvw + stepP.wwy, 0.0f).x -texture(tex, uvw - stepP.wwy, 0.0f).x, 
		texture(tex, uvw + stepP.wzw, 0.0f).x -texture(tex, uvw-  stepP.wzw, 0.0f).x 
	); 
	
	return normalize(-gradient); 
}

GS_OUT PlaceVertOnEdge(VS_OUT input, int edgeNum)
{ 

	GS_OUT output;
	// Along this cell edge, where does the density value hit zero? 
	float str0= dot(cornerAmask0123[edgeNum], input.f0123) + 
				dot(cornerAmask4567[edgeNum], input.f4567);
	
	float str1= dot(cornerBmask0123[edgeNum], input.f0123) + 
				dot(cornerBmask4567[edgeNum], input.f4567); 
	
	float t = clamp( str0/(str0 - str1), 0.0f, 1.0f ); //saturate( str0/(str0 -str1) );  //0..1

	// use that to get wsCoord and uvw coords 
	vec3 pos_within_cell = vec_start[edgeNum] + t * vec_dir[edgeNum]; //[0..1] 
	


	vec3 wsCoord = input.wsCoord.xyz + pos_within_cell.xyz * wsVoxelSize; 
	
	// TODO
	vec3 uvw = input.uvw + pos_within_cell * inv_voxelDimMinusOne.xzy;		// ACTUALLY USED THIS
	//vec3 uvw = input.uvw + pos_within_cell * inv_voxelDim.xzy;
	
	//vec3 grad_ambo = grad_ambo_tex.SampleLevel(Linear_ClampXY_WrapZ, uvw, 0);
	vec4 grad_ambo = texture(screenTexture, uvw, 0.0f);


	output.wsCoord_Ambo.xyz = wsCoord; 
	output.wsCoord_Ambo.w = texture(screenTexture, uvw, 0.0f).w; //grad_ambo_tex.SampleLevel(s, uvw, 0).w; 

	output.wsNormal = ComputeNormal(screenTexture, uvw); 
	
	return output;
}



// ------------------
// NOT MINE 
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



vec2 CalculateUV(vec3 p)
{
	return vec2(mod(p.x,1.0f),mod(p.y,1.0f));
}

vec2 CalculateUVTest(vec3 p)
{
	return vec2(mod(p.x,1),mod(p.y,1));
}


vec3 CalculateTangent(vec3 p1, vec3 p2, vec3 p3)
{
	vec3 tangent;

	vec3 edge1 = p2 - p1;
	vec3 edge2 = p3 - p1;

	

	vec2 uv1 =  p1.xy; //CalculateUV(p1);
	vec2 uv2 =  p2.xy; //CalculateUV(p2);
	vec2 uv3 =  p3.xy; //CalculateUV(p3);

    vec2 deltaUV1 = uv2 - uv1;
    vec2 deltaUV2 = uv3 - uv1;

    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

    tangent = normalize(tangent);

	return tangent;

}

vec3 CalculateBiTangent(vec3 p1, vec3 p2, vec3 p3)
{

	vec3 biTangent;

	vec3 edge1 = p2 - p1;
	vec3 edge2 = p3 - p1;

	

	vec2 uv1 =  p1.xy; //CalculateUV(p1);
	vec2 uv2 =  p2.xy; //CalculateUV(p2);
	vec2 uv3 =  p3.xy; //CalculateUV(p3);

    vec2 deltaUV1 = uv2 - uv1;
    vec2 deltaUV2 = uv3 - uv1;



    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);



    biTangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    biTangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    biTangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

    biTangent = normalize(biTangent);

	return biTangent;

}

//mat3 calculateTBN(vec3 p1, vec3 p2, vec3 p3)
//{
//	return TBN;
//}
// ------------------
// NOT MINE 

void main() {


	uint cube_case = vs_out[0].mc_case;


	//GS_OUT output; 
	//uint num_polys = case_to_numpolys[vs_out[0].mc_case];
	uint num_polys = case_to_numpolys[vs_out[0].mc_case];
	uint table_pos = cube_case * 5; 


	// TESTING
	// -------
	/*
	for(int i = 0; i < num_polys; i++)
	{
		//gl_Position = gl_in[0].gl_Position;
		ivec4 polydata = g_triTable[ table_pos++];


		//outValue = vs_out[0].wsCoord.y;
		//outValue = layerId[0];
		layerID = layerId[0];
		//gl_Position = vec4(vs_out[0].wsCoord.xz,0.0f, 1.0f);
		gl_Position = projection * view * vec4(PlaceVertOnEdge(vs_out[0], polydata.x).wsCoord_Ambo.xyz,  1.0f);
		//gl_Position = PlaceVertOnEdge(vs_out[0], polydata.x).wsCoord_Ambo;
		EmitVertex();

		gl_Position = projection * view * vec4(PlaceVertOnEdge(vs_out[0], polydata.y).wsCoord_Ambo.xyz,  1.0f);
		//gl_Position = PlaceVertOnEdge(vs_out[0], polydata.x).wsCoord_Ambo;
		EmitVertex();

		gl_Position = projection * view * vec4(PlaceVertOnEdge(vs_out[0], polydata.z).wsCoord_Ambo.xyz, 1.0f);
		//gl_Position = PlaceVertOnEdge(vs_out[0], polydata.x).wsCoord_Ambo;
		EmitVertex();
		//TexCoords = gs_in[i].texCoords;
		//EmitVertex();
		//outValue = cornerAmask0123[1].x;

		// = 1.0f; // geoValue[i];
		EndPrimitive();
	}
	*/



	for(int i = 0; i < num_polys; i++)

	{

		//gl_Position = gl_in[0].gl_Position;

		ivec4 polydata = g_triTable[ table_pos++];

		// positions
		vec3 pos1 = PlaceVertOnEdge(vs_out[0], polydata.x).wsCoord_Ambo.xyz;
		vec3 pos2 = PlaceVertOnEdge(vs_out[0], polydata.y).wsCoord_Ambo.xyz;
		vec3 pos3 = PlaceVertOnEdge(vs_out[0], polydata.z).wsCoord_Ambo.xyz;

	
		// texture coordinates

		// normal vector
		//vec3 normal = PlaceVertOnEdge(vs_out[0], polydata.x).wsNormal;
		vec3 normal = CalculateNormal(pos1, pos2, pos3);
		normal = PlaceVertOnEdge(vs_out[0], polydata.x).wsNormal;

		// tangents
		vec3 tangent = CalculateTangent(pos1, pos2, pos3);
		
		// bitangents
		vec3 bitangent = CalculateBiTangent(pos1,pos2,pos3);



		vec3 T   = normalize(mat3(1.0f) * tangent);
		vec3 B   = normalize(mat3(1.0f) * bitangent);
		vec3 N   = normalize(mat3(1.0f) * normal);

		//	N = cross(T, B);

		mat3 TBN = transpose(mat3(T, B, N));



		layerID = layerId[0];



		gl_Position = projection * view * vec4(pos1,  1.0f);

		vertex_out.position = pos1;

		vertex_out.TexCoords = CalculateUV(pos1);

		//vertex_out.tCoord = CalculateUV(pos1);		// TEST

		//vertex_out.normal = normal;
		vertex_out.normal = PlaceVertOnEdge(vs_out[0], polydata.x).wsNormal;

		vertex_out.TangentLightPos = TBN * lightPos;
		vertex_out.TangentViewPos  = TBN * viewPos;
		vertex_out.TangentFragPos  = TBN * pos1;

		EmitVertex();




		normal = PlaceVertOnEdge(vs_out[0], polydata.y).wsNormal;

		N   = normalize(mat3(1.0f) * normal);

		TBN = transpose(mat3(T, B, N));



		gl_Position = projection * view * vec4(pos2,  1.0f);

		vertex_out.position = pos2;

		vertex_out.TexCoords = CalculateUV(pos1);

		//vertex_out.normal = normal;

		vertex_out.normal = PlaceVertOnEdge(vs_out[0], polydata.y).wsNormal;

		vertex_out.TangentLightPos = TBN * lightPos;
		vertex_out.TangentViewPos  = TBN * viewPos;
		vertex_out.TangentFragPos  = TBN * pos2;

		EmitVertex();



		normal = PlaceVertOnEdge(vs_out[0], polydata.z).wsNormal;

		N   = normalize(mat3(1.0f) * normal);

		TBN = transpose(mat3(T, B, N));


		gl_Position = projection * view * vec4(pos3, 1.0f);

		vertex_out.position = pos3;

		vertex_out.TexCoords = CalculateUV(pos1);

		//vertex_out.normal = normal;

		vertex_out.normal = PlaceVertOnEdge(vs_out[0], polydata.z).wsNormal;

		vertex_out.TangentLightPos = TBN * lightPos;
		vertex_out.TangentViewPos  = TBN * viewPos;
		vertex_out.TangentFragPos  = TBN * pos3;

		EmitVertex();

		//TexCoords = gs_in[i].texCoords;

		//EmitVertex();

		//outValue = cornerAmask0123[1].x;



		// = 1.0f; // geoValue[i];

		EndPrimitive();

	}


	// ------------

	//testValue = 1.0f;
	
	//wsCoordTest = vec4(1.0f, 1.0f, 1.0f, 1.0f);    

//	for (uint p = 0; p < num_polys; p++) 
//	{  
//		ivec4 polydata = g_triTable[ table_pos];//++ ]; 
//		wsCoordTest = vec4(1.0f, 1.0f, 1.0f, 1.0f);    
//	
//	
//		
//		
//		gs_out.wsCoord_Ambo = PlaceVertOnEdge(vs_out[0], polydata.x).wsCoord_Ambo;    
//		gl_Position = PlaceVertOnEdge(vs_out[0], polydata.x).wsCoord_Ambo;
//		EmitVertex();	// Stream.Append(output);
//
//		
//		gs_out.wsCoord_Ambo = PlaceVertOnEdge(vs_out[0], polydata.y).wsCoord_Ambo;
//		gl_Position = PlaceVertOnEdge(vs_out[0], polydata.y).wsCoord_Ambo;
//		//output = PlaceVertOnEdge(gs_in[0], polydata.y);     
//		EmitVertex();	// Stream.Append(output); 
//		outValue = gs_out.wsCoord_Ambo.x;
////		
////		gs_out.wsCoord_Ambo = PlaceVertOnEdge(vs_out[0], polydata.z).wsCoord_Ambo; 
////		gl_Position = PlaceVertOnEdge(vs_out[0], polydata.z).wsCoord_Ambo;
////		//output = PlaceVertOnEdge(gs_in[0], polydata.z);     
////		EmitVertex();	// Stream.Append(output); 
//		EndPrimitive();	// Stream.RestartStrip(); 
//
//
//	}


}
