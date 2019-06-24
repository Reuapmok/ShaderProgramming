#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D image;

uniform bool gaussian;
  
uniform bool horizontal;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);


vec4 pingpong() {
    vec2 tex_offset = 1.0 / vec2(1024.0f,1024.0f); // gets size of single texel
    vec2 result = texture(image, TexCoords).rg * weight[0]; // current fragment's contribution
    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(image, TexCoords + vec2(tex_offset.x * i, 0.0)).rg * weight[i];
            result += texture(image, TexCoords - vec2(tex_offset.x * i, 0.0)).rg * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(image, TexCoords + vec2(0.0, tex_offset.y * i)).rg * weight[i];
            result += texture(image, TexCoords - vec2(0.0, tex_offset.y * i)).rg * weight[i];
        }
    }
    return vec4(result,0.0f, 1.0);
}

void main()
{             

		FragColor = pingpong();

}