#version 430 core
out vec4 FragColor;



in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
   vec4 FragPosLightSpace;
   //vec4 ShadowCoord;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;




//float ChebyshevUpperBound(vec2 Moments, float t)
//{
  // One-tailed inequality valid if t > Moments.x
  //float p = (t <= Moments.x) ? 1.0 : 0.0;
  // Compute variance
  //float Variance = (Moments.y – (Moments.x * Moments.x));
  //Variance = max(Variance, g_MinVariance);
  // Compute probabilistic upper bound.
  // float d = t – Moments.x;
  //float p_max = Variance / (Variance + d*d);
  //return max(p, p_max);
//}

float linstep(float low, float high, float v){
    return clamp((v-low)/(high-low), 0.0f, 1.0f);
}

float VSM(sampler2D depths, vec2 uv, float compare){
    vec2 moments = texture(depths, uv).xy;
    float p = smoothstep(compare-0.02f, compare, moments.x);
    float variance = max(moments.y - moments.x*moments.x, -0.001f);
    float d = compare - moments.x;

    float p_max = linstep(0.2f, 1.0f, variance / (variance + d*d));

    return clamp(max(p, p_max), 0.0f, 1.0f);
}

float VSMmine(vec4 ShadowCoord)
{
	//vec2 Moments = texture(shadowMap, LightTexCoord).xy;

	vec3 projCoords = ShadowCoord.xyz / ShadowCoord.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    vec2 moments = texture(shadowMap, projCoords.xy).rg; 

	float currentDepth = projCoords.z;

	// Surface is fully lit. as the current fragment is before the light occluder
	if (currentDepth <= moments.x)
			return 1.0 ;
	

	//float variance = E_x2 - Ex_2;
	//float mD = moments.x - currentDepth;
	//float mD_2 = mD * mD;
	//float p = variance / (variance + mD_2);

	// The fragment is either in shadow or penumbra. We now use chebyshev's upperBound to check
		// How likely this pixel is to be lit (p_max)
		//float variance = moments.y - (moments.x*moments.x);
		//variance = max(variance, 0.0002f);


	float p = smoothstep(currentDepth -0.0001f, currentDepth, moments.x);
    float variance = max(moments.y - moments.x*moments.x, 0.005f);
		// bias 
		vec3 lightDir = normalize(lightPos - fs_in.FragPos);
		vec3 normal = normalize(fs_in.Normal);
		float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

		//float d = currentDepth - moments.x;
		//float p_max = variance / (variance + d*d);

	float d = currentDepth - moments.x;
	//float p_max = variance / (variance + d*d);

	float p_max = linstep(0.2f, 1.0f, variance / (variance + d*d));
	

	if(currentDepth - bias  <= moments.x)
		p_max = 1.0;

	if(currentDepth + bias >= 1.0)
        p_max = 1.0;

     //return clamp(max(p, p_max), 0.0f, 1.0f);
	 //return p_max;

   
	//float shadow = clamp(max(p, p_max), 0.0f, 1.0f);
   	////
	////
	//if(projCoords.z > 1.0f)
	//	shadow = 1.0f;
   	////
   //return shadow;

	//float d = currentDepth - moments.x;
	//float p_max = variance / (variance + d*d);
	//
	return clamp(max(p,p_max),0.0f,1.0f);
	//return max(p, p_max);
}


float VSMmine1(vec4 ShadowCoord)
{
	vec3 projCoords = ShadowCoord.xyz / ShadowCoord.w;
	projCoords = projCoords * 0.5 + 0.5;

	// We retrive the two moments previously stored (depth and depth*depth)
    vec2 moments = texture(shadowMap, projCoords.xy).rg; 

	// get the current depth of the fragment
	float currentDepth = projCoords.z;

	// variance
	float variance = max(0.0001f,(moments.y - moments.x * moments.x));

	// Approximate the depth values in the kernel by a Gaussian distribution of mean μ and variance σ2 
	// Compute probabilistic upper bound.

	// max(σ^2/ (σ^2+ (d –μ)^2), (d < μ))

	//								(d	–	μ)
	float distanc = max(0, currentDepth - moments.x);
	
	//						σ^2	  /	(σ^2	+	(d –μ)^2
	float lightProbMax = variance /(variance+(distanc * distanc));


	vec3 lightDir = normalize(lightPos - fs_in.FragPos);
	vec3 normal = normalize(fs_in.Normal);
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
	//float lit_factor = 0.0f;

	if(currentDepth + bias > 1)
		lightProbMax = 1.0f;

	return lightProbMax;//*lightProbMax;

}



float ShadowCalculationSimple(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
	// bias 
	vec3 lightDir = normalize(lightPos - fs_in.FragPos);
	vec3 normal = normalize(fs_in.Normal);
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
	//shadow /= 9.0;
	
    return shadow;
}  

float ShadowCalculationESM(vec4 fragPosLightSpace)
{
	// perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;		// Z
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;								// D

	float constant = 50.0f;											// C
	// bias 
	vec3 lightDir = normalize(lightPos - fs_in.FragPos);
	vec3 normal = normalize(fs_in.Normal);
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

	float shadow = clamp(exp(constant * closestDepth) * exp(-constant * currentDepth),0.0f,1.0f);
	//shadow /= 9.0;

      // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z + bias > 1.0)
        shadow = 1.0;

    return shadow;
}

float texture2DCompare(sampler2D depths, vec2 uv, float compare){
    float depth = texture2D(depths, uv).r;
    return step(compare, depth);
}

float texture2DShadowLerp(sampler2D depths, vec2 size, vec2 uv, float compare){
    vec2 texelSize = vec2(1.0)/size;
    vec2 f = fract(uv*size+0.5);
    vec2 centroidUV = floor(uv*size+0.5)/size;

    float lb = texture2DCompare(depths, centroidUV+texelSize*vec2(0.0, 0.0), compare);
    float lt = texture2DCompare(depths, centroidUV+texelSize*vec2(0.0, 1.0), compare);
    float rb = texture2DCompare(depths, centroidUV+texelSize*vec2(1.0, 0.0), compare);
    float rt = texture2DCompare(depths, centroidUV+texelSize*vec2(1.0, 1.0), compare);
    float a = mix(lb, lt, f.y);
    float b = mix(rb, rt, f.y);
    float c = mix(a, b, f.x);
    return c;
}


 float ChebyshevUpperBound(vec2 Moments, float t)
{
  // One-tailed inequality valid if t > Moments.x
  //float p = (t <= Moments.x);
  vec3 lightDir = normalize(lightPos - fs_in.FragPos);
	vec3 normal = normalize(fs_in.Normal);
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

  float p = 0.0f;
  if(t - bias <= Moments.x)
		p = 1.0f;

  // Compute variance.
  float g_MinVariance = 0.0001f;
  float Variance = Moments.y - (Moments.x*Moments.x);
  Variance = max(Variance, g_MinVariance);

  // Compute probabilistic upper bound.
  float d = t - Moments.x;
  float p_max = Variance / (Variance + d*d);

  	if(t + bias > 1.0f)
		p_max = 1.0f;
  return max(p, p_max);
}

float ShadowContribution(vec2 LightTexCoord, float DistanceToLight)
{
  // Read the moments from the variance shadow map.
   vec2 Moments = texture(shadowMap, LightTexCoord).xy;
  // Compute the Chebyshev upper bound.
   return ChebyshevUpperBound(Moments, DistanceToLight);
}

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z + bias> 1.0)
        shadow = 0.0;
        
    return shadow;
}

void main()
{           
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(0.3);
    // ambient
    vec3 ambient = 0.3 * color;
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // calculate shadow
	vec3 projCoords = fs_in.FragPosLightSpace.xyz / fs_in.FragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
	float shadow = ShadowContribution(projCoords.xy, projCoords.z);
	//vec3 lighting = (ambient + (shadow) * (diffuse + specular)) * color; 

    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    

    FragColor = vec4(lighting, 1.0);
}