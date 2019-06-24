#version 330 core
out vec4 FragDepths;

in vec4 pos;

vec2 ComputeMoments(float Depth)
{
  vec2 Moments;
  //Depth = Depth * 0.5 + 0.5;
  // First moment is the depth itself.
  Moments.x = Depth;
  // Compute partial derivatives of depth.
  float dx = dFdx(Depth);
  float dy = dFdy(Depth);
  // Compute second moment over the pixel extents.
  Moments.y = Depth*Depth + 0.25f*(dx*dx + dy*dy);

  return Moments;
}

void main()
{             
    //gl_FragDepth = gl_FragCoord.z;
	////FragDepths = vec4(vec2(ComputeMoments(gl_FragCoord.z)),0.0f,1.0f);
	//FragColor = vec4(vec2(gl_FragCoord.z),0.0f,1.0f);
	FragDepths = vec4(vec2(ComputeMoments(pos.z * 0.5 + 0.5)),0.0f,1.0f);
}