#version 330 core
out vec4 FragColor;

flat in int layerID;

in float outValue;




in struct GS_OUT
{
	vec4 wsCoord_Ambo;
	vec3 wsNormal;
} gs_out;


in Vertex

{

	vec3 position;

	vec2 TexCoords;

	vec3 normal;

	vec2 tCoord;	//TEST


    vec3 TangentLightPos;

    vec3 TangentViewPos;

    vec3 TangentFragPos;

} vertex_out;


//uniform float height_scale;			// NEW
uniform vec3 lightPos;				// LIGHT



uniform sampler3D screenTexture;

uniform sampler2D diffuseMap;		// NEW
uniform sampler2D normalMap;		// NEW
uniform sampler2D depthMap;			// NEW

in struct VS_OUT {
	vec3 wsCoord;
	vec3 uvw;
	vec4 f0123;
	vec4 f4567;
	uint mc_case;
} vs_out;


float height_scale = 0.03f;//0.015f;

//vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
//{ 
//    float height =  texture(depthMap, texCoords).r;    
//    vec2 p = viewDir.xy / viewDir.z * (height * height_scale);
//    return texCoords - p;    
//} 

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
    // number of depth layers
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  

    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;

    // depth of current layer
    float currentLayerDepth = 0.0;

    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy * height_scale;//  /viewDir.z * height_scale; 
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(depthMap, currentTexCoords).r;
      
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(depthMap, currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }
    
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
	
    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(depthMap, prevTexCoords).r - currentLayerDepth + layerDepth;
 	
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

	
    return finalTexCoords;

	//return currentTexCoords;
}

void main()
{
	// ambient
	vec3 viewDir = normalize(vertex_out.TangentViewPos - vertex_out.TangentFragPos);

	vec2 texCoords = vertex_out.position.xy;
	//vec2 texCoords = vertex_out.TexCoords;

	texCoords = ParallaxMapping(vertex_out.position.xy, viewDir);
	//if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0) discard;

    vec3 normal = texture(normalMap, texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);   
   
    vec3 color = texture(diffuseMap, texCoords).rgb;
	    vec3 ambient = 0.3 * color;

    // diffuse
    vec3 lightDir = normalize(vertex_out.TangentLightPos - vertex_out.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;

    // specular    
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;
    FragColor = vec4(ambient + diffuse + specular, 1.0);

	//FragColor = vec4(0.5f,0.5f,0.0f,1.0f);

	// diffuse
	//vec3 norm = normalize(vertex_out.normal);
	//vec3 lightDir = normalize(lightPos - vertex_out.TangentFragPos);
	//float diff = max(dot(lightDir, norm), 0.0f);
	//vec3 color = texture(diffuseMap, vertex_out.position.rg).rgb;
	//vec3 diffuse = diff * vec3(1.0f);//color;
	//
	//
	//// specular
	//float specularStrength = 0.5f;
    //vec3 viewDir = normalize(vertex_out.TangentViewPos - vertex_out.TangentFragPos);
    //vec3 reflectDir = reflect(-lightDir, norm);  
    //float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    //vec3 specular = specularStrength * spec * vec3(1.0f,1.0f, 1.0f);//color;  



    //vec3 viewDir   = normalize(vertex_out.TangentViewPos - vertex_out.TangentFragPos);

    //vec2 texCoords = ParallaxMapping(vertex_out.TexCoords,  viewDir); // UNCOMMENT

	//vec2 texCoords = ParallaxMapping(vertex_out.position.rg,  viewDir);	// TEST
	//vertex_out.position.rg



	//vec3 color = texture(diffuseMap, texCoords).rgb;




		//FragColor = vec4(0.5f, 0.5f, 0.2f, 1.0f);
		    //FragColor = texture(diffuseMap, vec2(0.5f, 0.1f));
			 // then sample textures with new texture coords

			//vec4 diffuse = texture(diffuseMap, vertex_out.TexCoords);

			//vec4 diffuse = texture(diffuseMap, vertex_out.position.rg);		//TEST

			//vec4 diffuse = texture(diffuseMap, texCoords);
			//vec3 diffuse = diff * vec3(1.0f, 1.0f, 0.0f);//color; //* lightColor;
 //* lightColor;

			//FragColor = diffuse;
			//FragColor = vec4(diffuse ,1.0f);

		//FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
		//vec3 result = (ambient + diffuse + specular) * color;
		//FragColor = vec4(result, 1.0);
	

} 

