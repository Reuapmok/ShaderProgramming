#pragma once

#include "glm/glm/glm.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "Shader.h"
#include <string>

#include <map>

struct Character
{
	GLuint TextureID;
	glm::ivec2 Size;
	glm::ivec2 Bearing;
	GLuint Advance;
};

class TextRenderer
{
public:
	TextRenderer();
	void RenderText(Shader &s, std::string text, float x, float y, float scale, glm::vec3 color);
	std::map<char, Character> Characters;
	unsigned int VAO, VBO;
};