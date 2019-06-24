#version 330 core

out vec4 FragColor;

in vec4 pos;
in vec3 normal;
in vec2 texCoords;

uniform sampler2D terrainHeightmap;
uniform vec3 lightPos;



void main()
{
	
	//vec3 color = texture(terrainHeightmap, texCoords).rgb;
		
	vec3 color = vec3(1.0f, 1.0f, 0.2f);

	//FragColor = vec4(1.0f, 1.0f, 0.2f, 1.0f);
	FragColor = vec4(color, 1.0f);


	//vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal1 = normalize(normal);
    vec3 lightColor = vec3(0.3);
    // ambient
    vec3 ambient = 0.3 * color;
    // diffuse
    vec3 lightDir = normalize(lightPos - pos.xyz);
    float diff = max(dot(lightDir, normal1), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    //vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal1);
    float spec = 0.0;
    //vec3 halfwayDir = normalize(lightDir + viewDir);  
   // spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // calculate shadow
    //float shadow = ShadowCalculationESM(fs_in.FragPosLightSpace);
	//vec3 lighting = (ambient + (shadow) * (diffuse + specular)) * color;    
	
	//float shadow = ShadowCalculationSimple(fs_in.FragPosLightSpace);        
	//float shadow = VSMmine1(fs_in.FragPosLightSpace);			// OR USE THIS
	//float shadow = VSMmine1(fs_in.ShadowCoord);
	//vec3 projCoords = fs_in.FragPosLightSpace.xyz / fs_in.FragPosLightSpace.w;
    // transform to [0,1] range
   // projCoords = projCoords * 0.5 + 0.5;
	//float shadow = ShadowContribution(projCoords.xy, projCoords.z);
	vec3 lighting = (ambient + (diffuse + specular)) * color; 
	FragColor = vec4(lighting,1.0f);
}
