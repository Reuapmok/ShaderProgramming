#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in float Type1;

uniform sampler2D texSnow;


float circle()
{
    float a = sqrt((0.5 - TexCoords.x)*(0.5 - TexCoords.x) + (0.5 - TexCoords.y)*(0.5 - TexCoords.y));
    a = 1 - (a / 0.5);
    return a;
}


void main()
{

	vec4 TexColor = texture(texSnow, TexCoords);

	if(Type1 == 0.0f)
	{
		 FragColor = vec4(0.0f, 1.0f, 1.0f, circle());
	}
	else{
		FragColor = TexColor; //fragCol;//vec4(1.0f, 1.0f, 0.0f, 0.0f);//aColor;   
		}

}